#ifndef ITACVTF_HPP
#define ITACVTF_HPP

#pragma once

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "../include/ConfigParser.hpp"

namespace UserInterface
{

    /**
     * Per-rank ASCII event log for ITAC STF conversion (visualization/events2stf.py).
     * Records modeled SEND/RECV with sender, receiver, time, bytes, and tag.
     * Clock unit is nanoseconds (same as DisCostiC operation.time).
     */
    class ItacVtf
    {
    private:
        std::ofstream ofs;
        std::string filename;
        DisCosTiC_Indextype rank = static_cast<DisCosTiC_Indextype>(-1);

        static long long ns_of(double t)
        {
            if (t < 0.0)
            {
                return 0;
            }
            return static_cast<long long>(std::llround(t));
        }

        static std::string sanitize(const std::string &name)
        {
            std::string out;
            out.reserve(name.size());
            for (char c : name)
            {
                if (c == ' ' || c == '\t')
                {
                    out.push_back('_');
                }
                else if (c != '\n' && c != '\r')
                {
                    out.push_back(c);
                }
            }
            return out.empty() ? std::string("region") : out;
        }

        static bool skip_message(DisCosTiC_Indextype src, DisCosTiC_Indextype dst, double bytes)
        {
            return src == dst || bytes <= 0.0;
        }

    public:
        ItacVtf(UserInterface::ConfigParser *args_info, DisCosTiC_Datatype mpi_rank, DisCosTiC_Datatype /*numRanks*/)
        {
            if (mpi_rank != 0)
            {
                this->rank = mpi_rank - 1;
                this->filename = args_info->getValue<std::string>("chromevizfilename") + std::to_string(this->rank) + ".avt";
                ofs.open(filename);
            }
        }

        void comp(DisCosTiC_Indextype app_rank, double start_ns, double dur_ns, const std::string &name)
        {
            if (!ofs.is_open() || dur_ns < 0.0)
            {
                return;
            }
            ofs << "COMP " << app_rank << " " << ns_of(start_ns) << " " << ns_of(dur_ns) << " " << sanitize(name) << "\n";
        }

        void send(DisCosTiC_Indextype src, DisCosTiC_Indextype dst, double start_ns, double dur_ns,
                  double bytes, DisCosTiC_Indextype tag)
        {
            if (!ofs.is_open() || skip_message(src, dst, bytes))
            {
                return;
            }
            ofs << "SEND " << src << " " << dst << " " << ns_of(start_ns) << " " << ns_of(dur_ns) << " "
                << ns_of(bytes) << " " << tag << "\n";
        }

        void recv(DisCosTiC_Indextype dst, DisCosTiC_Indextype src, double start_ns, double dur_ns,
                  double bytes, DisCosTiC_Indextype tag)
        {
            if (!ofs.is_open() || skip_message(src, dst, bytes))
            {
                return;
            }
            ofs << "RECV " << dst << " " << src << " " << ns_of(start_ns) << " " << ns_of(dur_ns) << " "
                << ns_of(bytes) << " " << tag << "\n";
        }

        void collective(DisCosTiC_Indextype app_rank, double start_ns, double dur_ns, const std::string &name)
        {
            if (!ofs.is_open() || dur_ns < 0.0)
            {
                return;
            }
            ofs << "COLL " << app_rank << " " << ns_of(start_ns) << " " << ns_of(dur_ns) << " " << sanitize(name) << "\n";
        }

        void closeFile()
        {
            if (ofs.is_open())
            {
                ofs.flush();
                ofs.close();
            }
        }

        ~ItacVtf()
        {
            closeFile();
        }
    };

} //!< namespace UserInterface
#endif
