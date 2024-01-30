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

#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdlib.h>

#include "physvar.h"

///////////////////////////////////////////////////////////////////////////////
#ifdef SPEC_OPENMP_LOOP
#define OMP_SCHED  loop
#define OMP_SCHED_OUTER loop bind(teams)
#define OMP_SCHED_INNER loop bind(parallel)
#define USE_TARGET 1
#elif defined(SPEC_OPENMP_TARGET)
#define OMP_SCHED distribute parallel for 
#define OMP_SCHED_OUTER distribute
#define OMP_SCHED_INNER parallel for 
#define USE_TARGET 1
#else
#undef USE_TARGET
#endif
///////////////////////////////////////////////////////////////////////////////

#ifndef USE_DEBUG
#define NO_USE_DEBUG 1
#endif

#ifdef SPEC_Y_MAJOR_ORDER
#define Y_MAJOR_ORDER 1
#else
#define X_MAJOR_ORDER 1
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef SPEC
#define CHECK_MASS 1          // enable mass check
#define SPEC_RAND 1	      // use specrand
#endif

///////////////////////////////////////////////////////////////////////////////

#define NPOP 37                 // NUmber of Population.

#define HX  3                   // Dimension of horizontal halos.
#define HY  16                  // Dimension of vertical halos.

#define hx  3                   // Dimension of horizontal borders.
#define hy  3                   // Dimension of vertical borders.
 
///////////////////////////////////////////////////////////////////////////////

//#define __FLOAT__     // For single-precision code.
#define __DOUBLE__   // For double-precision code.

#ifdef __FLOAT__
# define data_t float
# undef __DOUBLE__
#endif

#ifdef __DOUBLE__
# define data_t double
# undef __FLOAT__
#endif

#if defined(USE_TARGET)
  typedef int* queue_t;
#else 
  typedef int  queue_t;
#endif

///////////////////////////////////////////////////////////////////////////////

// Type used to represent a population.
typedef struct {
  data_t p[NPOP];
} pop_t;

// Type used to define physical attributes (velocity, density, temperature).
typedef struct {
  data_t u;
  data_t v;
  data_t rho;
  data_t temp;
} uvrt_t;

// Tyoe use to define weights attributes.
typedef struct {
  data_t ww[NPOP];
  data_t cx[NPOP];
  data_t cy[NPOP];
  data_t H0[NPOP];
  data_t H1x[NPOP];
  data_t H1y[NPOP]; 
  data_t H2xx[NPOP];
  data_t H2xy[NPOP];
  data_t H2yy[NPOP]; 
  data_t H3xxx[NPOP];
  data_t H3xxy[NPOP];
  data_t H3xyy[NPOP];
  data_t H3yyy[NPOP];
  data_t H4xxxx[NPOP];
  data_t H4xxxy[NPOP];
  data_t H4xxyy[NPOP];
  data_t H4xyyy[NPOP];
  data_t H4yyyy[NPOP];
} par_t;
///////////////////////////////////////////////////////////////////////////////
// Constants

#define VZERO    0.0
#define VHALF    0.5
#define VONES    1.0
#define VTWOS    2.0
#define VTHRE    3.0
#define VFOUR    4.0
#define VSIXS    6.0
#define VEIGH    8.0
#define VTWO4    24.0
#define _02      (1.0/2.0)
#define _04      (1.0/4.0)
#define _06      (1.0/6.0)
#define _08      (1.0/8.0)
#define _24      (1.0/24.0)
#define VCONST1  CONST1
#define VCONST2  CONST2
#define VCONST3  CONST3
#define VTAU1B   TAU1B
#define VTAU2B   TAU2B
#define VTAU3B   TAU3B
#define forcex   (FORCEPOIS * DELTAT)

///////////////////////////////////////////////////////////////////////////////

#endif /* _COMMON_H */
