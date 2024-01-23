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

// Header file inclusion.
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>
#include <mpi.h>

#include <sys/types.h>
#include <unistd.h>

#include "physvar.h"
#include "common.h"

#ifdef SPEC_OPENACC
#include <openacc.h>
#elif defined(SPEC_CUDA)
#include <cuda.h>
#include <cuda_runtime.h>
#elif defined(SPEC_OPENMP) || defined(USE_TARGET)
#include <omp.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// Global variable

int mpi_rank;          // Rank of MPI node.
int mpi_size;          // Size of MPI world communicator.

int mpi_grid_dims[2];  // Sizes of MPI ranks grid of world communicator.

int n_mpi;             // total number of MPI ranks == n_mpi_x * n_mpi_y == mpi_size

int n_mpi_x;           // number of MPI ranks along X direction
int n_mpi_y;           // number of MPI ranks alonf Y direction

int mpi_rank_x;        // MPI X coordinate.
int mpi_rank_y;        // MPI Y coordinate.

int mpi_peer_left, mpi_peer_right, mpi_peer_top, mpi_peer_bot; // Rank of neighboring MPI ranks

char * mpi_order;      // MPI ranks order string

int GSIZEX, GSIZEY;    // Global Size X and Y of lattice
int LSIZEX, LSIZEY;    // Local Size X and Y of lattice
uint64_t NX, NY;       // X-length and Y-lenght of local lattice in memory
  
long int SEED;         // seed for random generator

const long long OFF0[NPOP] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

long long OFF[NPOP]; 

const int bits  = 8 * sizeof(int) - 1;

///////////////////////////////////////////////////////////////////////////////

#include "init.h"

///////////////////////////////////////////////////////////////////////////////
// Lattice-Boltzmann Method.

#ifdef __cplusplus
extern "C" 
{
#endif
void lbm(int, pop_t *p, par_t *);
#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]){
  pop_t  *p = NULL;
  //int config_errors;

  int namelen;
  int niter;
  int v;
  const int root = 0;
  FILE *fp;
  char *buf;
  size_t len, nread;
  char processor_name[MPI_MAX_PROCESSOR_NAME];

  int request, provided;

  par_t * param = NULL;

#ifdef SPEC_OPENACC
  acc_device_t my_device_type;
#endif

#if defined(SPEC_CUDA) || defined(SPEC_OPENACC) || defined(USE_TARGET)
  int num_devices;
  int gpuId;
  MPI_Comm shmcomm;
  int local_rank;
#endif

#ifndef SPEC
  struct timeval ti[3], tf[3];
  double dT[3];
#endif

#ifndef SPEC
  gettimeofday(&ti[0], NULL);
#endif

  /////////////////////////////////////////////////////////////////////////////

  // if (MPI_Init(&argc, &argv) != MPI_SUCCESS) { 
  //   fprintf(stderr, "Error in MPI_Init.\n");
  //   exit(-1);
  // }

  provided = -1;
  int requested_threading_model = -1;
#ifdef SPEC_OPENMP
  requested_threading_model = MPI_THREAD_FUNNELED;
#else
  requested_threading_model = MPI_THREAD_SINGLE;
#endif
  MPI_Init_thread(&argc, &argv, requested_threading_model, &provided);
  if(provided < requested_threading_model){
    fprintf(stdout, "Threading support level is lower than the requested one.\n");
    exit(-1);
  }

  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Get_processor_name(processor_name, &namelen);

  /////////////////////////////////////////////////////////////////////////////
  // reading inputs value form control file

  buf = (char*) malloc(1024*sizeof(char));
  len = 1024;

  if (mpi_rank == 0) {
    fp = fopen("control", "r");
    if (fp) {
	    if ( (nread = getline(&buf, &len, fp)) > 0 ) {
        sscanf(buf, "%d", &v);
      	GSIZEX = v;
      } else {
        printf("Error reading GSIZEX from control file. Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD,-1); 
      }
      if ( (nread = getline(&buf, &len, fp)) > 0 ) {
        sscanf(buf, "%d", &v);
      	GSIZEY = v;
      } else {
        printf("Error reading GSIZEX from control file. Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD,-1); 
      }
      if ( (nread = getline(&buf, &len, fp)) > 0 ) {
        sscanf(buf, "%d", &niter);
      } else {
        printf("Error reading NITER from control file. Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD,-1); 
      }
      if ( (nread = getline(&buf, &len, fp)) > 0 ) {
        sscanf(buf, "%ld", &SEED);
      } else {
        printf("Error reading SEED from control file. Aborting.\n");
        MPI_Abort(MPI_COMM_WORLD,-1); 
      }
    } else {
      printf("Error opening control file. Aborting.\n");
      MPI_Abort(MPI_COMM_WORLD,-1); 
     }
     fclose(fp);
     //if (niter <= 0 || niter >= 10000) {
     // printf("Error niter value out of range: %d\n",niter);
     // MPI_Abort(MPI_COMM_WORLD,-1); 
     //}
  }

  MPI_Bcast(&GSIZEX, 1, MPI_LONG_LONG, root, MPI_COMM_WORLD);
  MPI_Bcast(&GSIZEY, 1, MPI_LONG_LONG, root, MPI_COMM_WORLD);

  MPI_Bcast(&niter,  1, MPI_INT,       root, MPI_COMM_WORLD);
  MPI_Bcast(&SEED,   1, MPI_LONG,      root, MPI_COMM_WORLD);

  /////////////////////////////////////////////////////////////////////////////
  // set grid size of MPI ranks

  n_mpi = mpi_size;

  // n_mpi_x and n_mpi_y can be optionally set by NMPIX and NMPIY compilation flags
#ifdef NMPIX
  n_mpi_x = NMPIX;
#else
  n_mpi_x = 0;
#endif

#ifdef NMPIX
  n_mpi_y = NMPIY;
#else
  n_mpi_y = 0;
#endif

  // make sure mpi_grid_dims is zero!
  mpi_grid_dims[0] = 0;
  mpi_grid_dims[1] = 0;
  if ((n_mpi_x == 0) || (n_mpi_y == 0)) {
    // create a cartesian grid
    MPI_Dims_create( n_mpi, 2, mpi_grid_dims );

    n_mpi_x = mpi_grid_dims[0];
    n_mpi_y = mpi_grid_dims[1];
  }

  // check if size is OK
  if ( mpi_rank == 0 ) {
    if ( (n_mpi_x * n_mpi_y) != n_mpi ) {
      printf("Error: invalid value of grid size: n_mpi_x: %d X n_mpi_y: %d != n_mpi: %d\n", n_mpi_x, n_mpi_y, n_mpi);
      MPI_Abort(MPI_COMM_WORLD,-1); 
    }
  }

#ifdef X_MAJOR_ORDER
  mpi_rank_x = mpi_rank % n_mpi_x;
  mpi_rank_y = mpi_rank / n_mpi_x;
#endif
#ifdef Y_MAJOR_ORDER
  mpi_rank_x = mpi_rank / n_mpi_y;
  mpi_rank_y = mpi_rank % n_mpi_y;
#endif

  // Define peer ranks
#ifdef X_MAJOR_ORDER
  mpi_peer_left  =  ((mpi_rank    % n_mpi_x) == 0)       ? (mpi_rank+n_mpi_x-1)             : (mpi_rank-1);
  mpi_peer_right = (((mpi_rank+1) % n_mpi_x) == 0)       ? (mpi_rank-n_mpi_x+1)             : (mpi_rank+1);
  mpi_peer_top   =   (mpi_rank >= (n_mpi_x*(n_mpi_y-1))) ? (mpi_rank-(n_mpi_x*(n_mpi_y-1))) : (mpi_rank+n_mpi_x);
  mpi_peer_bot   =   (mpi_rank <   n_mpi_x)              ? (mpi_rank+(n_mpi_x*(n_mpi_y-1))) : (mpi_rank-n_mpi_x);
  mpi_order      = "X_MAJOR_ORDER";
#elif defined(Y_MAJOR_ORDER)
  mpi_peer_left  =   (mpi_rank <   n_mpi_y)              ? (mpi_rank+(n_mpi_y*(n_mpi_x-1))) : (mpi_rank-n_mpi_y);
  mpi_peer_right =   (mpi_rank >= (n_mpi_y*(n_mpi_x-1))) ? (mpi_rank-(n_mpi_y*(n_mpi_x-1))) : (mpi_rank+n_mpi_y);
  mpi_peer_top   = (((mpi_rank+1) % n_mpi_y) == 0)       ? (mpi_rank-n_mpi_y+1)             : (mpi_rank+1);
  mpi_peer_bot   =  ((mpi_rank    % n_mpi_y) == 0)       ? (mpi_rank+n_mpi_y-1)             : (mpi_rank-1);
  mpi_order      = "Y_MAJOR_ORDER"; 
#endif

  /////////////////////////////////////////////////////////////////////////////

  LSIZEX = (GSIZEX / n_mpi_x);   // Local lattice sizex.
  LSIZEY = (GSIZEY / n_mpi_y);   // Local lattice sizey.

  // distribute remainder equally to all ranks, starting from rank 0
  if (mpi_rank_x < (GSIZEX % n_mpi_x)) {
    LSIZEX += 1;
  }
  // distribute remainder equally to all ranks, starting from rank 0
  if (mpi_rank_y < (GSIZEY % n_mpi_y)) {
    LSIZEY += 1;
  }
  // add reminder to righ-most ranks of grid if GSIZEX is not multiple of n_mpi_x
  //if ( mpi_rank_x == (n_mpi_x-1) ) {
  //  LSIZEX += (GSIZEX % n_mpi_x);
  //}

  // add reminder to top-most ranks of grid if GSIZEY is not multiple of n_mpi_y
  //if ( mpi_rank_y == (n_mpi_y-1) ) {
  //  LSIZEY += (GSIZEY % n_mpi_y);
  //}

  NX     = (HX + LSIZEX + HX); // X-length of lattice in memory.
  NY     = (HY + LSIZEY + HY); // X-lenght of lattice in memory.

  // initialize offsets for propagation step
  OFF[ 0] = - 3*NY + 1;
  OFF[ 1] = - 3*NY    ;
  OFF[ 2] = - 3*NY - 1;
  OFF[ 3] = - 2*NY + 2;
  OFF[ 4] = - 2*NY + 1;
  OFF[ 5] = - 2*NY    ;
  OFF[ 6] = - 2*NY - 1;
  OFF[ 7] = - 2*NY - 2;
  OFF[ 8] = -	  NY + 3;
  OFF[ 9] = -	  NY + 2;
  OFF[10] = -	  NY + 1;
  OFF[11] = -	  NY    ;
  OFF[12] = -	  NY - 1;
  OFF[13] = -	  NY - 2;
  OFF[14] = -	  NY - 3;
  OFF[15] = 	     + 3;
  OFF[16] = 	     + 2;
  OFF[17] = 	     + 1;
  OFF[18] =          0;
  OFF[19] = 	     - 1;
  OFF[20] = 	     - 2;
  OFF[21] = 	     - 3;
  OFF[22] = +	  NY + 3;
  OFF[23] = +	  NY + 2;
  OFF[24] = +	  NY + 1;
  OFF[25] = +	  NY    ;
  OFF[26] = +	  NY - 1;
  OFF[27] = +	  NY - 2;
  OFF[28] = +	  NY - 3;
  OFF[29] = + 2*NY + 2;
  OFF[30] = + 2*NY + 1;
  OFF[31] = + 2*NY    ;
  OFF[32] = + 2*NY - 1;
  OFF[33] = + 2*NY - 2;
  OFF[34] = + 3*NY + 1;
  OFF[35] = + 3*NY    ;
  OFF[36] = + 3*NY - 1;

//////////////////////////////////////////////////////////////////////////////
// Set the Accelerator Number
// Determine the local rank ID
// The device type, number of devices of that type on the node.
// Set the device number to the mod of the local rank and number of devices.
#if defined(SPEC_CUDA) || defined(SPEC_OPENACC) || defined(USE_TARGET)
    MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, 0,
                    MPI_INFO_NULL, &shmcomm);
    MPI_Comm_rank(shmcomm, &local_rank);
#endif

#ifdef SPEC_CUDA
  cudaGetDeviceCount ( &num_devices );
  gpuId = local_rank % num_devices;
  cudaSetDevice( gpuId );
#endif
#ifdef SPEC_OPENACC
  my_device_type = acc_get_device_type();
  num_devices = acc_get_num_devices(my_device_type);
  gpuId = local_rank % num_devices;
  acc_set_device_num(gpuId, my_device_type);
#endif
#ifdef USE_TARGET
  num_devices = omp_get_num_devices();
  if (num_devices > 0) {
    gpuId = local_rank % num_devices;
    omp_set_default_device(gpuId);
  }
#endif

#if (defined(SPEC_CUDA) || defined(SPEC_OPENACC) || defined(USE_TARGET))
  fprintf(stderr, "[MPI%04d]: will execute on device %d (total number of devices: %d) \n", mpi_rank, gpuId, num_devices);
#endif  

  /////////////////////////////////////////////////////////////////////////////

#ifndef SPEC
  #if (defined(SPEC_CUDA) || defined(SPEC_OPENACC))
  printf("[MPI%04d]: %02d tasks, host %s, GpuId: %d, mpi_rank_x: %02d, mpi_rank_y: %02d, mpi_peer_left: %02d, mpi_peer_right: %02d, mpi_peer_top: %02d, mpi_peer_bot: %02d, pid: %d\n", 
    mpi_rank, mpi_size, processor_name, gpuId,
    mpi_rank_x, mpi_rank_y, 
    mpi_peer_left, mpi_peer_right, mpi_peer_top, mpi_peer_bot,
    getpid());
  #else
  printf("[MPI%04d]: %02d tasks, host %s, mpi_rank_x: %02d, mpi_rank_y: %02d, mpi_peer_left: %02d, mpi_peer_right: %02d, mpi_peer_top: %02d, mpi_peer_bot: %02d, LSIZEX: %d LSIZEY: %d (%.02f GB), pid: %d\n", 
    mpi_rank, mpi_size, processor_name,
    mpi_rank_x, mpi_rank_y, 
    mpi_peer_left, mpi_peer_right, mpi_peer_top, mpi_peer_bot,
    LSIZEX, LSIZEY, (double)(LSIZEX*LSIZEY*sizeof(pop_t)) / (1<<30),
    getpid());
  #endif
#endif

 if ( mpi_rank == 0 ) {
#ifdef SPEC
    printf("GSIZEX: %d, GSIZEY: %d \n", GSIZEX, GSIZEY);
#else
    printf("[MPI%04d]: GSIZEX: %d, GSIZEY: %d (%.02f GB) NMPIX: %d NMPIY: %d \n", 
      mpi_rank, 
      GSIZEX, GSIZEY, (double)(GSIZEX*GSIZEY*sizeof(pop_t)) / (1<<30),
      n_mpi_x, n_mpi_y );
#endif
  }

  //////////////////////////////////////////////////////////////////////////////

  MPI_Barrier(MPI_COMM_WORLD);
  
  //////////////////////////////////////////////////////////////////////////////
  // Population lattice is allocated on the host.
  
  posix_memalign((void **) &p, 4096, NX*NY*sizeof(pop_t));

  if(p == NULL){
    fprintf(stderr, "[main] ERROR: p _mm_malloc failed\n");
    MPI_Abort(MPI_COMM_WORLD,-1); 
  }
  
  posix_memalign((void **) &param, 4096, sizeof(par_t));

  if(param == NULL){
    fprintf(stderr, "[main] ERROR: param _mm_malloc failed\n");
    MPI_Abort(MPI_COMM_WORLD,-1); 
  }
  
  memset(p, 0, NX*NY*sizeof(pop_t));
 
 
  /////////////////////////////////////////////////////////////////////////////
  // Lattice inizialization.
#ifndef SPEC
  gettimeofday(&ti[1], NULL);
#endif
  init(param, p, SEED);
#ifndef SPEC
  gettimeofday(&tf[1], NULL);
#endif

#ifdef SPEC_OPENACC
  #pragma acc enter data copyin(p[0:NX*NY], param[0:1])
#elif defined(USE_TARGET)
  #pragma omp target enter data map(to:p[0:NX*NY], param[0:1])
#endif

  /////////////////////////////////////////////////////////////////////////////
  // Lattice-Boltzmann Method function is called.
#ifndef SPEC
  gettimeofday(&ti[2], NULL);
#endif
  lbm(niter, p, param);
#ifndef SPEC
  gettimeofday(&tf[2], NULL);
#endif
  
  /////////////////////////////////////////////////////////////////////////////

#ifndef SPEC
  gettimeofday(&tf[0], NULL);
#endif

  /////////////////////////////////////////////////////////////////////////////

#ifndef SPEC
  dT[0] = (double)(tf[0].tv_sec  - ti[0].tv_sec ) +
          (double)(tf[0].tv_usec - ti[0].tv_usec) * 1.e-6;

  dT[1] = (double)(tf[1].tv_sec  - ti[1].tv_sec ) +
          (double)(tf[1].tv_usec - ti[1].tv_usec) * 1.e-6;

  dT[2] = (double)(tf[2].tv_sec  - ti[2].tv_sec ) +
          (double)(tf[2].tv_usec - ti[2].tv_usec) * 1.e-6;

  fprintf(stderr, "[MPI#%04d]: Tall: %.02f sec  Tini: %.02f sec  Tlbm: %.2f sec\n", 
                  mpi_rank, dT[0], dT[1],dT[2]);
#endif

#ifdef SPEC_OPENACC
  #pragma acc exit data delete(p,param)
#elif defined(USE_TARGET)
  #pragma omp target exit data map(release:p[0:NX*NY],param[0:1])
#endif
  /////////////////////////////////////////////////////////////////////////////
  // Free arrays.
  free(p);
  
  /////////////////////////////////////////////////////////////////////////////
  
#ifdef SPEC_OPENACC
  acc_shutdown(my_device_type);
#endif

  /////////////////////////////////////////////////////////////////////////////
  // MPI finalize.
  MPI_Finalize();

#ifdef SPEC_CUDA
  cudaDeviceReset();
#endif

  return 0;
}
