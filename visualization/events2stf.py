#!/usr/bin/env python3
#    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
#
# Convert DisCostiC ITAC event logs (.avt) to Intel STF via OTF2 (MPI send/recv records).
# stftool cannot ingest ASCII VTF, so this writes a temporary OTF2 with MpiSend/MpiRecv
# and runs otf2-to-stf --p2p --user-func. The OTF2 directory is deleted afterwards.

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import tempfile
from collections import defaultdict, deque
from typing import Dict, List, Tuple

import otf2

TIMER_RESOLUTION = int(1e9)


def _parse_avt(path: str) -> Tuple[int, List[tuple]]:
    events: List[tuple] = []
    max_rank = 0
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for raw in fh:
            line = raw.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            kind = parts[0]
            if kind == "COMP" and len(parts) >= 5:
                rank, start, dur = int(parts[1]), int(parts[2]), int(parts[3])
                name = parts[4]
                if dur > 0:
                    events.append(("COMP", rank, start, dur, name))
                    max_rank = max(max_rank, rank)
            elif kind == "SEND" and len(parts) >= 7:
                src, dst = int(parts[1]), int(parts[2])
                start, dur, nbytes, tag = int(parts[3]), int(parts[4]), int(parts[5]), int(parts[6])
                if src != dst and nbytes > 0:
                    events.append(("SEND", src, dst, start, dur, nbytes, tag))
                    max_rank = max(max_rank, src, dst)
            elif kind == "RECV" and len(parts) >= 7:
                dst, src = int(parts[1]), int(parts[2])
                start, dur, nbytes, tag = int(parts[3]), int(parts[4]), int(parts[5]), int(parts[6])
                if src != dst and nbytes > 0:
                    events.append(("RECV", dst, src, start, dur, nbytes, tag))
                    max_rank = max(max_rank, src, dst)
            elif kind == "COLL" and len(parts) >= 5:
                rank, start, dur = int(parts[1]), int(parts[2]), int(parts[3])
                name = parts[4]
                if dur > 0:
                    events.append(("COLL", rank, start, dur, name))
                    max_rank = max(max_rank, rank)
            else:
                print(f"Ignoring malformed line: {line}", file=sys.stderr)
    return max_rank + 1, events


def _complete_messages(events: List[tuple]) -> List[tuple]:
    """Pair SEND/RECV and synthesize the missing half so otf2-to-stf --p2p can match.

    DisCostiC may omit RECV lines when the modeled target was overwritten with self.
    """
    send_q: Dict[tuple, deque] = defaultdict(deque)
    recvs: List[tuple] = []
    for ev in events:
        if ev[0] == "SEND":
            _, src, dst, start, dur, nbytes, tag = ev
            send_q[(src, dst, tag, nbytes)].append(ev)
        elif ev[0] == "RECV":
            recvs.append(ev)

    unmatched_recv: List[tuple] = []
    for ev in recvs:
        _, dst, src, start, dur, nbytes, tag = ev
        q = send_q[(src, dst, tag, nbytes)]
        if q:
            q.popleft()
        else:
            unmatched_recv.append(ev)

    extra: List[tuple] = []
    for (src, dst, tag, nbytes), q in send_q.items():
        for send in q:
            _, _, _, start, dur, _, _ = send
            recv_start = start + max(dur, 0)
            extra.append(("RECV", dst, src, recv_start, max(dur, 1), nbytes, tag))
    for ev in unmatched_recv:
        _, dst, src, start, dur, nbytes, tag = ev
        extra.append(("SEND", src, dst, max(start - max(dur, 1), 0), max(dur, 1), nbytes, tag))

    if extra:
        print(f"Synthesized {len(extra)} matching SEND/RECV record(s) for ITAC P2P pairing", file=sys.stderr)
    return events + extra


def _region(trace, cache: Dict[Tuple[str, object], object], name: str, paradigm):
    key = (name, paradigm)
    if key not in cache:
        cache[key] = trace.definitions.region(name, paradigm=paradigm)
    return cache[key]


def write_otf2(nprocs: int, events: List[tuple], out_dir: str) -> None:
    if os.path.exists(out_dir):
        shutil.rmtree(out_dir)
    with otf2.writer.open(out_dir, timer_resolution=TIMER_RESOLUTION) as trace:
        root = trace.definitions.system_tree_node("root node")
        host = trace.definitions.system_tree_node("myHost", parent=root)
        locations = []
        writers = []
        for rank in range(nprocs):
            group = trace.definitions.location_group(f"Rank {rank}", system_tree_parent=host)
            loc = trace.definitions.location(f"Rank {rank} thread", group=group)
            locations.append(loc)
            writers.append(trace.event_writer_from_location(loc))

        # COMM_LOCATIONS must exist before COMM_GROUP for otf2-to-stf --p2p.
        _comm_locations = trace.definitions.group(
            "MPI COMM_LOCATIONS",
            group_type=otf2.GroupType.COMM_LOCATIONS,
            members=tuple(locations),
        )
        comm_group = trace.definitions.group(
            "MPI_COMM_WORLD COMM_GROUP",
            group_type=otf2.GroupType.COMM_GROUP,
            members=tuple(range(nprocs)),
        )
        comm = trace.definitions.comm("MPI_COMM_WORLD", comm_group)

        regions: Dict[Tuple[str, object], object] = {}
        per_rank: List[List[object]] = [[] for _ in range(nprocs)]

        for ev in events:
            kind = ev[0]
            if kind == "COMP":
                _, rank, start, dur, name = ev
                if dur <= 0:
                    continue
                region = _region(trace, regions, name, otf2.Paradigm.USER)
                per_rank[rank].append(otf2.events.Enter(start, region))
                per_rank[rank].append(otf2.events.Leave(start + dur, region))
            elif kind == "SEND":
                _, src, dst, start, dur, nbytes, tag = ev
                region = _region(trace, regions, "MPI_Isend", otf2.Paradigm.MPI)
                end = start + max(dur, 1)
                per_rank[src].append(otf2.events.Enter(start, region))
                per_rank[src].append(otf2.events.MpiSend(start, dst, comm, tag, max(nbytes, 0)))
                per_rank[src].append(otf2.events.Leave(end, region))
            elif kind == "RECV":
                _, dst, src, start, dur, nbytes, tag = ev
                region = _region(trace, regions, "MPI_Irecv", otf2.Paradigm.MPI)
                end = start + max(dur, 1)
                per_rank[dst].append(otf2.events.Enter(start, region))
                per_rank[dst].append(otf2.events.MpiRecv(end, src, comm, tag, max(nbytes, 0)))
                per_rank[dst].append(otf2.events.Leave(end, region))
            elif kind == "COLL":
                _, rank, start, dur, name = ev
                if dur <= 0:
                    continue
                mpi_name = name if name.startswith("MPI_") else f"MPI_{name}"
                region = _region(trace, regions, mpi_name, otf2.Paradigm.MPI)
                per_rank[rank].append(otf2.events.Enter(start, region))
                per_rank[rank].append(otf2.events.Leave(start + dur, region))

        event_pri = {
            "MpiRecv": 0,
            "Leave": 1,
            "Enter": 2,
            "MpiSend": 3,
        }
        for rank, evs in enumerate(per_rank):
            for event in sorted(
                evs, key=lambda e: (e.time, event_pri.get(type(e).__name__, 4), type(e).__name__)
            ):
                writers[rank].write(event)

        del _comm_locations


def _find_otf2_to_stf() -> str:
    found = shutil.which("otf2-to-stf")
    if found:
        return found
    raise FileNotFoundError(
        "otf2-to-stf not on PATH. Load Intel ITAC, e.g. module load intel-itac/2022.4.0"
    )


def _find_otf2_anchor(otf2_dir: str) -> str:
    preferred = os.path.join(otf2_dir, "traces.otf2")
    if os.path.isfile(preferred):
        return preferred
    for root, _dirs, files in os.walk(otf2_dir):
        for name in files:
            if name.endswith(".otf2"):
                return os.path.join(root, name)
    raise FileNotFoundError(f"OTF2 anchor missing under {otf2_dir}")


def convert_to_stf(otf2_dir: str, stf_path: str) -> None:
    converter = _find_otf2_to_stf()
    anchor = _find_otf2_anchor(otf2_dir)
    work = os.path.dirname(os.path.abspath(anchor))
    name = os.path.basename(anchor)
    subprocess.run([converter, "--p2p", "--user-func", name], cwd=work, check=True)
    stem = os.path.splitext(name)[0]
    produced = os.path.join(work, f"{stem}.single.stf")
    if not os.path.isfile(produced):
        alt = os.path.join(work, "traces.single.stf")
        if os.path.isfile(alt):
            produced = alt
        else:
            raise FileNotFoundError(f"otf2-to-stf did not write {produced}")
    out_dir = os.path.dirname(os.path.abspath(stf_path))
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    shutil.move(produced, stf_path)


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert DisCostiC .avt event logs to ITAC STF")
    parser.add_argument("-i", "--input", required=True, help="merged .avt event log")
    parser.add_argument("-o", "--output", required=True, help="output .single.stf path")
    parser.add_argument("--keep-otf2", action="store_true", help="keep the temporary OTF2 directory")
    args = parser.parse_args()

    if not os.path.isfile(args.input):
        print(f"Input not found: {args.input}", file=sys.stderr)
        return 1

    nprocs, events = _parse_avt(args.input)
    events = _complete_messages(events)
    if nprocs < 1 or not events:
        print("No events parsed from", args.input, file=sys.stderr)
        return 1

    tmp = tempfile.mkdtemp(prefix="discostic-itac-")
    try:
        write_otf2(nprocs, events, tmp)
        convert_to_stf(tmp, args.output)
    finally:
        if args.keep_otf2:
            print("Kept OTF2 at", tmp)
        else:
            shutil.rmtree(tmp, ignore_errors=True)

    if not os.path.isfile(args.output) or os.path.getsize(args.output) < 50:
        print("STF conversion failed:", args.output, file=sys.stderr)
        return 1
    print(f"STF ready: {args.output}  ({nprocs} ranks, {len(events)} events)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
