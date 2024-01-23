/*
 * Copyright (C) 2022 NHR@FAU, University Erlangen-Nuremberg.
 * All rights reserved.
 * Use of this source code is governed by a MIT-style
 * license that can be found in the LICENSE file.
 */

// Command line to compile this program :  mpicc -O3 -lm poissonNS.c -o sample

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <mpi.h>
#include <errno.h>
#include <time.h>



#define PI 3.14159265358979323846
#define P(i,j) p[(j)*(imax+2) + (i)]
#define RHS(i,j) rhs[(j)*(imax+2) + (i)]

#ifndef MIN
#define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
#define MAX(x,y) ((x)>(y)?(x):(y))
#endif
#ifndef ABS
#define ABS(a) ((a) >= 0 ? (a) : -(a))
#endif

struct Solver 
{
    double  dx, dy;
    double  ys;
    int imax, jmax;
    int jmaxLocal;
    int rank;
    int size;
    double xlength, ylength;

    double *p, *rhs;
    double eps, omega, rho;
    int itermax;
};

static double 
getTimeStamp()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9;
}

static double 
getTimeResolution()
{
    struct timespec ts;
    clock_getres(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1.e-9;
}

static int 
sizeOfRank(int rank, int size, int N)
{
    return N/size + ((N%size>rank) ? 1 : 0);
}

static void
initSolver(struct Solver *solver)
{
    MPI_Comm_rank(MPI_COMM_WORLD, &(solver->rank));
    MPI_Comm_size(MPI_COMM_WORLD, &(solver->size));

    solver->xlength = 1.0;
    solver->ylength = 1.0;

    solver->eps = 0.000001;
    solver->omega = 1.991;

    solver->jmaxLocal = sizeOfRank(solver->rank, solver->size, solver->jmax);


    solver->dx = solver->xlength/solver->imax;
    solver->dy = solver->ylength/solver->jmax;
    solver->ys =  solver->rank * solver->jmaxLocal * solver->dy;

    int imax = solver->imax;
    int jmax = solver->jmax;
    int jmaxLocal = solver->jmaxLocal;

    printf("Rank : %d, %.2f GB allocated for P. %.2f GB allocated for RHS\n",solver->rank, ((imax+2) * (jmaxLocal+2) * sizeof(double))/(1024.0*1024.0*1024.0), ((imax+2) * (jmax+2) * sizeof(double))/(1024.0*1024.0*1024.0));


    void *ptr, *ptr2;
    posix_memalign(&ptr, 64, (imax+2) * (jmaxLocal+2) * sizeof(double));

    posix_memalign(&ptr2, 64, (imax+2) * (jmax+2) * sizeof(double));


    solver->p = ptr;
    solver->rhs = ptr2;

    double dx = solver->dx;
    double dy = solver->dy;
    double* p = solver->p;
    double* rhs = solver->rhs;

    for( int j=0; j<jmaxLocal+2; j++ ) 
    {
        double y = solver->ys + j * dy;
        for( int i=0; i<imax+2; i++ ) 
        {
            P(i,j) = sin(4.0*PI*i*dx)+sin(4.0*PI*y);
        }
    }

    for( int j=0; j<jmax+2; j++ ) 
    {
        for( int i=0; i<imax+2; i++ ) 
        {
            RHS(i,j) = sin(2.0*PI*i*dx);
        }
    }
    
}

static void 
exchange(struct Solver* solver)
{
    MPI_Request requests[4] = { MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL, MPI_REQUEST_NULL };
    MPI_Status status;
	int imax = solver->imax;
	int jmaxLocal = solver->jmax/solver->size + ((solver->jmax%solver->size>solver->rank) ? 1 : 0);
	double *src = solver->p;
    double *dst = solver->p;
	double dx = solver->xlength/solver->imax;
	double dy = solver->ylength/solver->jmax;
    double S, E;

    S = getTimeStamp();
    /* exchange ghost cells with top neighbor */
    if (solver->rank + 1 < solver->size) {
        int top = solver->rank + 1;
        
        // printf("Sending from %d to %d\n", solver->rank, top);
        // for(int i = 0; i<solver->imax;++i)
        // {
        //     printf("%.2f ", src[(solver->jmaxLocal) * (solver->imax+2) + 1 + i]);
        // }
        // printf("\n");

        MPI_Isend(&src[(solver->jmaxLocal) * (solver->imax+2) + 1], solver->imax, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &requests[0]);
        MPI_Irecv(&dst[(solver->jmaxLocal+1) * (solver->imax+2) + 1], solver->imax, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, &requests[1]);
        
        // MPI_Send(&src[(solver->jmaxLocal) * (solver->imax+2) + 1], solver->imax, MPI_DOUBLE, top, 1, MPI_COMM_WORLD);



        // MPI_Recv(&dst[(solver->jmaxLocal+1) * (solver->imax+2) + 1], solver->imax, MPI_DOUBLE, top, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // printf("Receiving from %d to %d\n", top, solver->rank);
        // for(int i = 0; i<solver->imax;++i)
        // {
        //     printf("%.2f ", dst[(solver->jmaxLocal+1) * (solver->imax+2) + 1 + i]);
        // }
        // printf("\n");
    }

    /* exchange ghost cells with bottom neighbor */
    if (solver->rank > 0) {
        int bottom = solver->rank - 1;


        // printf("Sending from %d to %d\n", solver->rank, bottom);
        // for(int i = 0; i<solver->imax;++i)
        // {
        //     printf("%.2f ", src[(solver->imax+2) + 1 + i]);
        // }
        // printf("\n");

        MPI_Isend(&src[(solver->imax+2) + 1],  solver->imax, MPI_DOUBLE, bottom, 2, MPI_COMM_WORLD,&requests[2]);
        MPI_Irecv(&dst[1],  solver->imax, MPI_DOUBLE, bottom, 1, MPI_COMM_WORLD, &requests[3]);

        // printf("Receiving from %d to %d\n", bottom, solver->rank);
        // for(int i = 0; i<solver->imax;++i)
        // {
        //     printf("%.2f ", dst[1 + i]);
        // }
        // printf("\n");
    }

    MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);

    E = getTimeStamp();
    // if(solver->rank == 1 ) printf("Rank : %d, 1 exchange takes : %.15f seconds\n", solver->rank, E-S);
}

static double
kernel(struct Solver *solver)
{
    double r1 = 0.0;
    double res = 0.0;

    int imax = solver->imax;
    int jmax = solver->jmax;
    int jmaxLocal = solver->jmaxLocal;
    double dx = solver->dx;
    double dy = solver->dx;
    double dx2 = dx * dx;
    double dy2 = dy * dy;
    double idx2 = 1.0 / dx2;
    double idy2 = 1.0 / dy2;
    double omega = solver->omega;
    double factor = omega * 0.5 * ( dx2 * dy2 ) / ( dx2 + dy2 );
    double *src = solver->p;
    double *rhs = solver->rhs;

    for( int j=1; j<jmaxLocal+1; j++ ) 
    {
        for( int i=1; i<imax+1; i++ ) 
        {
            r1 = rhs[(j)*(imax+2) + (i)] - ((src[(j)*(imax+2) + (i-1)] - 2.0 * src[(j)*(imax+2) + (i)] + src[(j)*(imax+2) + (i+1)]) * idx2 +
                    (src[(j-1)*(imax+2) + (i)] - 2.0 * src[(j)*(imax+2) + (i)] + src[(j+1)*(imax+2) + (i)]) * idy2);

            src[(j)*(imax+2) + (i)] = src[(j)*(imax+2) + (i)] - (factor * r1);
            res = res + (r1 * r1);
        }
    }
    return res;
}

static int 
solve(struct Solver *solver)
{
    double r;
    int it = 0;
    double res, res1;

    int imax = solver->imax;
    int jmax = solver->jmax;
    int jmaxLocal = solver->jmaxLocal = sizeOfRank(solver->rank, solver->size, solver->jmax);
    double eps= solver->eps;
    double omega = solver->omega;
    int itermax = solver->itermax;

    double dx2 = solver->dx * solver->dx;
    double dy2 = solver->dy * solver->dy;
    double idx2 = 1.0/dx2;
    double idy2 = 1.0/dy2;
    double factor = omega * 0.5 * (dx2*dy2) / (dx2+dy2);
    double *src = solver->p;
    double *rhs = solver->rhs;
    double epssq  = eps * eps;
    double S, E;

    res = eps + 1.0;

    printf("Rank : %d, imax : %d, jmaxLocal : %d\n",solver->rank, solver->imax, solver->jmaxLocal);

    //while((res >= epssq) && (it < itermax)) 
    for(it = 0; it < itermax ; ++it)
    {
        res = 0.0;
		exchange(solver);
        S = getTimeStamp();

        res = kernel(solver);
        
        E = getTimeStamp();
        if(solver->rank == 1 ) printf("Rank : %d, 1 iteration takes : %.8f seconds\n", solver->rank, E-S);
        if ( solver->rank == 0 ) 
        {
            for( int i=1; i<imax+1; i++ ) 
            {
                src[(0) * (imax+2) + (i)] = src[(1) * (imax+2) + (i)];
            }
        }

        if ( solver->rank == (solver->size-1) ) 
        {
            for( int i=1; i<imax+1; i++ ) 
            {
                src[(jmaxLocal+1) * (imax+2) + (i)] = src[(jmaxLocal) * (imax+2) + (i)];
            }
        }

        for( int j=1; j<jmaxLocal+1; j++ ) 
        {
            src[(j) * (imax+2) + (0)] = src[(j) * (imax+2) + (1)];
            src[(j) * (imax+2) + (imax+1)] = src[(j) * (imax+2) + (imax)];
        }

        // MPI_Allreduce(&res, &res1, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
        res = res1;
        res = sqrt(res / (imax*jmax));

        if(res <= -1.0)
        {
            break;
        }
        // ++it;
    }
}

int main (int argc, char** argv)
{
    int rank;
    double S, E;

    struct Solver solver;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    solver.imax = 1000;
    solver.jmax = 1000;
    solver.itermax = 5;

    if (argc > 1)
    {
        solver.imax = atoi(argv[1]);
    }

    if (argc > 2)
    {
        solver.jmax = atoi(argv[2]);
    }

    if (argc > 3)
    {
        solver.itermax = atoi(argv[3]);
    }   

    initSolver(&solver);
    MPI_Barrier(MPI_COMM_WORLD);

    S = getTimeStamp();

    solve(&solver);

    E = getTimeStamp();
    if (rank == 0) {
        printf("Solution took %.2fs\n", E - S);
        double metric = ((double)solver.imax * solver.jmax * solver.itermax) / ((E-S) * 1e9);
        printf("GLUP/s : %.5f\n", metric);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
