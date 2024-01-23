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

#ifndef _PACKUNPACK_H
#define _PACKUNPACK_H

///////////////////////////
//
//        8 15 22
//     3  9 16 23 29
//  0  4 10 17 24 30 34
//  1  5 11 18 25 31 35
//  2  6 12 19 26 32 36
//     7 13 20 27 33
//       14 21 28
//
///////////////////////////
//
// list of population indexes drifting/going towards Sud
//
// cont int toSud3[]    = { 0, 3, 4, 8, 9,10,15,16,17,22,23,24,29,30,34}; // 15
// cont int toSud2[]    = { 3, 8, 9,15,16,22,23,29};                      // 8
// cont int toSud1[]    = { 8, 15,22};                                    // 3

// list of population indexes drifting/going towards North
// cont int toNorth3[]  = {  2, 6, 7,12,13,14,19,20,21,26,27,28,32,33,36};
// cont int toNorth2[]  = {  7,13,14,20,21,27,28,33};
// cont int toNorth1[]  = { 14,21,28};

// list of population indexes drifting/going towards East
// const int toEast3[]  = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14};
// const int toEast2[]  = { 0, 1, 2, 3, 4, 5, 6, 7};
// const int toEast1[]  = { 0, 1, 2};

// list of population indexes drifting/going towards west
// const int toWest3    = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36};
// const int toWest2    = {29,30,31,32,33,34,35,36};
// const int toWest1    = {34,35,36};

////////////////////////////////////////////////////////////////////////

static inline void pack_bot_border ( data_t * botBorderBuf, data_t * f2_soa_h, queue_t queue ) {

  int ix;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(botBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED
#endif
  for ( ix = HX; ix < (HX + LSIZEX); ix++ ) {
    *(botBorderBuf+(26*(ix-HX))+ 0) = *(f2_soa_h+( 0*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 1) = *(f2_soa_h+( 3*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 2) = *(f2_soa_h+( 4*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 3) = *(f2_soa_h+( 8*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 4) = *(f2_soa_h+( 9*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 5) = *(f2_soa_h+(10*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 6) = *(f2_soa_h+(15*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 7) = *(f2_soa_h+(16*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 8) = *(f2_soa_h+(17*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+ 9) = *(f2_soa_h+(22*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+10) = *(f2_soa_h+(23*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+11) = *(f2_soa_h+(24*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+12) = *(f2_soa_h+(29*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+13) = *(f2_soa_h+(30*NX*NY)+(ix*NY)+(HY+0));
    *(botBorderBuf+(26*(ix-HX))+14) = *(f2_soa_h+(34*NX*NY)+(ix*NY)+(HY+0));

    *(botBorderBuf+(26*(ix-HX))+15) = *(f2_soa_h+( 3*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+16) = *(f2_soa_h+( 8*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+17) = *(f2_soa_h+( 9*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+18) = *(f2_soa_h+(15*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+19) = *(f2_soa_h+(16*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+20) = *(f2_soa_h+(22*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+21) = *(f2_soa_h+(23*NX*NY)+(ix*NY)+(HY+1));
    *(botBorderBuf+(26*(ix-HX))+22) = *(f2_soa_h+(29*NX*NY)+(ix*NY)+(HY+1));

    *(botBorderBuf+(26*(ix-HX))+23) = *(f2_soa_h+( 8*NX*NY)+(ix*NY)+(HY+2));
    *(botBorderBuf+(26*(ix-HX))+24) = *(f2_soa_h+(15*NX*NY)+(ix*NY)+(HY+2));
    *(botBorderBuf+(26*(ix-HX))+25) = *(f2_soa_h+(22*NX*NY)+(ix*NY)+(HY+2));
  }

}

////////////////////////////////////////////////////////////////////////

static inline void unpack_top_halo ( data_t * f2_soa_h, data_t * botBorderBuf, queue_t queue ) {

  int ix;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(botBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED
#endif
  for ( ix = HX; ix < (HX + LSIZEX); ix++ ) {
    *(f2_soa_h+( 0*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 0);
    *(f2_soa_h+( 3*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 1);
    *(f2_soa_h+( 4*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 2);
    *(f2_soa_h+( 8*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 3);
    *(f2_soa_h+( 9*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 4);
    *(f2_soa_h+(10*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 5);
    *(f2_soa_h+(15*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 6);
    *(f2_soa_h+(16*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 7);
    *(f2_soa_h+(17*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 8);
    *(f2_soa_h+(22*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+ 9);
    *(f2_soa_h+(23*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+10);
    *(f2_soa_h+(24*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+11);
    *(f2_soa_h+(29*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+12);
    *(f2_soa_h+(30*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+13);
    *(f2_soa_h+(34*NX*NY)+(ix*NY)+(HY+LSIZEY))   = *(botBorderBuf+(26*(ix-HX))+14);

    *(f2_soa_h+( 3*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+15);
    *(f2_soa_h+( 8*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+16);
    *(f2_soa_h+( 9*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+17);
    *(f2_soa_h+(15*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+18);
    *(f2_soa_h+(16*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+19);
    *(f2_soa_h+(22*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+20);
    *(f2_soa_h+(23*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+21);
    *(f2_soa_h+(29*NX*NY)+(ix*NY)+(HY+LSIZEY+1)) = *(botBorderBuf+(26*(ix-HX))+22);

    *(f2_soa_h+( 8*NX*NY)+(ix*NY)+(HY+LSIZEY+2)) = *(botBorderBuf+(26*(ix-HX))+23);
    *(f2_soa_h+(15*NX*NY)+(ix*NY)+(HY+LSIZEY+2)) = *(botBorderBuf+(26*(ix-HX))+24);
    *(f2_soa_h+(22*NX*NY)+(ix*NY)+(HY+LSIZEY+2)) = *(botBorderBuf+(26*(ix-HX))+25);
  }

}

////////////////////////////////////////////////////////////////////////

static inline void pack_top_border ( data_t * topBorderBuf, data_t * f2_soa_h, queue_t queue ) {

  int ix;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(topBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED 
#endif
  for ( ix = HX; ix < (HX + LSIZEX); ix++ ) {
    *(topBorderBuf+(26*(ix-HX))+ 0) = *(f2_soa_h+( 2*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 1) = *(f2_soa_h+( 6*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 2) = *(f2_soa_h+( 7*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 3) = *(f2_soa_h+(12*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 4) = *(f2_soa_h+(13*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 5) = *(f2_soa_h+(14*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 6) = *(f2_soa_h+(19*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 7) = *(f2_soa_h+(20*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 8) = *(f2_soa_h+(21*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+ 9) = *(f2_soa_h+(26*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+10) = *(f2_soa_h+(27*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+11) = *(f2_soa_h+(28*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+12) = *(f2_soa_h+(32*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+13) = *(f2_soa_h+(33*NX*NY)+(ix*NY)+(HY+LSIZEY-1));
    *(topBorderBuf+(26*(ix-HX))+14) = *(f2_soa_h+(36*NX*NY)+(ix*NY)+(HY+LSIZEY-1));

    *(topBorderBuf+(26*(ix-HX))+15) = *(f2_soa_h+( 7*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+16) = *(f2_soa_h+(13*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+17) = *(f2_soa_h+(14*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+18) = *(f2_soa_h+(20*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+19) = *(f2_soa_h+(21*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+20) = *(f2_soa_h+(27*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+21) = *(f2_soa_h+(28*NX*NY)+(ix*NY)+(HY+LSIZEY-2));
    *(topBorderBuf+(26*(ix-HX))+22) = *(f2_soa_h+(33*NX*NY)+(ix*NY)+(HY+LSIZEY-2));

    *(topBorderBuf+(26*(ix-HX))+23) = *(f2_soa_h+(14*NX*NY)+(ix*NY)+(HY+LSIZEY-3));
    *(topBorderBuf+(26*(ix-HX))+24) = *(f2_soa_h+(21*NX*NY)+(ix*NY)+(HY+LSIZEY-3));
    *(topBorderBuf+(26*(ix-HX))+25) = *(f2_soa_h+(28*NX*NY)+(ix*NY)+(HY+LSIZEY-3));
  }

}

////////////////////////////////////////////////////////////////////////

static inline void unpack_bot_halo ( data_t * f2_soa_h, data_t * topBorderBuf, queue_t queue  ) {

  int ix;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(topBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED
#endif
  for ( ix = HX; ix < (HX + LSIZEX); ix++ ) {
    *(f2_soa_h+( 2*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 0);
    *(f2_soa_h+( 6*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 1);
    *(f2_soa_h+( 7*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 2);
    *(f2_soa_h+(12*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 3);
    *(f2_soa_h+(13*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 4);
    *(f2_soa_h+(14*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 5);
    *(f2_soa_h+(19*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 6);
    *(f2_soa_h+(20*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 7);
    *(f2_soa_h+(21*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 8);
    *(f2_soa_h+(26*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+ 9);
    *(f2_soa_h+(27*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+10);
    *(f2_soa_h+(28*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+11);
    *(f2_soa_h+(32*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+12);
    *(f2_soa_h+(33*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+13);
    *(f2_soa_h+(36*NX*NY)+(ix*NY)+(HY-1)) = *(topBorderBuf+(26*(ix-HX))+14);

    *(f2_soa_h+( 7*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+15);
    *(f2_soa_h+(13*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+16);
    *(f2_soa_h+(14*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+17);
    *(f2_soa_h+(20*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+18);
    *(f2_soa_h+(21*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+19);
    *(f2_soa_h+(27*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+20);
    *(f2_soa_h+(28*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+21);
    *(f2_soa_h+(33*NX*NY)+(ix*NY)+(HY-2)) = *(topBorderBuf+(26*(ix-HX))+22);

    *(f2_soa_h+(14*NX*NY)+(ix*NY)+(HY-3)) = *(topBorderBuf+(26*(ix-HX))+23);
    *(f2_soa_h+(21*NX*NY)+(ix*NY)+(HY-3)) = *(topBorderBuf+(26*(ix-HX))+24);
    *(f2_soa_h+(28*NX*NY)+(ix*NY)+(HY-3)) = *(topBorderBuf+(26*(ix-HX))+25);
  }

}

////////////////////////////////////////////////////////////////////////

static inline void pack_left_border ( data_t * leftBorderBuf, data_t * f2_soa_h, queue_t queue ) {

  int iy;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(leftBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED
#endif
  for ( iy = 0; iy < NY; iy++ ) {
    *(leftBorderBuf+(26*iy)+ 0) = *(f2_soa_h+(22*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 1) = *(f2_soa_h+(23*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 2) = *(f2_soa_h+(24*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 3) = *(f2_soa_h+(25*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 4) = *(f2_soa_h+(26*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 5) = *(f2_soa_h+(27*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 6) = *(f2_soa_h+(28*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 7) = *(f2_soa_h+(29*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 8) = *(f2_soa_h+(30*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+ 9) = *(f2_soa_h+(31*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+10) = *(f2_soa_h+(32*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+11) = *(f2_soa_h+(33*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+12) = *(f2_soa_h+(34*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+13) = *(f2_soa_h+(35*NX*NY)+((HX)*NY)+iy);
    *(leftBorderBuf+(26*iy)+14) = *(f2_soa_h+(36*NX*NY)+((HX)*NY)+iy);

    *(leftBorderBuf+(26*iy)+15) = *(f2_soa_h+(29*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+16) = *(f2_soa_h+(30*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+17) = *(f2_soa_h+(31*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+18) = *(f2_soa_h+(32*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+19) = *(f2_soa_h+(33*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+20) = *(f2_soa_h+(34*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+21) = *(f2_soa_h+(35*NX*NY)+((HX+1)*NY)+iy);
    *(leftBorderBuf+(26*iy)+22) = *(f2_soa_h+(36*NX*NY)+((HX+1)*NY)+iy);

    *(leftBorderBuf+(26*iy)+23) = *(f2_soa_h+(34*NX*NY)+((HX+2)*NY)+iy);
    *(leftBorderBuf+(26*iy)+24) = *(f2_soa_h+(35*NX*NY)+((HX+2)*NY)+iy);
    *(leftBorderBuf+(26*iy)+25) = *(f2_soa_h+(36*NX*NY)+((HX+2)*NY)+iy);
  }

}

////////////////////////////////////////////////////////////////////////

static inline void unpack_right_halo ( data_t * f2_soa_h, data_t * leftBorderBuf, queue_t queue ) {

  int iy;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(leftBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED
#endif
  for ( iy = 0; iy < NY; iy++ ) {
    *(f2_soa_h+(22*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 0);
    *(f2_soa_h+(23*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 1);
    *(f2_soa_h+(24*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 2);
    *(f2_soa_h+(25*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 3);
    *(f2_soa_h+(26*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 4);
    *(f2_soa_h+(27*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 5);
    *(f2_soa_h+(28*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 6);
    *(f2_soa_h+(29*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 7);
    *(f2_soa_h+(30*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 8);
    *(f2_soa_h+(31*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+ 9);
    *(f2_soa_h+(32*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+10);
    *(f2_soa_h+(33*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+11);
    *(f2_soa_h+(34*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+12);
    *(f2_soa_h+(35*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+13);
    *(f2_soa_h+(36*NX*NY)+((HX+LSIZEX)*NY)+iy)   = *(leftBorderBuf+(26*iy)+14);

    *(f2_soa_h+(29*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+15);
    *(f2_soa_h+(30*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+16);
    *(f2_soa_h+(31*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+17);
    *(f2_soa_h+(32*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+18);
    *(f2_soa_h+(33*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+19);
    *(f2_soa_h+(34*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+20);
    *(f2_soa_h+(35*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+21);
    *(f2_soa_h+(36*NX*NY)+((HX+LSIZEX+1)*NY)+iy) = *(leftBorderBuf+(26*iy)+22);

    *(f2_soa_h+(34*NX*NY)+((HX+LSIZEX+2)*NY)+iy) = *(leftBorderBuf+(26*iy)+23);
    *(f2_soa_h+(35*NX*NY)+((HX+LSIZEX+2)*NY)+iy) = *(leftBorderBuf+(26*iy)+24);
    *(f2_soa_h+(36*NX*NY)+((HX+LSIZEX+2)*NY)+iy) = *(leftBorderBuf+(26*iy)+25);
  }

}

////////////////////////////////////////////////////////////////////////

static inline void pack_right_border ( data_t * rightBorderBuf, data_t * f2_soa_h, queue_t queue ) {

  int iy;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(rightBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED 
#endif
  for ( iy = 0; iy < NY; iy++ ) {
    *(rightBorderBuf+(26*iy)+ 0) = *(f2_soa_h+( 0*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 1) = *(f2_soa_h+( 1*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 2) = *(f2_soa_h+( 2*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 3) = *(f2_soa_h+( 3*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 4) = *(f2_soa_h+( 4*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 5) = *(f2_soa_h+( 5*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 6) = *(f2_soa_h+( 6*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 7) = *(f2_soa_h+( 7*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 8) = *(f2_soa_h+( 8*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+ 9) = *(f2_soa_h+( 9*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+10) = *(f2_soa_h+(10*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+11) = *(f2_soa_h+(11*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+12) = *(f2_soa_h+(12*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+13) = *(f2_soa_h+(13*NX*NY)+((HX+LSIZEX-1)*NY)+iy);
    *(rightBorderBuf+(26*iy)+14) = *(f2_soa_h+(14*NX*NY)+((HX+LSIZEX-1)*NY)+iy);

    *(rightBorderBuf+(26*iy)+15) = *(f2_soa_h+( 0*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+16) = *(f2_soa_h+( 1*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+17) = *(f2_soa_h+( 2*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+18) = *(f2_soa_h+( 3*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+19) = *(f2_soa_h+( 4*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+20) = *(f2_soa_h+( 5*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+21) = *(f2_soa_h+( 6*NX*NY)+((HX+LSIZEX-2)*NY)+iy);
    *(rightBorderBuf+(26*iy)+22) = *(f2_soa_h+( 7*NX*NY)+((HX+LSIZEX-2)*NY)+iy);

    *(rightBorderBuf+(26*iy)+23) = *(f2_soa_h+( 0*NX*NY)+((HX+LSIZEX-3)*NY)+iy);
    *(rightBorderBuf+(26*iy)+24) = *(f2_soa_h+( 1*NX*NY)+((HX+LSIZEX-3)*NY)+iy);
    *(rightBorderBuf+(26*iy)+25) = *(f2_soa_h+( 2*NX*NY)+((HX+LSIZEX-3)*NY)+iy);
  }

}

////////////////////////////////////////////////////////////////////////

static inline void unpack_left_halo ( data_t * f2_soa_h, data_t * rightBorderBuf, queue_t queue ) {
  
  int iy;

#ifdef SPEC_OPENACC
  #pragma acc kernels present(rightBorderBuf, f2_soa_h) async(queue)
  #pragma acc loop gang independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED 
#endif
  for ( iy = 0; iy < NY; iy++ ) {
    *(f2_soa_h+( 0*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 0);
    *(f2_soa_h+( 1*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 1);
    *(f2_soa_h+( 2*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 2);
    *(f2_soa_h+( 3*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 3);
    *(f2_soa_h+( 4*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 4);
    *(f2_soa_h+( 5*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 5);
    *(f2_soa_h+( 6*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 6);
    *(f2_soa_h+( 7*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 7);
    *(f2_soa_h+( 8*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 8);
    *(f2_soa_h+( 9*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+ 9);
    *(f2_soa_h+(10*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+10);
    *(f2_soa_h+(11*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+11);
    *(f2_soa_h+(12*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+12);
    *(f2_soa_h+(13*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+13);
    *(f2_soa_h+(14*NX*NY)+((HX-1)*NY)+iy) = *(rightBorderBuf+(26*iy)+14);

    *(f2_soa_h+( 0*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+15);
    *(f2_soa_h+( 1*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+16);
    *(f2_soa_h+( 2*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+17);
    *(f2_soa_h+( 3*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+18);
    *(f2_soa_h+( 4*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+19);
    *(f2_soa_h+( 5*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+20);
    *(f2_soa_h+( 6*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+21);
    *(f2_soa_h+( 7*NX*NY)+((HX-2)*NY)+iy) = *(rightBorderBuf+(26*iy)+22);

    *(f2_soa_h+( 0*NX*NY)+((HX-3)*NY)+iy) = *(rightBorderBuf+(26*iy)+23);
    *(f2_soa_h+( 1*NX*NY)+((HX-3)*NY)+iy) = *(rightBorderBuf+(26*iy)+24);
    *(f2_soa_h+( 2*NX*NY)+((HX-3)*NY)+iy) = *(rightBorderBuf+(26*iy)+25);
  }

}

////////////////////////////////////////////////////////////////////////

#endif // _PACKUNPACK_

