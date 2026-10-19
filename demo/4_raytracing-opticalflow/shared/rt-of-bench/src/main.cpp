#include <cstring>
#include <sstream>
#include <omp.h>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include "json.hpp"

using json = nlohmann::json;

#include <mpi.h>

#include "ray-tracing.h"
#include "optical-flow.h"
#include "image.h"
#include "timer.h"


void parseCLA(int argc, char *const *argv, size_t &numLevels, size_t &supersampling, size_t &mgIterations,
              double &tStart, double &dt, size_t &numTimeSteps, size_t &numReps, bool &printImages, double &offset, double &scalingFactor) {
    // default values
    numLevels = 10;
    supersampling = 2;
    mgIterations = 10;
    dt = 1e-2;
    numTimeSteps = 4;
    numReps = 1;
    printImages = false;
    offset = 0.625;
    scalingFactor = 0.375;

    // override with command line arguments
    int i = 1;
    if (argc > i) numLevels = atoi(argv[i]);
    ++i;
    if (argc > i) supersampling = atoi(argv[i]);
    ++i;
    if (argc > i) mgIterations = atoi(argv[i]);
    ++i;
    if (argc > i) dt = atof(argv[i]);
    ++i;
    if (argc > i) numTimeSteps = atoi(argv[i]);
    ++i;
    if (argc > i) numReps = atoi(argv[i]);
    ++i;
    if (argc > i) printImages = atoi(argv[i]);
    ++i;
    if (argc > i) offset = atof(argv[i]);
    ++i;
    if (argc > i) scalingFactor = atof(argv[i]);
    ++i;


    int mpiRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    tStart = mpiRank * dt * numTimeSteps;
}


void logEvent(const std::string& eventName, int phase, int rank, int iteration, std::string color) {
    static auto startTime = std::chrono::steady_clock::now();
    static std::ofstream traceFile("trace_rank" + std::to_string(rank) + ".json");
    static bool initialized = false;
    static bool firstEvent = true;

    if (!initialized) {
        traceFile << "[\n";  // start of JSON array
        initialized = true;
    }

    auto now = std::chrono::steady_clock::now();
    double timestamp = std::chrono::duration<double, std::micro>(now - startTime).count();

    json traceEvent = {
        {"name", eventName},
        {"ph", (phase == 1 ? "B" : "E")},
        {"ts", timestamp},
        {"pid", 0},
        {"tid", rank},
        {"cname", color},
        {"args", {
            {"iteration", iteration}
        }}
    };

    if (!firstEvent) {
        traceFile << ",\n";
    }
    traceFile << traceEvent.dump();
    firstEvent = false;
}

void closeTraceFile(int mpiRank) {
    std::ofstream traceFile("trace_rank" + std::to_string(mpiRank) + ".json", std::ios_base::app);
    if (traceFile.is_open()) {
        traceFile << "\n]";  // Close the JSON array
        std::cerr << "Closing trace file for rank " << mpiRank << "\n";  // Debugging
        traceFile.close();   // Explicitly close the file
        std::cerr << "Trace file closed for rank " << mpiRank << "\n";  // Debugging
    } else {
        std::cerr << "Error: Could not open trace file for rank " << mpiRank << "\n";
    }
}


int main(int argc, char *argv[]) {
    
    MPI_Init(&argc, &argv);
    //int provided;
    //MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    //if (provided < MPI_THREAD_MULTIPLE) {
        //printf("Threading support insufficient\n");
        //MPI_Abort(MPI_COMM_WORLD, 1);
    //}

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_INIT;
#endif

    size_t numLevels, supersampling, mgIterations;
    double tStart, dt;
    size_t numTimeSteps, numReps;
    bool printImages;
    double offset, scalingFactor;
    parseCLA(argc, argv, numLevels, supersampling, mgIterations, tStart, dt, numTimeSteps, numReps, printImages, offset, scalingFactor);

    size_t nxOF = (1u << (numLevels - 1)) + 2;
    size_t nyOF = (1u << (numLevels - 1)) + 2;
    auto nxRT = supersampling * nxOF;
    auto nyRT = supersampling * nyOF;

    // allocate for ray tracer
    auto imgRT = new Color[nxRT * nyRT];
    auto imgPrintRT = new Color[nxOF * nyOF]; // image to print on (reduced) OF resolution

    // allocate for optical flow solver
    auto imgOF_0 = new double[nxOF * nyOF];
    auto imgOF_1 = new double[nxOF * nyOF];

    auto mgSolU = new double *[numLevels];
    auto mgSolV = new double *[numLevels];
    auto mgSolNewU = new double *[numLevels];
    auto mgSolNewV = new double *[numLevels];
    auto mgResU = new double *[numLevels];
    auto mgResV = new double *[numLevels];
    auto mgRhsU = new double *[numLevels];
    auto mgRhsV = new double *[numLevels];
    auto mgIxIx = new double *[numLevels];
    auto mgIxIy = new double *[numLevels];
    auto mgIyIy = new double *[numLevels];

    for (auto level = 0; level < numLevels; ++level) {
        auto nx = (1u << level) + 2;
        auto ny = (1u << level) + 2;

        for (auto ptr: {mgSolU, mgSolV, mgSolNewU, mgSolNewV, mgResU, mgResV, mgRhsU, mgRhsV, mgIxIx, mgIxIy, mgIyIy}) {
            ptr[level] = new double[nx * ny];
            memset(ptr[level], 0, nx * ny);
        }
    }

    // init MPI
    int mpiRank, numRanks;
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiRank);
    MPI_Comm_size(MPI_COMM_WORLD, &numRanks);

    MPI_Request mpiReq;
    auto mpiBuffer = new Color[nxRT * nyRT];

    // allocate spheres
    numSpheres = 32;
    spheres = static_cast<Sphere *>(malloc(numSpheres * sizeof(Sphere)));
    if (0 == mpiRank)
        initSpheres(offset, scalingFactor);
    MPI_Bcast(spheres, numSpheres * (sizeof(Sphere) / sizeof(double)), MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    // prepare timers
    Timer timerRT("ray-tracing"), timerMap("map"), timerOF("optical-flow"), timerMPISend("mpi-send"), timerMPIRecv("mpi-recv");

    double timetillIter;
    
    if(mpiRank ==0 )
        std::cout << "Rank" << "," << "NITER" << "," << "TimePerReps" << "," << "Performance" << std::endl;


    //WARMUP STEPS
    for (size_t rep = 0; rep < 10; ++rep) {
        for (size_t tIt = 0; tIt < 2; ++tIt) {
            auto t = tStart + tIt * dt + rep * dt * 1.0 * numRanks;

            updateSpherePositions(t);

            // measurement
            if (1 == tIt && !(10 - 1 == rep && numRanks - 1 == mpiRank)) {
                MPI_Recv(imgRT, nxRT * nyRT * sizeof(Color) / sizeof(double), MPI_DOUBLE, (mpiRank + numRanks + 1) % numRanks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            } else {
                createImage(nxRT, nyRT, t, imgRT);
            }

            if (0 == tIt && !(0 == mpiRank && 0 == rep)) {
                memcpy(mpiBuffer, imgRT, nxRT * nyRT * sizeof(Color));
                MPI_Isend(mpiBuffer, nxRT * nyRT * sizeof(Color) / sizeof(double), MPI_DOUBLE, (mpiRank + numRanks - 1) % numRanks, 0, MPI_COMM_WORLD, &mpiReq);
            }

            mapImageToDouble(nxOF, nyOF, nxRT, imgRT, imgOF_1, supersampling);
            std::swap(imgOF_0, imgOF_1);

            if (tIt > 0) {
                initGradientsAndRHS(nxOF, nyOF, dt, imgOF_0, imgOF_1, mgIxIx[numLevels - 1], mgIxIy[numLevels - 1], mgIyIy[numLevels - 1], mgRhsU[numLevels - 1], mgRhsV[numLevels - 1]);
                for (auto level = numLevels - 1; level >= 1; --level)
                    coarsenOperator((1u << (level - 1)) + 2, (1u << (level - 1)) + 2, (1u << level) + 2, (1u << level) + 2,
                                    mgIxIx[level], mgIxIy[level], mgIyIy[level], mgIxIx[level - 1], mgIxIy[level - 1], mgIyIy[level - 1]);

                for (auto mgIt = 0; mgIt < mgIterations; ++mgIt)
                    multigrid(numLevels - 1, 2. / 3., mgRhsU, mgRhsV, mgSolU, mgSolV, mgSolNewU, mgSolNewV, mgResU, mgResV, mgIxIx, mgIxIy, mgIyIy);
            }
        }
    }

   //std::cout << "End of WarmUp loop" << std::endl;

    MPI_Barrier(MPI_COMM_WORLD);

    //std::cout << "Start of original Iterations" << std::endl;

    for (size_t rep = 0; rep < numReps; ++rep) {
        auto startRepsTime = std::chrono::steady_clock::now(); 
        for (size_t tIt = 0; tIt < numTimeSteps + 1; ++tIt) {
            auto t = tStart + tIt * dt + rep * dt * numTimeSteps * numRanks;

            updateSpherePositions(t);

           // measurement
            if (numTimeSteps == tIt && !(numReps - 1 == rep && numRanks - 1 == mpiRank)) {
                timerMPIRecv.start();
                logEvent("Recv", 1, mpiRank, rep, "terrible");
                {
                    MPI_Recv(imgRT, nxRT * nyRT * sizeof(Color) / sizeof(double), MPI_DOUBLE, (mpiRank + numRanks + 1) % numRanks, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                }
                logEvent("Recv", 0, mpiRank, rep, "terrible");
                timerMPIRecv.stop();
            } else {
                timerRT.start();
                logEvent("RT", 1, mpiRank, rep, "cq_build_failed");
                {
                    createImage(nxRT, nyRT, t, imgRT);
                }
                logEvent("RT", 0, mpiRank, rep, "cq_build_failed");
                timerRT.stop();
            }

            if (0 == tIt && !(0 == mpiRank && 0 == rep)) {
                timerMPISend.start();
                logEvent("Send", 1, mpiRank, rep, "terrible");
                {
                    memcpy(mpiBuffer, imgRT, nxRT * nyRT * sizeof(Color));
                    MPI_Isend(mpiBuffer, nxRT * nyRT * sizeof(Color) / sizeof(double), MPI_DOUBLE, (mpiRank + numRanks - 1) % numRanks, 0, MPI_COMM_WORLD, &mpiReq);
                }
                logEvent("Send", 0, mpiRank, rep, "terrible");
                timerMPISend.stop();
            }

            timerMap.start();
            {
                mapImageToDouble(nxOF, nyOF, nxRT, imgRT, imgOF_1, supersampling);
                std::swap(imgOF_0, imgOF_1);
            }
            timerMap.stop();

            if (tIt > 0) {
                timerOF.start();
                logEvent("OF", 1, mpiRank, rep, "cq_build_running");
                {
                    initGradientsAndRHS(nxOF, nyOF, dt, imgOF_0, imgOF_1, mgIxIx[numLevels - 1], mgIxIy[numLevels - 1], mgIyIy[numLevels - 1], mgRhsU[numLevels - 1], mgRhsV[numLevels - 1]);
                    for (auto level = numLevels - 1; level >= 1; --level)
                        coarsenOperator((1u << (level - 1)) + 2, (1u << (level - 1)) + 2, (1u << level) + 2, (1u << level) + 2,
                                        mgIxIx[level], mgIxIy[level], mgIyIy[level], mgIxIx[level - 1], mgIxIy[level - 1], mgIyIy[level - 1]);

//                auto initRes = resNorm(nxOF, nyOF,
//                                       mgSolU[numLevels - 1], mgSolV[numLevels - 1],
//                                       mgIxIx[numLevels - 1], mgIxIy[numLevels - 1], mgIyIy[numLevels - 1],
//                                       mgRhsU[numLevels - 1], mgRhsV[numLevels - 1]);

                    for (auto mgIt = 0; mgIt < mgIterations; ++mgIt)
                        multigrid(numLevels - 1, 2. / 3., mgRhsU, mgRhsV, mgSolU, mgSolV, mgSolNewU, mgSolNewV, mgResU, mgResV, mgIxIx, mgIxIy, mgIyIy);

//                auto res = resNorm(nxOF, nyOF,
//                                   mgSolU[numLevels - 1], mgSolV[numLevels - 1],
//                                   mgIxIx[numLevels - 1], mgIxIy[numLevels - 1], mgIyIy[numLevels - 1],
//                                   mgRhsU[numLevels - 1], mgRhsV[numLevels - 1]);
//                std::cout << "Residual before / after " << mgIterations << " : " << initRes << " / " << res << std::endl;
                }
                logEvent("OF", 0, mpiRank, rep, "cq_build_running");
                timerOF.stop();
            }

            // print images
            if (printImages) {
                if (tIt < numTimeSteps) {
                    mapImageToColor(nxOF, nyOF, nxRT, imgRT, imgPrintRT, supersampling);

                    std::stringstream filename;
                    filename << "../imageData/ray-tracing-" << std::setw(5) << std::setfill('0') << std::right << t / dt << ".raw";
                    printImage(nxOF, nyOF, imgPrintRT, filename.str());
                }
                if (tIt > 0) {
                    std::stringstream filename;
                    filename << "../imageData/optical-flow-" << std::setw(5) << std::setfill('0') << std::right << t / dt << ".raw";
                    printImage(nxOF, nyOF, mgSolU[numLevels - 1], mgSolV[numLevels - 1], filename.str());
                }
            }
        }

        if (mpiRank > 0 || rep > 0) {
            logEvent("Wait", 1, mpiRank, rep, "terrible");
            MPI_Wait(&mpiReq, MPI_STATUS_IGNORE);
            logEvent("Wait", 0, mpiRank, rep, "terrible");
            mpiReq = MPI_REQUEST_NULL;
        }

        auto endRepsTime = std::chrono::steady_clock::now();
        std::chrono::duration<double> duration = endRepsTime - startRepsTime;
        double timeperIter = duration.count();
        for (int rank = 0; rank < numRanks; ++rank) {
            if (mpiRank == rank)
                fprintf(stdout, "%d,%d,%.6f,%.6f\n", mpiRank, rep, timeperIter, 1.0/timeperIter);
                fflush(stdout);
                //std::cout << mpiRank << "," << rep << "," << timeperIter << "," << 1.0/timeperIter << std::endl;
        }

    }

    closeTraceFile(mpiRank);
    
    for (auto timer: {timerRT, timerOF, timerMap, timerMPISend, timerMPIRecv})
        timer.gatherAndPrint(mpiRank, numRanks);

    // de-allocate MPI
    delete[] mpiBuffer;

    // de-allocate optical flow solver
    delete[] imgOF_0;
    delete[] imgOF_1;

    for (auto ptr: {mgSolU, mgSolV, mgSolNewU, mgSolNewV,
                    mgResU, mgResV, mgRhsU, mgRhsV, mgIxIx,
                    mgIxIy, mgIyIy}) {
        for (auto level = 0; level < numLevels; ++level)
            delete[] ptr[level];
        delete[] ptr;
    }

    // de-allocate ray tracer
    delete[] imgRT;
    delete[] imgPrintRT;

    free(spheres);

    MPI_Finalize();

#ifdef LIKWID_PERFMON
    LIKWID_MARKER_CLOSE;
#endif

    return 0;
}
