/*****************************************************************************
* 
* Copyright (c) 2018, University of Ferrara, University of Rome and INFN. All rights reserved.
* 
* 
* Authors listed in alphabetic order:
* ----------------------------------
* Luca Biferale (University of Rome "Tor Vergata" and INFN)
* Enrico Calore (University of Ferrara and INFN)
* Alessandro Gabbana (University of Ferrara and INFN)
* Mauro Sbragaglia (University of Rome "Tor Vergata" and INFN)
* Andrea Scagliarini (CNR-IAC, Roma)
* Sebastiano Fabio Schifano (University of Ferrara and INFN)(*)
* Raffaele Tripiccione (University of Ferrara and INFN)
* 
* (*) corresponding author, sebastiano.schifano __at__ unife.it
* 
* We also thanks the following students:
* -------------------------------------
* Elisa Pellegrini (University of Ferrara)
* Marco Zanella (University of Ferrara)
* 
* 
* Relevant pubblications:
* ----------------------
* 
* - Sbragaglia, M., Benzi, R., Biferale, L., Chen, H., Shan, X., Succi, S.
*   Lattice Boltzmann method with self-consistent thermo-hydrodynamic equilibria
*   (2009) Journal of Fluid Mechanics, 628, pp. 299-309. Cited 66 times.
*   https://www.scopus.com/inward/record.uri?eid=2-s2.0-67651027859&doi=10.1017%2fS002211200900665X&partnerID=40&md5=1c57f2de4c0abc5aacaf5e30e9d566d7
*   DOI: 10.1017/S002211200900665X
* 
* - Scagliarini, A., Biferale, L., Sbragaglia, M., Sugiyama, K., Toschi, F.
*   Lattice Boltzmann methods for thermal flows: Continuum limit and applications to compressible Rayleigh-Taylor systems
*   (2010) Physics of Fluids, 22 (5), art. no. 019004PHF, pp. 1-21. Cited 51 times.
*   https://www.scopus.com/inward/record.uri?eid=2-s2.0-77955618379&doi=10.1063%2f1.3392774&partnerID=40&md5=8b47e1b6c7ffd95583e2dc30d36a66f2
*   DOI: 10.1063/1.3392774
* 
* - Calore, E., Gabbana, A., Kraus, J., Pellegrini, E., Schifano, S.F., Tripiccione, R.
*   Massively parallel lattice–Boltzmann codes on large GPU clusters
*   (2016) Parallel Computing, 58, pp. 1-24.
*   https://www.scopus.com/inward/record.uri?eid=2-s2.0-84983616316&doi=10.1016%2fj.parco.2016.08.005&partnerID=40&md5=a9b40226e16f6e24bc8bc7138b789f90
*   DOI: 10.1016/j.parco.2016.08.005
* 
* - Calore, E., Gabbana, A., Kraus, J., Schifano, S.F., Tripiccione, R.
*   Performance and portability of accelerated lattice Boltzmann applications with OpenACC
*   (2016) Concurrency Computation, 28 (12), pp. 3485-3502.
*   https://www.scopus.com/inward/record.uri?eid=2-s2.0-84971215878&doi=10.1002%2fcpe.3862&partnerID=40&md5=fe286342abec002fa287da5967b9d2c2
*   DOI: 10.1002/cpe.3862
* 
* 
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
* 
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
* 
*   * Neither the name of the above Institutions nor the names of their
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* 
*******************************************************************************/

#ifdef SPEC_CUDA
#include "lbm.cu"
#else
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <mpi.h>
#include <math.h>

#include "common.h"
#include "likwid.h"

#ifdef _OPENACC
#include <openacc.h>
#endif

extern int mpi_rank;          // Rank of MPI node.

extern int n_mpi;             // total number of MPI ranks == n_mpi_x * n_mpi_y == mpi_size

extern int n_mpi_x;           // number of MPI ranks along X direction
extern int n_mpi_y;           // number of MPI ranks alonf Y direction

extern int mpi_rank_x;        // MPI X coordinate.
extern int mpi_rank_y;        // MPI Y coordinate.

extern int mpi_rank;          // Rank of MPI node.
extern int mpi_size;          // Size of MPI world communicator.

extern int n_mpi;             // total number of MPI ranks == n_mpi_x * n_mpi_y == mpi_size

extern int n_mpi_x;           // number of MPI ranks along X direction
extern int n_mpi_y;           // number of MPI ranks alonf Y direction

extern int mpi_rank_x;        // MPI X coordinate.
extern int mpi_rank_y;        // MPI Y coordinate.

extern int mpi_peer_left, mpi_peer_right, mpi_peer_top, mpi_peer_bot; // Rank of neighboring MPI ranks

extern char * mpi_order;      // MPI ranks order string

extern int GSIZEX, GSIZEY;    // global lattice sizex and sizex
extern int LSIZEX, LSIZEY;    // local lattice sizex and sizex
extern uint64_t NX, NY;       // x-length and y-lenght of lattice in memory.

extern long int SEED;                // seed for random generator

extern const size_t OFF0[37];

extern size_t OFF[NPOP];

extern const int bits;

#include "utils.h"

#include "propagate_kernel.h"
#include "bc_kernel.h"
#include "collide_kernel.h"
#include "packunpack.h"

///////////////////////////////////////////////////////////////////////////////

#define NFLOP 6613

#if defined(USE_TARGET)
  int Q1[1];
  int Q2[1];
  int Q3[1];
  int Q4[1];
  int Q5[1];
#else
  #define Q1 (1)
  #define Q2 (2)
  #define Q3 (3)
  #define Q4 (4)
  #define Q5 (5)
#endif

///////////////////////////////////////////////////////////////////////////////
// Function used to launch kernels.

void lbm ( int niter, pop_t *f_aos_h, par_t *param ) {

  long ii, yoff; //  ip, ix not used;

  data_t *f1_soa_h = NULL, *f2_soa_h = NULL, *f_tmp_h = NULL; 

  data_t *topSndBuf = NULL, *topRxBuf = NULL, *botSndBuf = NULL, *botRxBuf = NULL;
  data_t *leftSndBuf = NULL, *leftRxBuf = NULL, *rightSndBuf = NULL, *rightRxBuf = NULL;

  data_t mass, i_mass, g_mass;

  // MPI_Status status;

  MPI_Request reqRxTopHalo, reqTxTopBorder, reqRxBotHalo, reqTxBotBorder;
  MPI_Request reqRxLeftHalo, reqTxRightBorder, reqRxRightHalo, reqTxLeftHalo;

  int tagUP=1, tagDOWN=2, tagLEFT=3, tagRIGHT=4;

  //  double mpimin_dt_tot, mpimax_dt_tot;
  //  double mpimax_dt_swap_nc, mpimin_dt_swap_nc, double mpimax_dt_swap_c, mpimin_dt_swap_c;

  double mpimax_dt_tot, mpimax_dt_swap_nc, mpimax_dt_swap_c;
  double t1, t2;
#ifndef SPEC 
  struct timeval ti[5], tf[5];
  double dT[5];
#endif
  struct timeval ttoti[1], ttotf[1];
  //double latticesize; //, bw, p;
  double mlups;

  LIKWID_MARKER_INIT;
  LIKWID_MARKER_REGISTER("c_kernel");

  /////////////////////////////////////////////////////////////////////////////

  posix_memalign((void *)&topSndBuf,   4096, (15+8+3)*LSIZEX*sizeof(data_t) );
  posix_memalign((void *)&topRxBuf,    4096, (15+8+3)*LSIZEX*sizeof(data_t) );
  posix_memalign((void *)&botSndBuf,   4096, (15+8+3)*LSIZEX*sizeof(data_t) );
  posix_memalign((void *)&botRxBuf,    4096, (15+8+3)*LSIZEX*sizeof(data_t) );

  posix_memalign((void *)&leftSndBuf,  4096, (15+8+3)*NY*sizeof(data_t) );
  posix_memalign((void *)&leftRxBuf,   4096, (15+8+3)*NY*sizeof(data_t) );
  posix_memalign((void *)&rightSndBuf, 4096, (15+8+3)*NY*sizeof(data_t) );
  posix_memalign((void *)&rightRxBuf,  4096, (15+8+3)*NY*sizeof(data_t) );

  if ( (topSndBuf==NULL)  || (topRxBuf==NULL)  || (botSndBuf==NULL)   || (botRxBuf==NULL) || 
       (leftSndBuf==NULL) || (leftRxBuf==NULL) || (rightSndBuf==NULL) || (rightRxBuf==NULL) ) {
    fprintf(stderr, "ERROR: allocation of MPI buffers failed\n");
    exit(-1);
  }

  /////////////////////////////////////////////////////////////////////////////
  unsigned long nx=NX;
  unsigned long ny=NY;
  unsigned long npop=NPOP;
  
  unsigned long nelem = nx*ny*npop;
  unsigned long size = nelem*sizeof(data_t);

  posix_memalign((void *)&f1_soa_h, 4096, size ); 


  if(f1_soa_h == NULL){
    fprintf(stderr, "ERROR: f1_soa_h posix_memalign failed\n");
    exit(-1);
  }

  posix_memalign((void *)&f2_soa_h, 4096, size );
  if(f2_soa_h == NULL){
    fprintf(stderr, "ERROR: f2_soa_h posix_memalign failed\n");
    exit(-1);
  }
#ifdef SPEC_OPENACC
#pragma acc enter data create(                                 \
    f1_soa_h[0:nelem], f2_soa_h[0:nelem])                      \
    copyin(leftSndBuf[0:(15+8+3)*NY], leftRxBuf[0:(15+8+3)*NY],\
    rightSndBuf[0:(15+8+3)*NY], rightRxBuf[0:(15+8+3)*NY],     \
    topSndBuf[0:(15+8+3)*LSIZEX], topRxBuf[0:(15+8+3)*LSIZEX], \
    botSndBuf[0:(15+8+3)*LSIZEX], botRxBuf[0:(15+8+3)*LSIZEX], \
    OFF0[0:37], OFF[0:37])
#elif defined(USE_TARGET)
#pragma omp target enter data                                  \
    map(alloc:f1_soa_h[0:nelem], f2_soa_h[0:nelem]),           \
    map(to:leftSndBuf[0:(15+8+3)*NY], leftRxBuf[0:(15+8+3)*NY],\
    rightSndBuf[0:(15+8+3)*NY], rightRxBuf[0:(15+8+3)*NY],     \
    topSndBuf[0:(15+8+3)*LSIZEX], topRxBuf[0:(15+8+3)*LSIZEX], \
    botSndBuf[0:(15+8+3)*LSIZEX], botRxBuf[0:(15+8+3)*LSIZEX], \
    OFF0[0:37], OFF[0:37])
#endif

#if defined(SPEC_OPENACC) || defined(USE_TARGET)
#ifdef SPEC_OPENACC
#pragma acc parallel loop present(2) present(f1_soa_h,f2_soa_h)
#elif defined(USE_TARGET)
#pragma omp target teams OMP_SCHED
#endif
  for (int i = 0; i < nelem; ++i) {
	f1_soa_h[i]=0.0;
	f2_soa_h[i]=0.0;
  }
#else
  memset(f1_soa_h, 0x0, size);
  memset(f2_soa_h, 0x0, size);
#endif
  /////////////////////////////////////////////////////////////////////////////

  AoStoSoA(f_aos_h, f2_soa_h); // f_aos_h --> f2_soa_h
#ifdef SPEC_OPENACC
  #pragma acc host_data use_device(f1_soa_h, f2_soa_h) 
  {
     acc_memcpy_device(f1_soa_h, f2_soa_h, size);
  }
#elif defined(USE_TARGET)
  int devnum=omp_get_device_num();
  omp_target_memcpy(f1_soa_h, f2_soa_h, size, 0, 0, devnum, devnum);
#else
  memcpy (f1_soa_h, f2_soa_h, size);
#endif

  /////////////////////////////////////////////////////////////////////////////
#ifndef SPEC
  // Delta-times are initialized.
  dT[0] = 0.0;
  dT[1] = 0.0;
  dT[2] = 0.0;
  dT[3] = 0.0;
  dT[4] = 0.0;
#endif 

  yoff = mpi_rank_y * (GSIZEY / n_mpi_y); 
#ifdef CHECK_MASS
  mass = dumpMass(f_aos_h);
  MPI_Reduce(&mass, &i_mass, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
#endif

  MPI_Barrier(MPI_COMM_WORLD);

  /////////////////////////////////////////////////////////////////////////////

#ifndef SPEC
    gettimeofday(&ti[0], NULL);
#endif

    if (mpi_rank == 0) {
        gettimeofday(&ttoti[0], NULL);
    }
    for(ii = 1; ii <= niter; ii++) {

      ///////////////////////////////////////////////////////////////////////////
#ifndef SPEC
      gettimeofday(&ti[1], NULL);
#endif
     
#ifdef SPEC
      if (mpi_rank == 0) {
        printf("Starting iteration: %d\n",ii);
      }
#endif

      ///////////////////////////////////////////////////////////////////////////
      // When using a 2D domain partitioning there is some extra work to be done
      ///////////////////////////////////////////////////////////////////////////
      if ( n_mpi_y > 1 ) {
        // update top and bottom halos

        ///////////////////////////////////////////////////////////////////////////
        // 1) We start by packing the top and the bottom borders which need to be
        //    exchanged among the appropriate MPI-ranks

        // 0 <= mpi_rank_y < n_mpi_y-1
        if ( mpi_rank_y < (n_mpi_y-1) ){ 

          pack_top_border ( topSndBuf, f2_soa_h, Q1 );

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
          #pragma acc update self(topSndBuf[0:(15+8+3)*LSIZEX]) async(Q1)
          
#elif defined(USE_TARGET)
          #pragma omp target update from(topSndBuf[0:(15+8+3)*LSIZEX])
#endif
        }

        // 0 < mpi_rank_y <= (n_mpi_y-1)  
        if ( mpi_rank_y > 0 ){

          pack_bot_border ( botSndBuf, f2_soa_h, Q2 );

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
          #pragma acc update self(botSndBuf[0:(15+8+3)*LSIZEX]) async(Q2)

#elif defined(USE_TARGET)
          #pragma omp target update from(botSndBuf[0:(15+8+3)*LSIZEX]) 
#endif
        }

        ///////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////
        // 2) We initiate the asyncronous MPI-comm for 0 <= mpi_rank_y < n_mpi_y-1

        if ( mpi_rank_y < (n_mpi_y-1) ) { 

#if defined(SPEC_OPENACC) 
          #pragma acc wait(Q1)
#if defined(SPEC_ACCEL_AWARE_MPI) 
          #pragma acc host_data use_device(topSndBuf, topRxBuf)
#endif
#endif
          {
            MPI_Irecv ( topRxBuf,  (15+8+3)*LSIZEX, MPI_DOUBLE, mpi_peer_top, tagDOWN, MPI_COMM_WORLD, &reqRxTopHalo   );

            MPI_Isend ( topSndBuf, (15+8+3)*LSIZEX, MPI_DOUBLE, mpi_peer_top, tagUP,   MPI_COMM_WORLD, &reqTxTopBorder );
          }

        }
        ///////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////
        // 3) We initiate the asyncronous MPI-comm for 0 < mpi_rank_y <= (n_mpi_y-1)

        if ( mpi_rank_y > 0 ) {

#if defined(SPEC_OPENACC) 
          #pragma acc wait(Q2)

#if defined(SPEC_ACCEL_AWARE_MPI) 
          #pragma acc host_data use_device(botSndBuf, botRxBuf)
#endif
#endif  
          {            
            MPI_Irecv ( botRxBuf,  (15+8+3)*LSIZEX, MPI_DOUBLE, mpi_peer_bot, tagUP,   MPI_COMM_WORLD, &reqRxBotHalo   );

            MPI_Isend ( botSndBuf, (15+8+3)*LSIZEX, MPI_DOUBLE, mpi_peer_bot, tagDOWN, MPI_COMM_WORLD, &reqTxBotBorder );
          }

        }
        ///////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////
        // 4) We wait for MPI-comm to finish for 0 <= mpi_rank_y < n_mpi_y-1 
        //    and copy back the top-halo on the device memory

        if ( mpi_rank_y < (n_mpi_y-1) ) { // 0 <= mpi_rank_y < n_mpi_y-1

          MPI_Wait ( &reqTxTopBorder, MPI_STATUS_IGNORE );  
          MPI_Wait ( &reqRxTopHalo  , MPI_STATUS_IGNORE ); 

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
          #pragma acc update device(topRxBuf[0:(15+8+3)*LSIZEX]) async(Q1)
#elif defined(USE_TARGET)
          #pragma omp target update  to(topRxBuf[0:(15+8+3)*LSIZEX]) 
#endif

          unpack_top_halo ( f2_soa_h, topRxBuf, Q1 );
        }
        ///////////////////////////////////////////////////////////////////////////

        ///////////////////////////////////////////////////////////////////////////
        // 5) We wait for MPI-comm to finish for 0 < mpi_rank_y <= (n_mpi_y-1)
        //    and copy back the bottom-halo on the device memory

        if ( mpi_rank_y > 0 ) { 
          
          MPI_Wait ( &reqTxBotBorder, MPI_STATUS_IGNORE ); 
          MPI_Wait ( &reqRxBotHalo  , MPI_STATUS_IGNORE ); 

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
          #pragma acc update device(botRxBuf[0:(15+8+3)*LSIZEX]) async(Q2)
#elif defined(USE_TARGET)
          #pragma omp target update to(botRxBuf[0:(15+8+3)*LSIZEX]) 
#endif

          unpack_bot_halo ( f2_soa_h, botRxBuf, Q2 );
        }

      } // end ( n_mpi_y > 1 )

      // That's it for the 2D-domain partitioning case
      ///////////////////////////////////////////////////////////////////////////

#if defined(SPEC_OPENACC)
      #pragma acc wait(Q1, Q2)
#endif 

#ifndef SPEC
      gettimeofday(&tf[1], NULL);
#endif

      ////////////////////////////////////////////////////////////////////
      // update left and right halos ...

      pack_right_border ( leftSndBuf, f2_soa_h, Q1 );

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
      #pragma acc update self( leftSndBuf[0:(15+8+3)*NY]) async(Q1)
#elif defined(USE_TARGET)
      #pragma omp target update from( leftSndBuf[0:(15+8+3)*NY]) 
#endif

      pack_left_border ( rightSndBuf, f2_soa_h, Q2 );

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
      #pragma acc update self(rightSndBuf[0:(15+8+3)*NY]) async(Q2)
#elif defined(USE_TARGET)
      #pragma omp target update from(rightSndBuf[0:(15+8+3)*NY])       
#endif

      ///////////////////////////////////////////////////////////////////////////
      // run propagate and collide on Bulk
      LIKWID_MARKER_START("c_kernel");
      PROPAGATECOLLIDE(f1_soa_h, f2_soa_h, (HX+hx), (HX+LSIZEX-hx), (HY+hy+1), (HY+LSIZEY-hy-1), yoff, Q3);
      LIKWID_MARKER_STOP("c_kernel");

      ///////////////////////////////////////////////////////////////////////////

#ifndef SPEC
      gettimeofday(&ti[2], NULL);
#endif

      ///////////////////////////////////////////////////////////////////////////
      // MPI-Comm Right-Border

#if defined(SPEC_OPENACC) 
      #pragma acc wait(Q1)

#if defined(SPEC_ACCEL_AWARE_MPI) 
      #pragma acc host_data use_device(leftSndBuf,leftRxBuf)
#endif
#endif        
      {  
      MPI_Irecv ( leftRxBuf,  (15+8+3)*NY, MPI_DOUBLE, mpi_peer_left,  tagLEFT, MPI_COMM_WORLD, &reqRxLeftHalo    );

      MPI_Isend ( leftSndBuf, (15+8+3)*NY, MPI_DOUBLE, mpi_peer_right, tagLEFT, MPI_COMM_WORLD, &reqTxRightBorder );
      }

      ///////////////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////////////
      // MPI-Comm Left-Border

#if defined(SPEC_OPENACC) 
      #pragma acc wait(Q2)

#if defined(SPEC_ACCEL_AWARE_MPI) 
      #pragma acc host_data use_device(rightSndBuf,rightRxBuf)
#endif
#endif       
      {  
      MPI_Irecv ( rightRxBuf,  (15+8+3)*NY, MPI_DOUBLE, mpi_peer_right, tagRIGHT, MPI_COMM_WORLD, &reqRxRightHalo );

      MPI_Isend ( rightSndBuf, (15+8+3)*NY, MPI_DOUBLE, mpi_peer_left,  tagRIGHT, MPI_COMM_WORLD, &reqTxLeftHalo  );
      }

      ///////////////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////////////
      // Wait right-border Comm and tranfer back to device

      MPI_Wait ( &reqTxRightBorder, MPI_STATUS_IGNORE );
      MPI_Wait ( &reqRxLeftHalo   , MPI_STATUS_IGNORE );

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
      #pragma acc update device( leftRxBuf[0:(15+8+3)*NY]) async(Q1)

#elif defined(USE_TARGET)
      #pragma omp target update to( leftRxBuf[0:(15+8+3)*NY]) 
#endif

      unpack_left_halo  ( f2_soa_h,  leftRxBuf, Q1 ); 

      ///////////////////////////////////////////////////////////////////////////

      ///////////////////////////////////////////////////////////////////////////
      // Wait left-border Comm and tranfer back to device

      MPI_Wait ( &reqTxLeftHalo , MPI_STATUS_IGNORE );
      MPI_Wait ( &reqRxRightHalo, MPI_STATUS_IGNORE );

#if defined(SPEC_OPENACC) && !defined(SPEC_ACCEL_AWARE_MPI)
      #pragma acc update device(rightRxBuf[0:(15+8+3)*NY]) async(Q2)

#elif defined(USE_TARGET)
      #pragma omp target update to(rightRxBuf[0:(15+8+3)*NY])   
#endif

      unpack_right_halo ( f2_soa_h, rightRxBuf, Q2 );

      ///////////////////////////////////////////////////////////////////////////

#ifndef SPEC
      gettimeofday(&tf[2], NULL);
#endif

      ///////////////////////////////////////////////////////////////////////////
      // Wait END of left and righ unpack kernels: processing of TOP and BOTTOM 
      // borders require that unpack of left and right halos has been fully completed  
      ///////////////////////////////////////////////////////////////////////////

#if defined(SPEC_OPENACC)
      #pragma acc wait(Q1, Q2)
#endif 

      // propagate and collide on Left border

      LIKWID_MARKER_START("c_kernel");
      PROPAGATECOLLIDE(f1_soa_h, f2_soa_h, (HX), (HX+hx), (HY+hy+1), (HY+LSIZEY-hy-1), yoff, Q1);

      // propagate and collide on Righ border

      PROPAGATECOLLIDE(f1_soa_h, f2_soa_h, (HX+LSIZEX-hx), (HX+LSIZEX), (HY+hy+1), (HY+LSIZEY-hy-1), yoff, Q2);
      LIKWID_MARKER_STOP("c_kernel");

      // Bottom Border
      if ( mpi_rank_y == 0 ){ 
        propagate (f1_soa_h, f2_soa_h,        (HX), (HX+LSIZEX), (HY), (HY+hy+1), Q4);
        bcBottom  (f1_soa_h, f2_soa_h, param, (HX), (HX+LSIZEX), Q4 );

      LIKWID_MARKER_START("c_kernel");
        COLLIDE(f1_soa_h, f1_soa_h, (HX), (HX+LSIZEX), (HY), (HY+hy+1), yoff, Q4);
      LIKWID_MARKER_STOP("c_kernel");
      } else {
      LIKWID_MARKER_START("c_kernel");
        PROPAGATECOLLIDE (f1_soa_h, f2_soa_h, (HX), (HX+LSIZEX), (HY), (HY+hy+1), yoff, Q4); 
      LIKWID_MARKER_STOP("c_kernel");
      }

      // Top Border
      if ( mpi_rank_y == (n_mpi_y-1)) { 
        propagate (f1_soa_h, f2_soa_h,        (HX), (HX+LSIZEX), (HY+LSIZEY-hy-1), (HY+LSIZEY), Q5);
        bcTop     (f1_soa_h, f2_soa_h, param, (HX), (HX+LSIZEX), Q5);

      LIKWID_MARKER_START("c_kernel");
        COLLIDE(f1_soa_h, f1_soa_h, (HX), (HX+LSIZEX), (HY+LSIZEY-hy-1), (HY+LSIZEY), yoff, Q5);
      LIKWID_MARKER_STOP("c_kernel");
      } else {
      LIKWID_MARKER_START("c_kernel");
        PROPAGATECOLLIDE(f1_soa_h, f2_soa_h, (HX), (HX+LSIZEX), (HY+LSIZEY-hy-1), (HY+LSIZEY), yoff, Q5);
      LIKWID_MARKER_STOP("c_kernel");
      }

      ///////////////////////////////////////////////////////////////////////////

#if defined(SPEC_OPENACC)
      #pragma acc wait(Q1, Q2, Q3, Q4, Q5)
#endif 

      ///////////////////////////////////////////////////////////////////////////
      // swap pointers

      f_tmp_h  = f1_soa_h;
      f1_soa_h = f2_soa_h;
      f2_soa_h = f_tmp_h;

      ///////////////////////////////////////////////////////////////////////////

#ifndef SPEC
      dT[1] += (double)(tf[1].tv_sec  - ti[1].tv_sec )*1.e6 +
        (double)(tf[1].tv_usec - ti[1].tv_usec);

      dT[2] += (double)(tf[2].tv_sec  - ti[2].tv_sec )*1.e6 +
        (double)(tf[2].tv_usec - ti[2].tv_usec);
#endif

      ///////////////////////////////////////////////////////////////////////////
      // this barrier helps to keep process synchronized
      MPI_Barrier(MPI_COMM_WORLD);

    }
    //////////////////////////////////////////////////////////////////////////////
    // End of LBM iterations.
    LIKWID_MARKER_CLOSE;
    if (mpi_rank == 0) {
        gettimeofday(&ttotf[0], NULL);
        fprintf(stdout, "Total iteration time: %.3f s ( %d ranks)\n", (double) (ttotf[0].tv_sec - ttoti[0].tv_sec) + (double)(ttotf[0].tv_usec - ttoti[0].tv_usec)*1.e-6, n_mpi);
    }

#ifndef SPEC  
    gettimeofday(&tf[0], NULL);
#endif


#ifndef SPEC
  dT[0] = (double)(tf[0].tv_sec  - ti[0].tv_sec )*1.e6 +
    (double)(tf[0].tv_usec - ti[0].tv_usec);  
#endif  

  // mass check
#ifdef CHECK_MASS
  // Lattice is copied back to the host and converted to AoS
  SoAtoAoS(f2_soa_h, f_aos_h); // f2_soa_h --> f_aos_h
  mass = dumpMass(f_aos_h);
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Reduce(&mass, &g_mass, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
  if ( mpi_rank == 0 ) {
    if( fabs((i_mass - g_mass) / i_mass) < 1.e-10 ){
#ifdef SPEC
      fprintf(stdout, "Mass check: PASSED.\n");
#else
      fprintf(stdout, "Mass check: PASSED %.10e == %.10e (Er: %e).\n", i_mass, g_mass, fabs((i_mass - g_mass))/i_mass);
#endif
    } else{
      fprintf(stdout, "Mass check: FAILED %.10e <> %.10e (Er: %e).\n", i_mass, g_mass, fabs((i_mass - g_mass))/i_mass);
    }
    fflush(stdout);
  }
#endif

#ifndef SPEC 
  MPI_Barrier(MPI_COMM_WORLD);

  // collect execution times from different MPI processes 
  //MPI_Reduce(&dT[0], &mpimin_dt_tot, 1,     MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  //MPI_Reduce(&dT[1], &mpimin_dt_swap_nc, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  //MPI_Reduce(&dT[2], &mpimin_dt_swap_c, 1,  MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&dT[0], &mpimax_dt_tot, 1,     MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  MPI_Reduce(&dT[1], &mpimax_dt_swap_nc, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
  MPI_Reduce(&dT[2], &mpimax_dt_swap_c, 1,  MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

  /////////////////////////////////////////////////////////////////////////// 
  // print Statistics.

  //latticesize = ((double)GSIZEX*(double)GSIZEY*37.0*(double)sizeof(data_t)); 
  //bw = (2.0*latticesize*(double)niter) / (dT[1]*1.e3);
  //p  = ((double)NFLOP*(double)GSIZEX*(double)GSIZEY*(double)(niter)) / (dT[3]*1.e3); 
  mlups = (double)(LSIZEX*LSIZEY)/(dT[0]/(double)niter); 

  // Each node prints its own satistics
  for(ii = 0; ii < mpi_size; ii++){
    if(ii == mpi_rank){
      fprintf(stdout, "[MPI%04d]: Wct: %.2f s, (%.2f us/iter), Tswp_nc: %.2f s (%.2f us/iter), Tswp_c %.2f s (%.2f us/iter), MLUP/s: %4.2f \n", 
          (int)ii, dT[0]*1.e-6, dT[0]/(double)niter, dT[1]*1.e-6, dT[1]/(double)niter, dT[2]*1.e-6, dT[2]/(double)niter, mlups );
    }
    fflush(stdout);

    MPI_Barrier(MPI_COMM_WORLD);  
  }

  MPI_Barrier(MPI_COMM_WORLD);

  // MPI0 prints global info
  if(mpi_rank == 0){
    fprintf(stdout, "[GLOBAL-INFO] [%dx%d] NITER: %d NMPI: %d [%02dx%02d %s] Wct: %.02f s (%.02f ms/iter), Tswp_nc: %.02f s (%.02f us/iter), Tswp_c %.2f s (%.2f us/iter), P: %.2f GFLOPs, MLUP/s: %.2f (FLOPs/site: %d)\n",
        GSIZEX, GSIZEY,
        niter, mpi_size,
        n_mpi_x, n_mpi_y, mpi_order,
        mpimax_dt_tot*1.e-6,     (mpimax_dt_tot*1.0e-3) / (double)niter,
        mpimax_dt_swap_nc*1.e-6, (mpimax_dt_swap_nc) / (double)niter,
        mpimax_dt_swap_c*1.e-6, (mpimax_dt_swap_c) / (double)niter,
        ((double)NFLOP*(double)GSIZEX*(double)GSIZEY*(double)(niter))/(mpimax_dt_tot*1e3),
        (double)(GSIZEX*GSIZEY)/((mpimax_dt_tot)/(double)niter),
        NFLOP
           );
  }

  fflush(stdout);
#endif

  MPI_Barrier(MPI_COMM_WORLD);

#ifdef SPEC_OPENACC
#pragma acc exit data delete(f1_soa_h, leftSndBuf, leftRxBuf,  \
    rightSndBuf, rightRxBuf, topSndBuf, topRxBuf, botSndBuf,   \
    botRxBuf, OFF0, OFF, f2_soa_h)

#elif defined(USE_TARGET)

#pragma omp target exit data                                   \
    map(release:f1_soa_h[0:nelem], f2_soa_h[0:nelem],          \
    leftSndBuf[0:(15+8+3)*NY], leftRxBuf[0:(15+8+3)*NY],       \
    rightSndBuf[0:(15+8+3)*NY], rightRxBuf[0:(15+8+3)*NY],     \
    topSndBuf[0:(15+8+3)*LSIZEX], topRxBuf[0:(15+8+3)*LSIZEX], \
    botSndBuf[0:(15+8+3)*LSIZEX], botRxBuf[0:(15+8+3)*LSIZEX], \
    OFF0[0:37], OFF[0:37])
#endif
}
#endif // end SPEC_CUDA
