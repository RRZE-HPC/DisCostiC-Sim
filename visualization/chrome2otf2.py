#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import argparse
import copy
import gzip
import json
import os
import shutil
import traceback

from dataclasses import dataclass, field
from typing import Any, Callable, Dict, List, Optional, Set, Tuple

import otf2

TIMER_GRANULARITY = int(1e9)  # chrome traces uses micro seconds but has precision up to nanoseconds in TF2!


@dataclass
class Location:
    name: str
    writer: otf2.event_writer.EventWriter
    location: otf2.definitions.Location


@dataclass
class Process:
    name: str
    group: otf2.definitions.LocationGroup
    threads: Dict[int, Location] = field(default_factory=dict)


@dataclass
class DurationEvent:
    is_begin: bool

    time: int  # ticks in TIMER_GRANULARITY
    pid: int
    tid: int

    # These are optional in the chrome trace format, in which case they will be empty left empty
    name: str
    category: str
    args: Dict[str, Any]


@dataclass
class Communicator:
    name: str
    members: Set[otf2.definitions.Location]
    communicator: otf2.definitions.Comm


def is_gzip_file(path):
    try:
        file = gzip.open(path)
        file.read(1)
        return True
    except Exception:
        return False


class ChromeTrace2OTF2:
    def __init__(self, input_path: str, memory_profile_path: Optional[str] = None) -> None:
        """
        input_path : A path to a folder containing a "<hostname>.memory_profile.json.gz" and
                     "<hostname>.trace.json.gz" or path to the latter directly.
        memory_profile_path : Path to the "<hostname>.memory_profile.json.gz". If input_path
                     is a folder and this is not set, then search in the folder for this file.
        """

        if not input_path or not os.path.exists(input_path):
            raise Exception("Specified location does not exist:", input_path)

        if memory_profile_path and not os.path.exists(memory_profile_path):
            raise Exception("Specified memory profile location does not exist:", memory_profile_path)

        self._trace_file: Optional[str] = None
        self._memory_trace_file = memory_profile_path

        if os.path.isfile(input_path):
            self._trace_file = input_path
        elif os.path.isdir(input_path):
            for root, _, files in os.walk(input_path):
                for filename in files:
                    if filename.endswith('.trace.json.gz') or filename.endswith('.trace.json'):
                        if self._trace_file is None:
                            self._trace_file = os.path.join(root, filename)
                        else:
                            raise Exception("Found multiple chrome traces. Please specify the file or folder directly!")

                    if filename.endswith('.memory_profile.json.gz') or filename.endswith('.memory_profile.json'):
                        if self._memory_trace_file is None:
                            self._memory_trace_file = os.path.join(root, filename)
                        else:
                            raise Exception(
                                "Found multiple memory profiles. Please specify the file or folder directly!"
                            )

        if not self._trace_file:
            raise Exception("No chrome trace found")

        self._process_map: Dict[int, Process] = {}
        self._function_map: Dict[str, otf2.definitions.Region] = {}
        self._metric_map: Dict[str, otf2.definitions.Metric] = {}
        self._dataflow_start: Dict[str, Any] = {}
        self._communicators: List[Communicator] = []

        self._otf2_root_node: Optional[otf2.definitions.SystemTreeNode] = None
        self._otf2_system_tree_host: Optional[otf2.definitions.SystemTreeNode] = None
        self._otf2_comm_world: Optional[otf2.definitions.Group] = None

        self._phase_handlers: Dict[str, Callable[[Dict, otf2.writer.Writer], None]] = {
            'B': self._handle_duration_begin_end,
            'E': self._handle_duration_begin_end,
            'X': self._handle_complete,
            'i': self._handle_instant,
            'I': self._handle_deprecated,  # Deprecated instant event
            'C': self._handle_counter,
            'b': self._handle_async_nestable_start,
            'n': self._handle_async_nestable_instant,
            'e': self._handle_async_nestable_end,
            'S': self._handle_deprecated,  # Deprecated async start event
            'T': self._handle_deprecated,  # Deprecated async step into event
            'p': self._handle_deprecated,  # Deprecated async step past event
            'F': self._handle_deprecated,  # Deprecated async end event
            's': self._handle_flow_start,
            't': self._handle_flow_step,
            'f': self._handle_flow_end,
            'P': self._handle_sample,
            'N': self._handle_object_create,
            'O': self._handle_object_snapshot,
            'D': self._handle_object_destroy,
            'M': self._handle_metadata,
            'V': self._handle_memory_dump_global,
            'v': self._handle_memory_dump_process,
            'R': self._handle_mark,
            'c': self._handle_clock_sync,
            '(': self._handle_context_enter,
            ')': self._handle_context_leave,
        }

        self._duration_events: List[DurationEvent] = []
        self._flow_events: List[Tuple[Dict, Dict]] = []
        self._location_events: Dict[otf2.definitions.Location, List[otf2.events._Event]] = {}

    def convert_trace(self, output_dir: str) -> None:
        if not output_dir:
            raise Exception("No output trace")
        if not self._trace_file:
            raise Exception("No input trace")

        with otf2.writer.open(output_dir, timer_resolution=TIMER_GRANULARITY) as otf2_trace:
            self._otf2_root_node = otf2_trace.definitions.system_tree_node("root node")
            self._otf2_system_tree_host = otf2_trace.definitions.system_tree_node("myHost", parent=self._otf2_root_node)

            with gzip.open(self._trace_file, 'rb') if is_gzip_file(self._trace_file) else open(
                self._trace_file, 'rb'
            ) as json_file:
                chrome_data = json.load(json_file)
                self._convert_event_trace(chrome_data, otf2_trace)

            if self._memory_trace_file:
                with gzip.open(self._memory_trace_file, 'rb') if is_gzip_file(self._memory_trace_file) else open(
                    self._memory_trace_file, 'rb'
                ) as json_file:
                    memory_data = json.load(json_file)
                    self._convert_memory_profile(memory_data, otf2_trace)

    def _convert_event_trace(self, chrome_data: Dict, otf2_trace: otf2.writer.Writer) -> None:
        self._duration_events = []
        self._flow_events = []
        for event in chrome_data['traceEvents']:
            if not event:
                # Trace might contain an empty event at the end for some reason
                continue

            for key in ['ts', 'dur']:
                if key in event:
                    event[key] = int(float(event[key]))

            phase = event['ph']
            if phase in self._phase_handlers:
                try:
                    self._phase_handlers[phase](event, otf2_trace)
                except Exception as exception:
                    print("Trying to process event raised an exception:")
                    print("    Event:", event)
                    print("    Exception:", exception)
                    traceback.print_exc()
            else:
                print(f"Unknown event found: {event}")

        for event in sorted(self._duration_events, key=lambda e: e.time):
            if event.name not in self._function_map:
                self._otf2_add_function(event.name, otf2_trace)
            otf2_function = self._function_map[event.name]

            location = self._get_location(event.pid, event.tid, otf2_trace).location

            self._location_events[location].append(
                otf2.events.Enter(event.time, otf2_function)
                if event.is_begin
                else otf2.events.Leave(event.time, otf2_function)
            )

        # Collect all OTF2 locations participating in flow events and create a COMM_LOCATIONS group containing
        # all of them for the paradigm.
        paradigm_locations: Set[Tuple[int, int]] = set()
        for send_event, receive_event in self._flow_events:
            paradigm_locations.add((int(send_event['pid']), int(send_event['tid'])))
            paradigm_locations.add((int(receive_event['pid']), int(receive_event['tid'])))
        members = tuple((self._get_location(s, r, otf2_trace).location for s, r in paradigm_locations))
        paradigm_group = otf2_trace.definitions.group(
            "DataFlow (COMM_LOCATIONS)",
            group_type=otf2.GroupType.COMM_LOCATIONS,
            members=members,
        )

        for send_event, receive_event in self._flow_events:
            send_location = self._get_location_from_event(send_event, otf2_trace)
            recv_location = self._get_location_from_event(receive_event, otf2_trace)
            communicator = self._get_communicator(
                str(send_event['cat']),
                paradigm_group,
                set([send_location.location, recv_location.location]),
                otf2_trace,
            )
            otf2_communicator = communicator.communicator

            self._location_events[send_location.location].append(
                otf2.events.MpiSend(
                    self._convert_time_to_ticks(int(send_event['ts'])),
                    otf2_communicator.rank(recv_location.location),
                    otf2_communicator,
                    int(send_event['id']),
                    0,
                )
            )

            self._location_events[recv_location.location].append(
                otf2.events.MpiRecv(
                    self._convert_time_to_ticks(int(receive_event['ts'])),
                    otf2_communicator.rank(send_location.location),
                    otf2_communicator,
                    int(receive_event['id']),
                    0,
                )
            )

        for location, events in self._location_events.items():
            writer = otf2_trace.event_writer_from_location(location)
            for event in sorted(events, key=lambda e: e.time):
                writer.write(event)

    @staticmethod
    def _convert_duration_event(event: Dict) -> DurationEvent:
        if event['ph'] not in ['B', 'E']:
            raise ValueError("May only be constructed from chrome trace duration events!")

        for key in event.keys():
            if key not in ['ph', 'ts', 'pid', 'tid', 'name', 'cat', 'args']:
                print("Ignoring unknown event key:", key)

        return DurationEvent(
            is_begin=event['ph'] == 'B',
            time=int(event['ts']),
            pid=int(event['pid']),
            tid=int(event['tid']),
            # Optional arguments
            name=event['name'] if 'name' in event else "",
            category=event['cat'] if 'cat' in event else "",
            args=event['args'] if 'args' in event else {},
        )

    def _convert_memory_profile(self, memory_data: Dict, otf2_trace: otf2.writer.Writer) -> None:
        otf2_location_group = otf2_trace.definitions.location_group(
            "TF Memory Allocators", system_tree_parent=self._otf2_system_tree_host
        )

        memory_activities = {}
        otf2_attributes = {}
        uint_metadata = [  # These are some values which are strings in the JSON even though they are integers
            "stackReservedBytes",
            "heapAllocatedBytes",
            "freeMemoryBytes",
            "peakBytesInUse",
            "requestedBytes",
            "allocationBytes",
            "address",
            "stepId",
        ]

        last_leave = None

        for allocator_name, profile in memory_data['memoryProfilePerAllocator'].items():
            location_writer = otf2_trace.event_writer(allocator_name, group=otf2_location_group)

            for snapshot in profile['memoryProfileSnapshots']:
                activity = snapshot['activityMetadata']['memoryActivity']
                if activity not in memory_activities:
                    memory_activities[activity] = otf2_trace.definitions.region(activity, paradigm=otf2.Paradigm.USER)

                event_attributes = copy.deepcopy(snapshot['activityMetadata'])
                event_attributes.update(snapshot['aggregationStats'])

                otf2_event_attributes = {}

                for key, value in event_attributes.items():
                    if key not in otf2_attributes:
                        attribute_type = otf2.Type.STRING
                        if key in uint_metadata:
                            value = int(value)
                            attribute_type = otf2.Type.UINT64
                        elif key is isinstance(value, int):
                            attribute_type = otf2.Type.INT64
                        elif key is isinstance(value, float):
                            attribute_type = otf2.Type.DOUBLE

                        otf2_attributes[key] = otf2_trace.definitions.attribute(name=key, type=attribute_type)

                    otf2_event_attributes[otf2_attributes[key]] = value

                timestamp = int(snapshot['timeOffsetPs']) // 1000  # Time is in picoseconds but precision is nanoseconds
                if last_leave:  # Put the leave at the next enter in order to not create invisible metrics and regions
                    location_writer.leave(timestamp, region=last_leave)
                location_writer.enter(timestamp, memory_activities[activity], attributes=otf2_event_attributes)
                last_leave = memory_activities[activity]

        if last_leave:
            location_writer.leave(timestamp, region=last_leave)

    @staticmethod
    def _convert_time_to_ticks(timestamp: float) -> int:
        """Converts microseconds with 3 decimal places for nanoseconds to nanoseconds (integer)"""
        return int(timestamp * 1e3)

    def _get_location(self, pid: int, tid: int, otf2_trace: otf2.writer.Writer) -> Location:
        if pid not in self._process_map:
            self._otf2_add_process(pid, otf2_trace, self._otf2_system_tree_host)

        if tid not in self._process_map[pid].threads:
            self._otf2_add_thread(tid, pid, otf2_trace)

        return self._process_map[pid].threads[tid]

    def _get_location_from_event(self, event: Dict, otf2_trace: otf2.writer.Writer) -> Location:
        return self._get_location(int(event['pid']), int(event['tid']), otf2_trace)

    def _get_communicator(
        self,
        name: str,
        paradigm_group: otf2.definitions.Group,
        members: Set[otf2.definitions.Location],
        otf2_trace: otf2.writer.Writer,
    ) -> Communicator:
        for communicator in self._communicators:
            if communicator.name == name and communicator.members == members:
                return communicator

        ranks = tuple((paradigm_group.rank(location) for location in members))
        communicator_group = otf2_trace.definitions.group(
            name + " COMM_GROUP", group_type=otf2.GroupType.COMM_GROUP, members=ranks
        )
        communicator = otf2_trace.definitions.comm(name, communicator_group)
        self._communicators.append(Communicator(name, members, communicator))
        return self._communicators[-1]

    # Event handlers for every phase

    def _handle_deprecated(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled deprecated event", event)

    def _handle_duration_begin_end(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        event['ts'] = ChromeTrace2OTF2._convert_time_to_ticks(int(event['ts']))
        self._duration_events.append(self._convert_duration_event(event))

    def _handle_complete(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        # Complete Events, TensorFlow seems to not use B and E events in an attempt to reduce the trace file size.
        # Split these complete events into enter/leave events in order to sort them by timestamp.
        # A special case might be X and B,E events being used in the same trace.
        if 'ts' not in event:
            raise KeyError("Required ts is missing in the given event!")

        enter_event = copy.deepcopy(event)
        del enter_event['dur']
        enter_event['ph'] = 'B'
        # Try to get more time precision from rocprof-specific data in args
        if 'args' in event and 'BeginNs' in event['args']:
            enter_event['ts'] = event['args']['BeginNs']
        else:
            enter_event['ts'] = ChromeTrace2OTF2._convert_time_to_ticks(int(event['ts']))

        leave_event = copy.deepcopy(event)
        del leave_event['dur']
        leave_event['ph'] = 'E'
        # Try to get more time precision from rocprof-specific data in args
        if 'args' in event and 'EndNs' in event['args']:
            leave_event['ts'] = event['args']['EndNs']
        else:
            # dur key is only optional but I've yet to see a case where it isn't set.
            duration = int(event['dur']) if 'dur' in event else 0
            leave_event['ts'] = ChromeTrace2OTF2._convert_time_to_ticks(int(event['ts']) + duration)

        self._duration_events.append(self._convert_duration_event(enter_event))
        self._duration_events.append(self._convert_duration_event(leave_event))

    def _handle_instant(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        # E.g., with a kineto pytorch trace, these are generated for memory accesses
        # This would map well to a counter
        # However, in general, these are more like samples
        print("Unhandled event", event)

    # TODO Map newly created processes for only collecting one metric to process with same name
    def _handle_counter(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        metric_name = event['name']
        if metric_name == 'Allocated Bytes':
            if metric_name not in self._metric_map:
                self.otf2_add_metric(otf2_trace, metric_name, 'Bytes')

            metric_value = event['args']['Allocator Bytes in Use']
            writer = self._get_location_from_event(event, otf2_trace).writer
            writer.metric(self._convert_time_to_ticks(event['ts']), self._metric_map[metric_name], metric_value)

    def _handle_async_nestable_start(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_async_nestable_instant(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_async_nestable_end(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    # TODO implementation of dataflow
    def _handle_flow_start(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        # The (category, scope (optional), id) triple identifies an event tree of corresponding flow events.
        if 'ts' not in event:
            print("Expected the flow event to have a timestamp.")
            return

        if 'scope' in event:
            print("Flow events with scope are not yet supported.")
            return

        if self._dataflow_start:
            print(f"Unsupported dataflow event usage in trace: {event}")
            self._dataflow_start = {}
        self._dataflow_start = event

    def _handle_flow_step(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        if 'id' not in self._dataflow_start or self._dataflow_start['id'] != event['id']:
            print(f"Unsupported dataflow event usage in trace: {event}")

        self._flow_events.append((self._dataflow_start, event))
        self._dataflow_start = {}

    def _handle_flow_end(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        self._dataflow_start = {}

        #print("Unhandled event", event)

    def _handle_sample(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_object_create(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_object_snapshot(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_object_destroy(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_metadata(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        if 'name' not in event:
            return
        event_type = event['name']

        if event_type == 'process_name':
            pid = int(event['pid'])
            name = event['args']['name']
            self._otf2_add_process(pid, otf2_trace, self._otf2_system_tree_host, name)

        elif event_type == 'thread_name':
            pid = int(event['pid'])
            tid = int(event['tid'])
            name = event['args']['name']
            if pid not in self._process_map:
                self._otf2_add_process(pid, otf2_trace, self._otf2_system_tree_host, name)
            assert (
                tid not in self._process_map[pid].threads
            ), "The thread_name metadata event should be the very first event for that thread!"
            self._otf2_add_thread(tid, pid, otf2_trace, name)

        elif event_type == 'process_labels':
            labels = event['args']['labels']
            print("Unhandled metadata event type:", event_type, labels)

        elif event_type == 'process_sort_index:':
            sort_index = event['sort_index']['labels']
            print("Unhandled metadata event type:", event_type, sort_index)

        elif event_type == 'thread_sort_index:':
            sort_index = event['sort_index']['labels']
            print("Unhandled metadata event type:", event_type, sort_index)

        else:
            print("Unknown metadata event:", event)

    def _handle_memory_dump_global(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_memory_dump_process(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_mark(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_clock_sync(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_context_enter(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    def _handle_context_leave(self, event: Dict, otf2_trace: otf2.writer.Writer) -> None:
        print("Unhandled event", event)

    # OTF2 Helpers to Add Defintiions

    def otf2_add_metric(self, otf2_trace: otf2.writer.Writer, name: str, unit: str) -> None:
        metric = otf2_trace.definitions.metric(name, unit=unit)
        self._metric_map[name] = metric

    def _otf2_add_process(
        self,
        pid: int,
        otf2_trace: otf2.writer.Writer,
        otf2_system_tree_node: otf2.definitions.SystemTreeNode,
        name: Optional[str] = None,
    ) -> None:
        process_name = name if name else str(pid)
        otf2_location_group = otf2_trace.definitions.location_group(
            process_name, system_tree_parent=otf2_system_tree_node
        )

        if pid in self._process_map:
            process = self._process_map[pid]
            process.group = otf2_location_group
            process.name = process_name
        else:
            self._process_map[pid] = Process(name=process_name, group=otf2_location_group)

    def _otf2_add_thread(self, tid: int, pid: int, otf2_trace: otf2.writer.Writer, name: Optional[str] = None) -> None:
        process = self._process_map[pid]
        thread_name = name if name else f"{process.name} {tid}"

        location = Location(
            thread_name,
            otf2_trace.event_writer(thread_name, group=process.group),
            otf2_trace.definitions.location(thread_name, group=process.group),
        )

        process.threads[tid] = location
        if location.location not in self._location_events:
            self._location_events[location.location] = []

    def _otf2_add_function(self, name: str, otf2_trace: otf2.writer.Writer) -> None:
        otf2_function = otf2_trace.definitions.region(name, paradigm=otf2.Paradigm.USER)
        self._function_map[name] = otf2_function


def cli():
    parser = argparse.ArgumentParser(description="Convert chrome traces into OTF2")
    parser.add_argument(
        "-i",
        "--input",
        type=str,
        required=True,
        help="chrome tracing file",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=str,
        required=True,
        help="OTF2 Tracing folder",
    )
    parser.add_argument(
        "-c",
        "--clean",
        action="store_true",
        help="Clean (delete) the output folder if it exists",
    )
    args = parser.parse_args()

    out_folder = args.output
    if args.clean and os.path.exists(out_folder):
        shutil.rmtree(out_folder)

    converter = ChromeTrace2OTF2(args.input)
    converter.convert_trace(out_folder)


if __name__ == '__main__':
    cli()