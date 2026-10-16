#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>


#ifdef LIKWID_PERFMON

#   include <likwid-marker.h>

#endif


struct Timer {
    std::string name;

    std::chrono::time_point<std::chrono::steady_clock> startTime;
    double lastElapsed = 0.;
    size_t numElapsed = 0;
    double sumElapsed = 0;
    double minElapsed = 0, maxElapsed = 0;

    inline explicit Timer(const std::string &name_) : name(name_) {
#ifdef LIKWID_PERFMON
        LIKWID_MARKER_REGISTER(name.c_str());
#endif
    }

    inline void start() {
        startTime = std::chrono::steady_clock::now();
#ifdef LIKWID_PERFMON
        LIKWID_MARKER_START(name.c_str());
#endif
    }

    inline void stop() {
#ifdef LIKWID_PERFMON
        LIKWID_MARKER_STOP(name.c_str());
#endif
        auto stopTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = stopTime - startTime;
        lastElapsed = elapsed.count();

        sumElapsed += lastElapsed;
        if (0 == numElapsed) {
            minElapsed = lastElapsed;
            maxElapsed = lastElapsed;
        } else {
            minElapsed = std::min(minElapsed, lastElapsed);
            maxElapsed = std::max(maxElapsed, lastElapsed);
        }

        ++numElapsed;
    }

    inline double meanElapsed() {
        return 0 == numElapsed ? 0. : sumElapsed / numElapsed;
    }

    inline void print(int mpiRank) {
        std::cout << "Timer " << std::setw(16) << std::left << name << " on rank " << std::right << std::setw(3) << mpiRank << " : "
                  << std::setw(6) << (size_t) (1e3 * sumElapsed) << " ms, "
                  << std::setw(6) << (size_t) (1e3 * meanElapsed()) << " ms, "
                  << std::setw(6) << (size_t) (1e3 * minElapsed) << " ms, "
                  << std::setw(6) << (size_t) (1e3 * maxElapsed) << " ms, "
                  << std::setw(3) << numElapsed
                  << " (total, mean, min, max, timings)"
                  << std::endl;
    }

    inline void gatherAndPrint(int mpiRank, int numRanks) {
        if (0 != mpiRank) {
            // simply send data to root
            MPI_Gather(&numElapsed, 1, MPI_UINT64_T, nullptr, 0, MPI_UINT64_T, 0, MPI_COMM_WORLD);
            MPI_Gather(&sumElapsed, 1, MPI_DOUBLE, nullptr, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Gather(&minElapsed, 1, MPI_DOUBLE, nullptr, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Gather(&maxElapsed, 1, MPI_DOUBLE, nullptr, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        } else {
            // allocate data for gathering data
            auto numElapsedVec = new size_t[numRanks];
            auto sumElapsedVec = new double[numRanks];
            auto minElapsedVec = new double[numRanks];
            auto maxElapsedVec = new double[numRanks];

            // gather timer data
            MPI_Gather(&numElapsed, 1, MPI_UINT64_T, numElapsedVec, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
            MPI_Gather(&sumElapsed, 1, MPI_DOUBLE, sumElapsedVec, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Gather(&minElapsed, 1, MPI_DOUBLE, minElapsedVec, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
            MPI_Gather(&maxElapsed, 1, MPI_DOUBLE, maxElapsedVec, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

            // impersonate timers from other ranks by overwriting internal state
            for (auto r = 0; r < numRanks; ++r) {
                numElapsed = numElapsedVec[r];
                sumElapsed = sumElapsedVec[r];
                minElapsed = minElapsedVec[r];
                maxElapsed = maxElapsedVec[r];

                print(r);
            }

            // restore original values
            numElapsed = numElapsedVec[0];
            sumElapsed = sumElapsedVec[0];
            minElapsed = minElapsedVec[0];
            maxElapsed = maxElapsedVec[0];

            // clean up
            delete[] numElapsedVec;
            delete[] sumElapsedVec;
            delete[] minElapsedVec;
            delete[] maxElapsedVec;
        }
    }
};
