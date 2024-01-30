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

#ifndef _UTILS_M_H
#define _UTILS_M_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

///////////////////////////////////////////////////////////////////////////////

void AoStoSoA(pop_t *AoS, data_t * array){
  int ii, jj;
  uint64_t size;

  size = NX * NY;
#if defined(SPEC_OPENMP) 
  #pragma omp parallel for private(jj)
#elif defined (SPEC_OPENACC)
  #pragma acc parallel loop collapse(2) present(array,AoS)
#elif defined (USE_TARGET)
  #pragma omp target teams OMP_SCHED collapse(2)
#endif
  for(ii = 0; ii < size; ii++){
    for(jj = 0; jj < NPOP; jj++){
      array[jj * size + ii] = AoS[ii].p[jj];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

void SoAtoAoS(data_t * array, pop_t * AoS){
  int ii, jj;
  uint64_t size;
  
  size = NX * NY;
#if defined(SPEC_OPENMP) 
  #pragma omp parallel for private(jj)
#elif defined (SPEC_OPENACC)
  #pragma acc parallel loop collapse(2) present(AoS,array)
#elif defined (USE_TARGET)
  #pragma omp target teams OMP_SCHED collapse(2)
#endif
  for(ii = 0; ii < size; ii++){
    for(jj = 0; jj < NPOP; jj++){
      AoS[ii].p[jj] = array[jj * size + ii];
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

data_t dumpMass(pop_t *f){
  int ii, jj, kk;
  double mass;
  
  mass = 0.0;
#if defined(SPEC_OPENMP) 
  #pragma omp parallel for private(jj,kk) reduction(+:mass)
#elif defined (SPEC_OPENACC)
  #pragma acc parallel loop collapse(2) present(f) reduction(+:mass)
#elif defined (USE_TARGET)
  #pragma omp target teams OMP_SCHED collapse(2) reduction(+:mass) 
#endif
  for(ii = HX; ii < HX+LSIZEX; ii++){
    for(jj = HY; jj < HY+LSIZEY; jj++){
      for(kk = 0; kk < NPOP; kk++){
        mass += f[ii*NY + jj].p[kk];
      }
    }
  }
  return (data_t)mass;
}

///////////////////////////////////////////////////////////////////////////////

#endif // _UTILS_M_H
