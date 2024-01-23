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

#ifndef _PROPAGATE_H
#define _PROPAGATE_H

///////////////////////////////////////////////////////////////////////////////

void propagate (
                      data_t * const __restrict__ nxt,
              const   data_t * const __restrict__ prv,
              const int startX, const int endX,
              const int startY, const int endY, queue_t queue 
  ) {

  int ix, iy;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(prv, nxt) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for collapse(2) private(ix, iy)
#elif defined(USE_TARGET)
  #pragma omp target teams private(ix, iy)
  #pragma omp OMP_SCHED collapse(2)
#endif
  for ( ix = startX; ix < endX; ix++) {
#ifdef SPEC_OPENACC
    #pragma acc loop vector independent
#endif
    for ( iy = startY; iy < endY; iy++) {
      nxt[ ix*NY + iy            ] = prv[ ix*NY + iy +          - 3*NY + 1];
      nxt[ ix*NY + iy +    NX*NY ] = prv[ ix*NY + iy +    NX*NY - 3*NY    ];
      nxt[ ix*NY + iy +  2*NX*NY ] = prv[ ix*NY + iy +  2*NX*NY - 3*NY - 1];
      nxt[ ix*NY + iy +  3*NX*NY ] = prv[ ix*NY + iy +  3*NX*NY - 2*NY + 2];
      nxt[ ix*NY + iy +  4*NX*NY ] = prv[ ix*NY + iy +  4*NX*NY - 2*NY + 1];
      nxt[ ix*NY + iy +  5*NX*NY ] = prv[ ix*NY + iy +  5*NX*NY - 2*NY    ];
      nxt[ ix*NY + iy +  6*NX*NY ] = prv[ ix*NY + iy +  6*NX*NY - 2*NY - 1];
      nxt[ ix*NY + iy +  7*NX*NY ] = prv[ ix*NY + iy +  7*NX*NY - 2*NY - 2];
      nxt[ ix*NY + iy +  8*NX*NY ] = prv[ ix*NY + iy +  8*NX*NY -   NY + 3];
      nxt[ ix*NY + iy +  9*NX*NY ] = prv[ ix*NY + iy +  9*NX*NY -   NY + 2];
      nxt[ ix*NY + iy + 10*NX*NY ] = prv[ ix*NY + iy + 10*NX*NY -   NY + 1];
      nxt[ ix*NY + iy + 11*NX*NY ] = prv[ ix*NY + iy + 11*NX*NY -   NY    ];
      nxt[ ix*NY + iy + 12*NX*NY ] = prv[ ix*NY + iy + 12*NX*NY -   NY - 1];
      nxt[ ix*NY + iy + 13*NX*NY ] = prv[ ix*NY + iy + 13*NX*NY -   NY - 2];
      nxt[ ix*NY + iy + 14*NX*NY ] = prv[ ix*NY + iy + 14*NX*NY -   NY - 3];
      nxt[ ix*NY + iy + 15*NX*NY ] = prv[ ix*NY + iy + 15*NX*NY        + 3];
      nxt[ ix*NY + iy + 16*NX*NY ] = prv[ ix*NY + iy + 16*NX*NY        + 2];
      nxt[ ix*NY + iy + 17*NX*NY ] = prv[ ix*NY + iy + 17*NX*NY        + 1];
      nxt[ ix*NY + iy + 18*NX*NY ] = prv[ ix*NY + iy + 18*NX*NY           ];
      nxt[ ix*NY + iy + 19*NX*NY ] = prv[ ix*NY + iy + 19*NX*NY        - 1];
      nxt[ ix*NY + iy + 20*NX*NY ] = prv[ ix*NY + iy + 20*NX*NY        - 2];
      nxt[ ix*NY + iy + 21*NX*NY ] = prv[ ix*NY + iy + 21*NX*NY        - 3];
      nxt[ ix*NY + iy + 22*NX*NY ] = prv[ ix*NY + iy + 22*NX*NY +   NY + 3];
      nxt[ ix*NY + iy + 23*NX*NY ] = prv[ ix*NY + iy + 23*NX*NY +   NY + 2];
      nxt[ ix*NY + iy + 24*NX*NY ] = prv[ ix*NY + iy + 24*NX*NY +   NY + 1];
      nxt[ ix*NY + iy + 25*NX*NY ] = prv[ ix*NY + iy + 25*NX*NY +   NY    ];
      nxt[ ix*NY + iy + 26*NX*NY ] = prv[ ix*NY + iy + 26*NX*NY +   NY - 1];
      nxt[ ix*NY + iy + 27*NX*NY ] = prv[ ix*NY + iy + 27*NX*NY +   NY - 2];
      nxt[ ix*NY + iy + 28*NX*NY ] = prv[ ix*NY + iy + 28*NX*NY +   NY - 3];
      nxt[ ix*NY + iy + 29*NX*NY ] = prv[ ix*NY + iy + 29*NX*NY + 2*NY + 2];
      nxt[ ix*NY + iy + 30*NX*NY ] = prv[ ix*NY + iy + 30*NX*NY + 2*NY + 1];
      nxt[ ix*NY + iy + 31*NX*NY ] = prv[ ix*NY + iy + 31*NX*NY + 2*NY    ];
      nxt[ ix*NY + iy + 32*NX*NY ] = prv[ ix*NY + iy + 32*NX*NY + 2*NY - 1];
      nxt[ ix*NY + iy + 33*NX*NY ] = prv[ ix*NY + iy + 33*NX*NY + 2*NY - 2];
      nxt[ ix*NY + iy + 34*NX*NY ] = prv[ ix*NY + iy + 34*NX*NY + 3*NY + 1];
      nxt[ ix*NY + iy + 35*NX*NY ] = prv[ ix*NY + iy + 35*NX*NY + 3*NY    ];
      nxt[ ix*NY + iy + 36*NX*NY ] = prv[ ix*NY + iy + 36*NX*NY + 3*NY - 1];
    }

  }

}

///////////////////////////////////////////////////////////////////////////////

#endif /* _PROPAGATE_H */

