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

#ifndef _BC_H
#define _BC_H

////////////////////////////////////////////////////////////////////////////////

#define _polint(xa,ya)                                \
                                                      \
  x=xa[0];                                            \
                                                      \
  dif=fabs(x-xa[1]);                                  \
                                                      \
  for (i=1;i<=ENNE;i++) {                             \
    if ( (dift=fabs(x-xa[i])) < dif) {                \
      ns=i;                                           \
      dif=dift;                                       \
    }                                                 \
    c[i]=ya[i];                                       \
    d[i]=ya[i];                                       \
  }                                                   \
                                                      \
  y=ya[ns--];                                         \
  for (m=1;m<ENNE;m++) {                              \
    for (i=1;i<=ENNE-m;i++) {                         \
      ho=xa[i]-x;                                     \
      hp=xa[i+m]-x;                                   \
      w=c[i+1]-d[i];                                  \
      den=ho-hp;                                      \
      den=w/den;                                      \
      d[i]=hp*den;                                    \
      c[i]=ho*den;                                    \
    }                                                 \
    y += (dy=(2*ns < (ENNE-m) ? c[ns+1] : d[ns--]));  \
  }                                                   \
                                                      \
  ya[0]=y;

////////////////////////////////////////////////////////////////////////////////

void bcBottom ( data_t * __restrict__ const nxt, const data_t* __restrict__ const prv, 
                const par_t * __restrict__ const param, const int startX, const int endX, queue_t queue){

  #define PHI(_ii)      nxt[ ((_ii)*NX*NY) + idx       ]
  #define LOCALPOP(_ii) nxt[ ((_ii)*NX*NY) + idx + off ]
  #define FPRE(_ii)     prv[ ((_ii)*NX*NY) + idx       ]
  #define FPOST(_ii)    nxt[ ((_ii)*NX*NY) + idx       ]

  unsigned long ix;
  
  //////////////////////////////////////////////////////////////////////////////
  ////////////////////// LOWER BORDER //////////////////////////////////////////

#ifdef SPEC_OPENACC
  #pragma acc kernels present(prv, nxt, param) async(queue)
  #pragma acc loop gang independent 
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams private(ix)
  #pragma omp OMP_SCHED
#endif
  for ( ix = startX; ix < endX; ix++ ) {    

    unsigned long iy; 
    unsigned long idx;

    //int curY;

    //////////////////////////////////////////////////////////////////////////////

    double a1,a2,a3;
    double b1,b2;
    double c1,c2,c3;
    double d1,d2,d3;

    double utilde1,vtilde1;
    double utilde2,vtilde2;
    double utilde3;

    double term1,term2;

    double temptilde1,temptilde2,temptilde3;
    double localTempfic;

    double px,py,E,S,N,Ox,Oy,OE;
    double ptildex,ptildey,Etilde,massapost;

    #ifndef ADIABATIC
      data_t xa[ENNE+2], ya[ENNE+2];
    #endif

    double rhoi, forcey, scalar;
    
    int p, off;
    
    double  rho, temp, u, v;

    ////////////////////LOWER LAYER 1 //////////////////////////////////////

    iy     = HY;
    idx = ix * NY + iy;

    //curY = 0;
    off  = 3;

    //////////////////////////////////////////////////////////////////////////
    // Compute local u, v, rho e temp.

    rho   = 0.0;
    u     = 0.0;
    v     = 0.0;
    temp  = 0.0;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      rho = rho + LOCALPOP(p);
      u   = u + param->cx[p] * LOCALPOP(p);
      v   = v + param->cy[p] * LOCALPOP(p);
    }

    //forcex = FORCEPOIS * DELTAT;
    forcey =   GRAVITY * DELTAT * rho;

    rhoi  = 1.0 / rho;
    u     = u * rhoi;
    v     = v * rhoi;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      scalar =  (param->cx[p] - u) * (param->cx[p] - u) + (param->cy[p] - v) * (param->cy[p] - v);
      temp   = temp + 0.5 * scalar * LOCALPOP(p);
    }

    temp = temp * rhoi;

    localTempfic = temp + (forcex * forcex + forcey * forcey) * rhoi * rhoi / 8.0;

    //////////////////////////////////////////////////////////////////////////

#ifdef ADIABATIC

    temptilde1 = localTempfic ;
    temptilde2 = localTempfic ;
    temptilde3 = localTempfic ;

#else

    temptilde1 = TEMPWALLDOWN;
    temptilde2 = TEMPWALLDOWN;
    temptilde3 = TEMPWALLDOWN;

    xa[1] = 3.0;
    xa[2] = 4.0;

    ya[1] = temptilde3;
    // In the original code here we found:
    //   ya[2]=tempfic[idx3];
    // the avaluation of tempfic is made above, so
    // we replace it with the following:
    ya[2] = localTempfic;

    // fprintf(stdout,"x: %03d %f\n", i, ya[2]);

    xa[0] = 2.0;
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0] = 1.0;
    _polint(xa,ya);
    temptilde1 = ya[0];

#endif

    // X velocity.
    utilde1 = 0.0;
    utilde2 = 0.0;
    utilde3 = 0.0;
    // Y velocity.
    vtilde1 = 0.0;
    vtilde2 = 0.0;
     
    N =
        FPRE(15) + FPRE(22) + FPRE( 8) +  FPRE(29) +
        FPRE(23) + FPRE(16) + FPRE( 9) +  FPRE( 3) +
        FPRE(34) + FPRE(30) + FPRE(24) +  FPRE(17) +
        FPRE(10) + FPRE( 4) + FPRE( 0);

    massapost = N +
        FPOST(35) + FPOST(31) + FPOST(25) + FPOST(11) +
        FPOST( 5) + FPOST( 1) + FPOST(34) + FPOST(30) +
        FPOST(24) + FPOST(17) + FPOST(10) + FPOST( 4) +
        FPOST( 0) + FPOST(29) + FPOST(23) + FPOST(16) +
        FPOST( 9) + FPOST( 3) + FPOST(22) + FPOST(15) +
        FPOST( 8) + FPOST(18);

    ptildex = massapost*(utilde1) - 0.5*FORCEPOIS*DELTAT;
    ptildey = massapost*(vtilde1) - 0.5*GRAVITY*massapost*DELTAT;

    Etilde  =  (temptilde1)*massapost +
                0.5*(ptildex*ptildex+ptildey*ptildey)/massapost;


    Ox=
          param->cx[35] * FPOST(35) + param->cx[31] * FPOST(31) +
          param->cx[25] * FPOST(25) + param->cx[11] * FPOST(11) +
          param->cx[ 5] * FPOST( 5) + param->cx[ 1] * FPOST( 1) +
          param->cx[34] * FPOST(34) + param->cx[30] * FPOST(30) +
          param->cx[24] * FPOST(24) + param->cx[17] * FPOST(17) +
          param->cx[10] * FPOST(10) + param->cx[ 4] * FPOST( 4) +
          param->cx[ 0] * FPOST( 0) + param->cx[29] * FPOST(29) +
          param->cx[23] * FPOST(23) + param->cx[16] * FPOST(16) +
          param->cx[ 9] * FPOST( 9) + param->cx[ 3] * FPOST( 3) +
          param->cx[22] * FPOST(22) + param->cx[15] * FPOST(15) +
          param->cx[ 8] * FPOST( 8);

    Oy=
          param->cy[35] * FPOST(35) + param->cy[31] * FPOST(31) +
          param->cy[25] * FPOST(25) + param->cy[11] * FPOST(11) +
          param->cy[ 5] * FPOST( 5) + param->cy[ 1] * FPOST( 1) +
          param->cy[34] * FPOST(34) + param->cy[30] * FPOST(30) +
          param->cy[24] * FPOST(24) + param->cy[17] * FPOST(17) +
          param->cy[10] * FPOST(10) + param->cy[ 4] * FPOST( 4) +
          param->cy[ 0] * FPOST( 0) + param->cy[29] * FPOST(29) +
          param->cy[23] * FPOST(23) + param->cy[16] * FPOST(16) +
          param->cy[ 9] * FPOST( 9) + param->cy[ 3] * FPOST( 3) +
          param->cy[22] * FPOST(22) + param->cy[15] * FPOST(15) +
          param->cy[ 8] * FPOST( 8);

    OE=
          0.5 * (param->cx[35]*param->cx[35]+param->cy[35]*param->cy[35]) * FPOST(35) +
          0.5 * (param->cx[31]*param->cx[31]+param->cy[31]*param->cy[31]) * FPOST(31) +
          0.5 * (param->cx[25]*param->cx[25]+param->cy[25]*param->cy[25]) * FPOST(25) +
          0.5 * (param->cx[11]*param->cx[11]+param->cy[11]*param->cy[11]) * FPOST(11) +
          0.5 * (param->cx[ 5]*param->cx[ 5]+param->cy[ 5]*param->cy[ 5]) * FPOST( 5) +
          0.5 * (param->cx[ 1]*param->cx[ 1]+param->cy[ 1]*param->cy[ 1]) * FPOST( 1) +
          0.5 * (param->cx[34]*param->cx[34]+param->cy[34]*param->cy[34]) * FPOST(34) +
          0.5 * (param->cx[30]*param->cx[30]+param->cy[30]*param->cy[30]) * FPOST(30) +
          0.5 * (param->cx[24]*param->cx[24]+param->cy[24]*param->cy[24]) * FPOST(24) +
          0.5 * (param->cx[17]*param->cx[17]+param->cy[17]*param->cy[17]) * FPOST(17) +
          0.5 * (param->cx[10]*param->cx[10]+param->cy[10]*param->cy[10]) * FPOST(10) +
          0.5 * (param->cx[ 4]*param->cx[ 4]+param->cy[ 4]*param->cy[ 4]) * FPOST( 4) +
          0.5 * (param->cx[ 0]*param->cx[ 0]+param->cy[ 0]*param->cy[ 0]) * FPOST( 0) +
          0.5 * (param->cx[29]*param->cx[29]+param->cy[29]*param->cy[29]) * FPOST(29) +
          0.5 * (param->cx[23]*param->cx[23]+param->cy[23]*param->cy[23]) * FPOST(23) +
          0.5 * (param->cx[16]*param->cx[16]+param->cy[16]*param->cy[16]) * FPOST(16) +
          0.5 * (param->cx[ 9]*param->cx[ 9]+param->cy[ 9]*param->cy[ 9]) * FPOST( 9) +
          0.5 * (param->cx[ 3]*param->cx[ 3]+param->cy[ 3]*param->cy[ 3]) * FPOST( 3) +
          0.5 * (param->cx[22]*param->cx[22]+param->cy[22]*param->cy[22]) * FPOST(22) +
          0.5 * (param->cx[15]*param->cx[15]+param->cy[15]*param->cy[15]) * FPOST(15) +
          0.5 * (param->cx[ 8]*param->cx[ 8]+param->cy[ 8]*param->cy[ 8]) * FPOST( 8);

    a1 = 26.*(ptildex-Ox)*UNIT;
    b1 =-40.*N*UNIT*UNIT;
    c1 = 47.*(ptildex-Ox)*UNIT*UNIT;
    d1 = 15.*(ptildex-Ox);

    a2 = 26.*(ptildey-Oy)*UNIT-54.*N*UNIT*UNIT;
    c2 = 47.*(ptildey-Oy)*UNIT*UNIT-91.*N*UNIT*UNIT*UNIT;
    d2 = 15.*(ptildey-Oy)-26.*N*UNIT;

    a3 = 26.*(Etilde-OE)*UNIT-91.*N*UNIT*UNIT*UNIT;
    c3 = 47.*(Etilde-OE)*UNIT*UNIT-N*UNIT*UNIT*UNIT*UNIT*367./2.;
    d3 = 15.*(Etilde-OE)-47.*N*UNIT*UNIT;

    py = (-c3*d2+c2*d3)/(-a3*c2+a2*c3);
    px = (a2*c3*d1-a2*c1*d3-a3*c2*d1-c3*a1*d2+c1*a3*d2+a1*c2*d3)/(b1*(a3*c2-a2*c3));

    E = (-a3*d2+a2*d3)/(a3*c2-a2*c3);


    PHI(19) = 1.+param->cx[19]*px+param->cy[19]*py+0.5*(param->cx[19]*param->cx[19]+param->cy[19]*param->cy[19])*E;
    PHI(20) = 1.+param->cx[20]*px+param->cy[20]*py+0.5*(param->cx[20]*param->cx[20]+param->cy[20]*param->cy[20])*E;
    PHI(21) = 1.+param->cx[21]*px+param->cy[21]*py+0.5*(param->cx[21]*param->cx[21]+param->cy[21]*param->cy[21])*E;
    PHI(12) = 1.+param->cx[12]*px+param->cy[12]*py+0.5*(param->cx[12]*param->cx[12]+param->cy[12]*param->cy[12])*E;
    PHI(13) = 1.+param->cx[13]*px+param->cy[13]*py+0.5*(param->cx[13]*param->cx[13]+param->cy[13]*param->cy[13])*E;
    PHI(14) = 1.+param->cx[14]*px+param->cy[14]*py+0.5*(param->cx[14]*param->cx[14]+param->cy[14]*param->cy[14])*E;
    PHI(26) = 1.+param->cx[26]*px+param->cy[26]*py+0.5*(param->cx[26]*param->cx[26]+param->cy[26]*param->cy[26])*E;
    PHI(27) = 1.+param->cx[27]*px+param->cy[27]*py+0.5*(param->cx[27]*param->cx[27]+param->cy[27]*param->cy[27])*E;
    PHI(28) = 1.+param->cx[28]*px+param->cy[28]*py+0.5*(param->cx[28]*param->cx[28]+param->cy[28]*param->cy[28])*E;
    PHI( 7) = 1.+param->cx[ 7]*px+param->cy[ 7]*py+0.5*(param->cx[ 7]*param->cx[ 7]+param->cy[ 7]*param->cy[ 7])*E;
    PHI( 6) = 1.+param->cx[ 6]*px+param->cy[ 6]*py+0.5*(param->cx[ 6]*param->cx[ 6]+param->cy[ 6]*param->cy[ 6])*E;
    PHI(33) = 1.+param->cx[33]*px+param->cy[33]*py+0.5*(param->cx[33]*param->cx[33]+param->cy[33]*param->cy[33])*E;
    PHI(32) = 1.+param->cx[32]*px+param->cy[32]*py+0.5*(param->cx[32]*param->cx[32]+param->cy[32]*param->cy[32])*E;
    PHI( 2) = 1.+param->cx[ 2]*px+param->cy[ 2]*py+0.5*(param->cx[ 2]*param->cx[ 2]+param->cy[ 2]*param->cy[ 2])*E;
    PHI(36) = 1.+param->cx[36]*px+param->cy[36]*py+0.5*(param->cx[36]*param->cx[36]+param->cy[36]*param->cy[36])*E; 

    S =
         PHI(19) + PHI(20) + PHI(21) + PHI(12) + PHI(13) +
         PHI(14) + PHI(26) + PHI(27) + PHI(28) + PHI( 7) +
         PHI( 6) + PHI(33) + PHI(32) + PHI( 2) + PHI(36);

    FPOST(19) = PHI(19)*N/S;
    FPOST(20) = PHI(20)*N/S;
    FPOST(21) = PHI(21)*N/S;

    FPOST(12) = PHI(12)*N/S;
    FPOST(13) = PHI(13)*N/S;
    FPOST(14) = PHI(14)*N/S;

    FPOST(26) = PHI(26)*N/S;
    FPOST(27) = PHI(27)*N/S;
    FPOST(28) = PHI(28)*N/S;

    FPOST( 7) = PHI( 7)*N/S;
    FPOST( 6) = PHI( 6)*N/S;

    FPOST(33) = PHI(33)*N/S;
    FPOST(32) = PHI(32)*N/S;

    FPOST( 2) = PHI( 2)*N/S;
    FPOST(36) = PHI(36)*N/S;

    ////////////////////END LOWER LAYER1////////////////////

    //////////LOWER LAYER 2/////////////////////////////////

    iy     = HY + 1;
    idx = ix * NY + iy;

    //curY = 1;
    off  = 2;

    //////////////////////////////////////////////////////////////////////////
    // Compute local u, v, rho e temp.

    rho   = 0.0;
    u     = 0.0;
    v     = 0.0;
    temp  = 0.0;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      rho = rho + LOCALPOP(p);
      u   = u + param->cx[p] * LOCALPOP(p);
      v   = v + param->cy[p] * LOCALPOP(p);
    }

    //forcex = FORCEPOIS * DELTAT;
    forcey =   GRAVITY * DELTAT * rho;
    
    rhoi  = 1.0 / rho;
    u     = u * rhoi;
    v     = v * rhoi;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
   for (p = 0; p < NPOP; p++) {
      scalar =  (param->cx[p] - u) * (param->cx[p] - u) + (param->cy[p] - v) * (param->cy[p] - v);
      temp   = temp + 0.5 * scalar * LOCALPOP(p);
    }

    temp = temp * rhoi;

    localTempfic = temp + (forcex * forcex + forcey * forcey) * rhoi * rhoi / 8.0;

    //////////////////////////////////////////////////////////////////////////

#ifdef ADIABATIC

    temptilde1 = localTempfic ;
    temptilde2 = localTempfic ;
    temptilde3 = localTempfic ;

#else

    temptilde1 = TEMPWALLDOWN;
    temptilde2 = TEMPWALLDOWN;
    temptilde3 = TEMPWALLDOWN;

    xa[1] = 3.0;
    xa[2] = 4.0;

    ya[1] = temptilde3;
    // In the original code here we found:
    //   ya[2]=tempfic[idx3];
    // the avaluation of tempfic is made above, so
    // we replace it with the following:
    ya[2] = localTempfic;

    // fprintf(stdout,"x: %03d %f\n", i, ya[2]);

    xa[0] = 2.0;
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0] = 1.0;
    _polint(xa,ya);
    temptilde1 = ya[0];

#endif

    // X velocity.
    utilde1 = 0.0;
    utilde2 = 0.0;
    utilde3 = 0.0;
    // Y velocity.
    utilde1 = 0.0;
    utilde2 = 0.0;

    N = FPRE(15) + FPRE(22) + FPRE( 8) + FPRE(29) + FPRE(23) + FPRE(16) + FPRE( 9) + FPRE( 3);

    massapost = N + 
            FPOST(36) + FPOST(32) + FPOST(26) +       
            FPOST(19) + FPOST(12) + FPOST( 6) +
            FPOST( 2) + FPOST(35) + FPOST(31) +
            FPOST(25) + FPOST(11) + FPOST( 5) +
            FPOST( 1) + FPOST(34) + FPOST(30) +
            FPOST(24) + FPOST(17) + FPOST(10) +
            FPOST( 4) + FPOST( 0) + FPOST(29) +
            FPOST(23) + FPOST(16) + FPOST( 9) +
            FPOST( 3) + FPOST(22) + FPOST(15) +
            FPOST( 8) + FPOST(18);

    ptildex = massapost*(utilde2) - 0.5*FORCEPOIS*DELTAT;
    ptildey = massapost*(vtilde2) - 0.5*GRAVITY*massapost*DELTAT;

    Etilde = (temptilde2)*massapost+
                 0.5*(ptildex*ptildex+ptildey*ptildey)/massapost;

    Ox =
            param->cx[36]*FPOST(36) + param->cx[32]*FPOST(32) +
            param->cx[26]*FPOST(26) + param->cx[19]*FPOST(19) +
            param->cx[12]*FPOST(12) + param->cx[ 6]*FPOST( 6) +
            param->cx[ 2]*FPOST( 2) + param->cx[35]*FPOST(35) +
            param->cx[31]*FPOST(31) + param->cx[25]*FPOST(25) +
            param->cx[11]*FPOST(11) + param->cx[ 5]*FPOST( 5) +
            param->cx[ 1]*FPOST( 1) + param->cx[34]*FPOST(34) +
            param->cx[30]*FPOST(30) + param->cx[24]*FPOST(24) +
            param->cx[17]*FPOST(17) + param->cx[10]*FPOST(10) +
            param->cx[ 4]*FPOST( 4) + param->cx[ 0]*FPOST( 0) +
            param->cx[29]*FPOST(29) + param->cx[23]*FPOST(23) +
            param->cx[16]*FPOST(16) + param->cx[ 9]*FPOST( 9) +
            param->cx[ 3]*FPOST( 3) + param->cx[22]*FPOST(22) +
            param->cx[15]*FPOST(15) + param->cx[ 8]*FPOST( 8);

    Oy =
            param->cy[36]*FPOST(36) + param->cy[32]*FPOST(32) +
            param->cy[26]*FPOST(26) + param->cy[19]*FPOST(19) +
            param->cy[12]*FPOST(12) + param->cy[ 6]*FPOST( 6) +
            param->cy[ 2]*FPOST( 2) + param->cy[35]*FPOST(35) +
            param->cy[31]*FPOST(31) + param->cy[25]*FPOST(25) +
            param->cy[11]*FPOST(11) + param->cy[ 5]*FPOST( 5) +
            param->cy[ 1]*FPOST( 1) + param->cy[34]*FPOST(34) +
            param->cy[30]*FPOST(30) + param->cy[24]*FPOST(24) +
            param->cy[17]*FPOST(17) + param->cy[10]*FPOST(10) +
            param->cy[ 4]*FPOST( 4) + param->cy[ 0]*FPOST( 0) +
            param->cy[29]*FPOST(29) + param->cy[23]*FPOST(23) +
            param->cy[16]*FPOST(16) + param->cy[ 9]*FPOST( 9) +
            param->cy[ 3]*FPOST( 3) + param->cy[22]*FPOST(22) +
            param->cy[15]*FPOST(15) + param->cy[ 8]*FPOST( 8);


    OE =
          0.5*(param->cx[36]*param->cx[36] + param->cy[36]*param->cy[36])*FPOST(36) +
          0.5*(param->cx[32]*param->cx[32] + param->cy[32]*param->cy[32])*FPOST(32) +
          0.5*(param->cx[26]*param->cx[26] + param->cy[26]*param->cy[26])*FPOST(26) +
          0.5*(param->cx[19]*param->cx[19] + param->cy[19]*param->cy[19])*FPOST(19) +
          0.5*(param->cx[12]*param->cx[12] + param->cy[12]*param->cy[12])*FPOST(12) +
          0.5*(param->cx[ 6]*param->cx[ 6] + param->cy[ 6]*param->cy[ 6])*FPOST( 6) +
          0.5*(param->cx[ 2]*param->cx[ 2] + param->cy[ 2]*param->cy[ 2])*FPOST( 2) +
          0.5*(param->cx[35]*param->cx[35] + param->cy[35]*param->cy[35])*FPOST(35) +
          0.5*(param->cx[31]*param->cx[31] + param->cy[31]*param->cy[31])*FPOST(31) +
          0.5*(param->cx[25]*param->cx[25] + param->cy[25]*param->cy[25])*FPOST(25) +
          0.5*(param->cx[11]*param->cx[11] + param->cy[11]*param->cy[11])*FPOST(11) +
          0.5*(param->cx[ 5]*param->cx[ 5] + param->cy[ 5]*param->cy[ 5])*FPOST( 5) +
          0.5*(param->cx[ 1]*param->cx[ 1] + param->cy[ 1]*param->cy[ 1])*FPOST( 1) +
          0.5*(param->cx[34]*param->cx[34] + param->cy[34]*param->cy[34])*FPOST(34) +
          0.5*(param->cx[30]*param->cx[30] + param->cy[30]*param->cy[30])*FPOST(30) +
          0.5*(param->cx[24]*param->cx[24] + param->cy[24]*param->cy[24])*FPOST(24) +
          0.5*(param->cx[17]*param->cx[17] + param->cy[17]*param->cy[17])*FPOST(17) + 
          0.5*(param->cx[10]*param->cx[10] + param->cy[10]*param->cy[10])*FPOST(10) +
          0.5*(param->cx[ 4]*param->cx[ 4] + param->cy[ 4]*param->cy[ 4])*FPOST( 4) +
          0.5*(param->cx[ 0]*param->cx[ 0] + param->cy[ 0]*param->cy[ 0])*FPOST( 0) +
          0.5*(param->cx[29]*param->cx[29] + param->cy[29]*param->cy[29])*FPOST(29) +
          0.5*(param->cx[23]*param->cx[23] + param->cy[23]*param->cy[23])*FPOST(23) +
          0.5*(param->cx[16]*param->cx[16] + param->cy[16]*param->cy[16])*FPOST(16) +
          0.5*(param->cx[ 9]*param->cx[ 9] + param->cy[ 9]*param->cy[ 9])*FPOST( 9) +
          0.5*(param->cx[ 3]*param->cx[ 3] + param->cy[ 3]*param->cy[ 3])*FPOST( 3) +
          0.5*(param->cx[22]*param->cx[22] + param->cy[22]*param->cy[22])*FPOST(22) +
          0.5*(param->cx[15]*param->cx[15] + param->cy[15]*param->cy[15])*FPOST(15) +
          0.5*(param->cx[ 8]*param->cx[ 8] + param->cy[ 8]*param->cy[ 8])*FPOST( 8) ;

    a1 = 19.*(ptildex-Ox)*UNIT;
    b1 = -12.*N*UNIT*UNIT;
    c1 = (59/2.)*(ptildex-Ox)*UNIT*UNIT;
    d1 = 8.*(ptildex-Ox);

    a2 = 19.*(ptildey-Oy)*UNIT-47.*N*UNIT*UNIT;
    c2 = (59/2.)*(ptildey-Oy)*UNIT*UNIT-(147./2.)*N*UNIT*UNIT*UNIT;
    d2 = 8.*(ptildey-Oy)-19.*N*UNIT;

    a3 = 19.*(Etilde-OE)*UNIT-(147./2.)*N*UNIT*UNIT*UNIT;
    c3 = (59./2.)*(Etilde-OE)*UNIT*UNIT-N*UNIT*UNIT*UNIT*UNIT*475./4.;
    d3 = 8.*(Etilde-OE)-(59./2.)*N*UNIT*UNIT;


    py = (-c3*d2+c2*d3)/(-a3*c2+a2*c3);

    px = (a2*c3*d1-a2*c1*d3-a3*c2*d1-c3*a1*d2+c1*a3*d2+a1*c2*d3)/(b1*(a3*c2-a2*c3));

    E  = (-a3*d2+a2*d3)/(a3*c2-a2*c3);

    PHI(20) = 1.+param->cx[20]*px+param->cy[20]*py+0.5*(param->cx[20]*param->cx[20]+param->cy[20]*param->cy[20])*E;
    PHI(21) = 1.+param->cx[21]*px+param->cy[21]*py+0.5*(param->cx[21]*param->cx[21]+param->cy[21]*param->cy[21])*E;
    PHI(13) = 1.+param->cx[13]*px+param->cy[13]*py+0.5*(param->cx[13]*param->cx[13]+param->cy[13]*param->cy[13])*E;
    PHI(14) = 1.+param->cx[14]*px+param->cy[14]*py+0.5*(param->cx[14]*param->cx[14]+param->cy[14]*param->cy[14])*E;
    PHI(27) = 1.+param->cx[27]*px+param->cy[27]*py+0.5*(param->cx[27]*param->cx[27]+param->cy[27]*param->cy[27])*E;
    PHI(28) = 1.+param->cx[28]*px+param->cy[28]*py+0.5*(param->cx[28]*param->cx[28]+param->cy[28]*param->cy[28])*E;
    PHI( 7) = 1.+param->cx[ 7]*px+param->cy[ 7]*py+0.5*(param->cx[ 7]*param->cx[ 7]+param->cy[ 7]*param->cy[ 7])*E;
    PHI(33) = 1.+param->cx[33]*px+param->cy[33]*py+0.5*(param->cx[33]*param->cx[33]+param->cy[33]*param->cy[33])*E;

    S = PHI(20) + PHI(21) + PHI(13) + PHI(14) + 
        PHI(27) + PHI(28) + PHI( 7) + PHI(33);
    
    FPOST(20) = PHI(20)*N/S;  
    FPOST(21) = PHI(21)*N/S;  
          
    FPOST(13) = PHI(13)*N/S;
    FPOST(14) = PHI(14)*N/S;  
              
    FPOST(27) = PHI(27)*N/S;  
    FPOST(28) = PHI(28)*N/S;

    FPOST( 7) = PHI( 7)*N/S;  

    FPOST(33) = PHI(33)*N/S;  

    ////////////END LOWER LAYER 2////////////////////////

    //////////LOWER LAYER 3/////////////////////////////////

    iy     = HY + 2;
    idx = ix * NY + iy;

    //curY = 2;
    off  = 1;
          
    //////////////////////////////////////////////////////////////////////////
    // Compute local u, v, rho e temp.

    rho   = 0.0;
    u     = 0.0;
    v     = 0.0;
    temp  = 0.0;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
   for (p = 0; p < NPOP; p++) {
      rho = rho + LOCALPOP(p);
      u   = u + param->cx[p] * LOCALPOP(p);
      v   = v + param->cy[p] * LOCALPOP(p);
    }

    //forcex = FORCEPOIS * DELTAT;
    forcey = GRAVITY * DELTAT * rho;
    
    rhoi  = 1.0 / rho;
    u     = u * rhoi;
    v     = v * rhoi;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
   for (p = 0; p < NPOP; p++) {
      scalar =  (param->cx[p] - u) * (param->cx[p] - u) + (param->cy[p] - v) * (param->cy[p] - v);
      temp   = temp + 0.5 * scalar * LOCALPOP(p);
    }

    temp = temp * rhoi;

    localTempfic = temp + (forcex * forcex + forcey * forcey) * rhoi * rhoi / 8.0;

    //////////////////////////////////////////////////////////////////////////

#ifdef ADIABATIC

    temptilde1 = localTempfic ;
    temptilde2 = localTempfic ;
    temptilde3 = localTempfic ;

#else

    temptilde1 = TEMPWALLDOWN;
    temptilde2 = TEMPWALLDOWN;
    temptilde3 = TEMPWALLDOWN;

    xa[1] = 3.0;
    xa[2] = 4.0;

    ya[1] = temptilde3;
    // In the original code here we found:
    //   ya[2]=tempfic[idx3];
    // the avaluation of tempfic is made above, so
    // we replace it with the following:
    ya[2] = localTempfic;

    // fprintf(stdout,"x: %03d %f\n", i, ya[2]);

    xa[0] = 2.0;
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0] = 1.0;
    _polint(xa,ya);
    temptilde1 = ya[0];

#endif

    // X velocity.
    utilde1 = 0.0;
    utilde2 = 0.0;
    utilde3 = 0.0;
    // Y velocity.
    utilde1 = 0.0;
    utilde2 = 0.0;

    //N=(f2[idx2].p[15]+f2[idx2].p[22]+f2[idx2].p[ 8]); new

    term1 = 
        param->cy[33]*FPOST(33) + param->cy[27]*FPOST(27) +
        param->cy[20]*FPOST(20) + param->cy[13]*FPOST(13) +
        param->cy[ 7]*FPOST( 7) + param->cy[36]*FPOST(36) +
        param->cy[32]*FPOST(32) + param->cy[26]*FPOST(26) +
        param->cy[19]*FPOST(19) + param->cy[12]*FPOST(12) +
        param->cy[ 6]*FPOST( 6) + param->cy[ 2]*FPOST( 2) +
        param->cy[35]*FPOST(35) + param->cy[31]*FPOST(31) +
        param->cy[25]*FPOST(25) + param->cy[11]*FPOST(11) +
        param->cy[ 5]*FPOST( 5) + param->cy[ 1]*FPOST( 1) +
        param->cy[34]*FPOST(34) + param->cy[30]*FPOST(30) +
        param->cy[24]*FPOST(24) + param->cy[17]*FPOST(17) +
        param->cy[10]*FPOST(10) + param->cy[ 4]*FPOST( 4) +
        param->cy[ 0]*FPOST( 0) + param->cy[29]*FPOST(29) +
        param->cy[23]*FPOST(23) + param->cy[16]*FPOST(16) +
        param->cy[ 9]*FPOST( 9) + param->cy[ 3]*FPOST( 3) +
        param->cy[22]*FPOST(22) + param->cy[15]*FPOST(15) +
        param->cy[ 8]*FPOST( 8);

    term2 = 
        FPOST(33) + FPOST(27) +
        FPOST(20) + FPOST(13) +
        FPOST( 7) + FPOST(36) +
        FPOST(32) + FPOST(26) +
        FPOST(19) + FPOST(12) +
        FPOST( 6) + FPOST( 2) +
        FPOST(35) + FPOST(31) +
        FPOST(25) + FPOST(11) +
        FPOST( 5) + FPOST( 1) +
        FPOST(34) + FPOST(30) +
        FPOST(24) + FPOST(17) +
        FPOST(10) + FPOST( 4) +
        FPOST( 0) + FPOST(29) +
        FPOST(23) + FPOST(16) +
        FPOST( 9) + FPOST( 3) +
        FPOST(22) + FPOST(15) +
        FPOST( 8) + FPOST(18) +
        (FPRE(15) +  FPRE(22) + FPRE( 8));

    N = -term1/(3.*UNIT)-0.5*GRAVITY*DELTAT*(term2)/(3.*UNIT);

    FPOST(18) = FPOST(18) - N + (FPRE(15)+FPRE(22)+FPRE( 8)); 

    massapost = N + 
            FPOST(33) + FPOST(27) +
            FPOST(20) + FPOST(13) +
            FPOST( 7) + FPOST(36) +
            FPOST(32) + FPOST(26) +
            FPOST(19) + FPOST(12) +
            FPOST( 6) + FPOST( 2) +
            FPOST(35) + FPOST(31) +
            FPOST(25) + FPOST(11) +
            FPOST( 5) + FPOST( 1) +
            FPOST(34) + FPOST(30) +
            FPOST(24) + FPOST(17) +
            FPOST(10) + FPOST( 4) +
            FPOST( 0) + FPOST(29) +
            FPOST(23) + FPOST(16) +
            FPOST( 9) + FPOST( 3) +
            FPOST(22) + FPOST(15) +
            FPOST( 8) + FPOST(18);
                
    ptildex=massapost*(utilde3)-0.5*FORCEPOIS*DELTAT;

    ptildey = 3. * N * UNIT + 
        param->cy[33]*FPOST(33) + param->cy[27]*FPOST(27) +
        param->cy[20]*FPOST(20) + param->cy[13]*FPOST(13) +
        param->cy[ 7]*FPOST( 7) + param->cy[36]*FPOST(36) +
        param->cy[32]*FPOST(32) + param->cy[26]*FPOST(26) +
        param->cy[19]*FPOST(19) + param->cy[12]*FPOST(12) +
        param->cy[ 6]*FPOST( 6) + param->cy[ 2]*FPOST( 2) +
        param->cy[35]*FPOST(35) + param->cy[31]*FPOST(31) +
        param->cy[25]*FPOST(25) + param->cy[11]*FPOST(11) +
        param->cy[ 5]*FPOST( 5) + param->cy[ 1]*FPOST( 1) +
        param->cy[34]*FPOST(34) + param->cy[30]*FPOST(30) +
        param->cy[24]*FPOST(24) + param->cy[17]*FPOST(17) +
        param->cy[10]*FPOST(10) + param->cy[ 4]*FPOST( 4) +
        param->cy[ 0]*FPOST( 0) + param->cy[29]*FPOST(29) +
        param->cy[23]*FPOST(23) + param->cy[16]*FPOST(16) +
        param->cy[ 9]*FPOST( 9) + param->cy[ 3]*FPOST( 3) +
        param->cy[22]*FPOST(22) + param->cy[15]*FPOST(15) +
        param->cy[ 8]*FPOST( 8);

    Etilde = (temptilde3)*massapost +
         0.5*(ptildex*ptildex + ptildey*ptildey)/massapost;


    Ox = 
        param->cx[33]*FPOST(33) + param->cx[27]*FPOST(27) +
        param->cx[20]*FPOST(20) + param->cx[13]*FPOST(13) +
        param->cx[ 7]*FPOST( 7) + param->cx[36]*FPOST(36) +
        param->cx[32]*FPOST(32) + param->cx[26]*FPOST(26) +
        param->cx[19]*FPOST(19) + param->cx[12]*FPOST(12) +
        param->cx[ 6]*FPOST( 6) + param->cx[ 2]*FPOST( 2) +
        param->cx[35]*FPOST(35) + param->cx[31]*FPOST(31) +
        param->cx[25]*FPOST(25) + param->cx[11]*FPOST(11) +
        param->cx[ 5]*FPOST( 5) + param->cx[ 1]*FPOST( 1) +
        param->cx[34]*FPOST(34) + param->cx[30]*FPOST(30) +
        param->cx[24]*FPOST(24) + param->cx[17]*FPOST(17) +
        param->cx[10]*FPOST(10) + param->cx[ 4]*FPOST( 4) +
        param->cx[ 0]*FPOST( 0) + param->cx[29]*FPOST(29) +
        param->cx[23]*FPOST(23) + param->cx[16]*FPOST(16) +
        param->cx[ 9]*FPOST( 9) + param->cx[ 3]*FPOST( 3) +
        param->cx[22]*FPOST(22) + param->cx[15]*FPOST(15) +
        param->cx[ 8]*FPOST( 8);

    OE = 
        0.5*(param->cx[33]*param->cx[33]+param->cy[33]*param->cy[33])*FPOST(33) +
        0.5*(param->cx[27]*param->cx[27]+param->cy[27]*param->cy[27])*FPOST(27) +
        0.5*(param->cx[20]*param->cx[20]+param->cy[20]*param->cy[20])*FPOST(20) +
        0.5*(param->cx[13]*param->cx[13]+param->cy[13]*param->cy[13])*FPOST(13) +
        0.5*(param->cx[ 7]*param->cx[ 7]+param->cy[ 7]*param->cy[ 7])*FPOST( 7) +
        0.5*(param->cx[36]*param->cx[36]+param->cy[36]*param->cy[36])*FPOST(36) +
        0.5*(param->cx[32]*param->cx[32]+param->cy[32]*param->cy[32])*FPOST(32) +
        0.5*(param->cx[26]*param->cx[26]+param->cy[26]*param->cy[26])*FPOST(26) +
        0.5*(param->cx[19]*param->cx[19]+param->cy[19]*param->cy[19])*FPOST(19) +
        0.5*(param->cx[12]*param->cx[12]+param->cy[12]*param->cy[12])*FPOST(12) +
        0.5*(param->cx[ 6]*param->cx[ 6]+param->cy[ 6]*param->cy[ 6])*FPOST( 6) +
        0.5*(param->cx[ 2]*param->cx[ 2]+param->cy[ 2]*param->cy[ 2])*FPOST( 2) +
        0.5*(param->cx[35]*param->cx[35]+param->cy[35]*param->cy[35])*FPOST(35) +
        0.5*(param->cx[31]*param->cx[31]+param->cy[31]*param->cy[31])*FPOST(31) +
        0.5*(param->cx[25]*param->cx[25]+param->cy[25]*param->cy[25])*FPOST(25) +
        0.5*(param->cx[11]*param->cx[11]+param->cy[11]*param->cy[11])*FPOST(11) +
        0.5*(param->cx[ 5]*param->cx[ 5]+param->cy[ 5]*param->cy[ 5])*FPOST( 5) +
        0.5*(param->cx[ 1]*param->cx[ 1]+param->cy[ 1]*param->cy[ 1])*FPOST( 1) +
        0.5*(param->cx[34]*param->cx[34]+param->cy[34]*param->cy[34])*FPOST(34) +
        0.5*(param->cx[30]*param->cx[30]+param->cy[30]*param->cy[30])*FPOST(30) +
        0.5*(param->cx[24]*param->cx[24]+param->cy[24]*param->cy[24])*FPOST(24) +
        0.5*(param->cx[17]*param->cx[17]+param->cy[17]*param->cy[17])*FPOST(17) + 
        0.5*(param->cx[10]*param->cx[10]+param->cy[10]*param->cy[10])*FPOST(10) +
        0.5*(param->cx[ 4]*param->cx[ 4]+param->cy[ 4]*param->cy[ 4])*FPOST( 4) +
        0.5*(param->cx[ 0]*param->cx[ 0]+param->cy[ 0]*param->cy[ 0])*FPOST( 0) +
        0.5*(param->cx[29]*param->cx[29]+param->cy[29]*param->cy[29])*FPOST(29) +
        0.5*(param->cx[23]*param->cx[23]+param->cy[23]*param->cy[23])*FPOST(23) +
        0.5*(param->cx[16]*param->cx[16]+param->cy[16]*param->cy[16])*FPOST(16) +
        0.5*(param->cx[ 9]*param->cx[ 9]+param->cy[ 9]*param->cy[ 9])*FPOST( 9) +
        0.5*(param->cx[ 3]*param->cx[ 3]+param->cy[ 3]*param->cy[ 3])*FPOST( 3) +
        0.5*(param->cx[22]*param->cx[22]+param->cy[22]*param->cy[22])*FPOST(22) +
        0.5*(param->cx[15]*param->cx[15]+param->cy[15]*param->cy[15])*FPOST(15) +
        0.5*(param->cx[ 8]*param->cx[ 8]+param->cy[ 8]*param->cy[ 8])*FPOST( 8);

    a1 = -2.*N*UNIT*UNIT;
    b1 = (29./2.)*(ptildex-Ox)*UNIT*UNIT;
    d1 = 3.*(ptildex-Ox);

    b2 = (Etilde-OE)*(29./2.)*UNIT*UNIT-(281./4.)*N*UNIT*UNIT*UNIT*UNIT;
    d2 = 3.*(Etilde-OE)-(29./2.)*N*UNIT*UNIT;

    E = -d2/b2;

    px = (b1*d2-d1*b2)/(a1*b2);

    PHI(21) = 1.+param->cx[21]*px+0.5*(param->cx[21]*param->cx[21]+param->cy[21]*param->cy[21])*E;
    PHI(14) = 1.+param->cx[14]*px+0.5*(param->cx[14]*param->cx[14]+param->cy[14]*param->cy[14])*E;
    PHI(28) = 1.+param->cx[28]*px+0.5*(param->cx[28]*param->cx[28]+param->cy[28]*param->cy[28])*E;

    S = PHI(21)+PHI(14)+PHI(28);

    FPOST(21)=PHI(21)*N/S;  
    FPOST(14)=PHI(14)*N/S;  
    FPOST(28)=PHI(28)*N/S;

    ////////////END LOWER LAYER 3////////////////////////

  } // this closes the loop over ix
      
    //////////////////////////////////////////////////////////////////////////////

  #undef PHI
  #undef LOCALPOP
  #undef FPRE
  #undef FPOST
} // this closes the function body


///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void bcTop ( data_t * __restrict__ const nxt, const data_t* __restrict__ const prv, 
            const par_t * __restrict__ const param, const int startX, const int endX, queue_t queue){

  #define PHI(_ii)      nxt[ ((_ii)*NX*NY) + idx       ]
  #define LOCALPOP(_ii) nxt[ ((_ii)*NX*NY) + idx + off ]
  #define FPRE(_ii)     prv[ ((_ii)*NX*NY) + idx       ]
  #define FPOST(_ii)    nxt[ ((_ii)*NX*NY) + idx       ]

  unsigned long ix;
  
  //////////////////////////////////////////////////////////////////////////////
  ////////////////////// Upper BORDER //////////////////////////////////////////

#ifdef SPEC_OPENACC
  #pragma acc kernels present(prv, nxt, param) async(queue)
  #pragma acc loop gang vector independent  
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for 
#elif defined(USE_TARGET)
  #pragma omp target teams private(ix)
  #pragma omp OMP_SCHED
#endif
  for ( ix = startX; ix < endX; ix++ ) {  

    unsigned long iy;
    unsigned long idx;
    
    int curY;

  //////////////////////////////////////////////////////////////////////////////

    double a1,a2,a3;
    double b1,b2;
    double c1,c2,c3;
    double d1,d2,d3;

    double utilde1,vtilde1;
    double utilde2,vtilde2;
    double utilde3;

    double term1,term2;

    double temptilde1,temptilde2,temptilde3;
    double localTempfic;

    double px,py,E,S,N,Ox,Oy,OE;
    double ptildex,ptildey,Etilde,massapost;

    #ifndef ADIABATIC
      data_t xa[ENNE+2], ya[ENNE+2];
    #endif

    double rhoi, forcey, scalar;
    
    int p, off;
    
    double  rho, temp, u, v;

    ////////////UPPER LAYER 1 ///////////////////

    iy     = HY+LSIZEY-1;
    idx = ix * NY + iy;
    curY   = iy-HY;
    off    = (LSIZEY - 4) - curY;

    //////////////////////////////////////////////////////////////////////////
    // Compute local u, v, rho e temp.

    rho   = 0.0;
    u     = 0.0;
    v     = 0.0;
    temp  = 0.0;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      rho = rho + LOCALPOP(p);
      u   = u + param->cx[p] * LOCALPOP(p);
      v   = v + param->cy[p] * LOCALPOP(p);
    }

    //forcex = FORCEPOIS * DELTAT;
    forcey = GRAVITY * DELTAT * rho;
    
    rhoi  = 1.0 / rho;
    u     = u * rhoi;
    v     = v * rhoi;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      scalar =  (param->cx[p] - u) * (param->cx[p] - u) + (param->cy[p] - v) * (param->cy[p] - v);
      temp   = temp + 0.5 * scalar * LOCALPOP(p);
    }

    temp = temp * rhoi;

    localTempfic = temp + (forcex * forcex + forcey * forcey) * rhoi * rhoi / 8.0;

#ifdef ADIABATIC

    temptilde1 = localTempfic ;
    temptilde2 = localTempfic ;
    temptilde3 = localTempfic ;

#else

    temptilde1 = TEMPWALLUP;
    temptilde2 = TEMPWALLUP;
    temptilde3 = TEMPWALLUP;

    xa[1] = 3.0;  //this point seems crucial! 
    xa[2] = 4.0;   

    ya[1] = TEMPWALLUP;
    ya[2] = localTempfic;

    xa[0] = 2.0;//I want it here
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0]=2.0;//I want it here
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0]=1.0;//I want it here
    _polint(xa,ya);
    temptilde1 = ya[0];

#endif
       
    //x velocity
    utilde1=0.0;
    utilde2=0.0;
    utilde3=0.0;

    ///y velocity
    vtilde1=0.0;
    vtilde2=0.0;
        

    N= FPRE(21) + FPRE(14) + FPRE(28) + 
       FPRE( 7) + FPRE(13) + FPRE(20) + FPRE(27) + FPRE(33) +
       FPRE( 2) + FPRE( 6) + FPRE(12) + FPRE(19) + FPRE(26) + FPRE(32) + FPRE(36);

    massapost = N +
      FPOST(35) + FPOST(31) + FPOST(25) +
      FPOST(11) + FPOST( 5) + FPOST( 1) + FPOST( 2) +
      FPOST( 6) + FPOST(12) + FPOST(19) + FPOST(26) +
      FPOST(32) + FPOST(36) + FPOST( 7) + FPOST(13) +
      FPOST(20) + FPOST(27) + FPOST(33) + FPOST(14) +
      FPOST(21) + FPOST(28) + FPOST(18);

    ptildex = massapost*(utilde1)-0.5*FORCEPOIS*DELTAT;
    ptildey = massapost*(vtilde1)-0.5*GRAVITY*massapost*DELTAT;

    Etilde = (temptilde1)*massapost+0.5*(ptildex*ptildex+ptildey*ptildey)/massapost;


    Ox=
        param->cx[35]*FPOST(35) + param->cx[31]*FPOST(31) + param->cx[25]*FPOST(25) +
        param->cx[11]*FPOST(11) + param->cx[ 5]*FPOST( 5) + param->cx[ 1]*FPOST( 1) +
        param->cx[ 2]*FPOST( 2) + param->cx[ 6]*FPOST( 6) + param->cx[12]*FPOST(12) +
        param->cx[19]*FPOST(19) + param->cx[26]*FPOST(26) + param->cx[32]*FPOST(32) +
        param->cx[36]*FPOST(36) + param->cx[ 7]*FPOST( 7) + param->cx[13]*FPOST(13) +
        param->cx[20]*FPOST(20) + param->cx[27]*FPOST(27) + param->cx[33]*FPOST(33) +
        param->cx[14]*FPOST(14) + param->cx[21]*FPOST(21) + param->cx[28]*FPOST(28) ;

    Oy =
        param->cy[35]*FPOST(35) + param->cy[31]*FPOST(31) + param->cy[25]*FPOST(25) +
        param->cy[11]*FPOST(11) + param->cy[ 5]*FPOST( 5) + param->cy[ 1]*FPOST( 1) +
        param->cy[ 2]*FPOST( 2) + param->cy[ 6]*FPOST( 6) + param->cy[12]*FPOST(12) +
        param->cy[19]*FPOST(19) + param->cy[26]*FPOST(26) + param->cy[32]*FPOST(32) +
        param->cy[36]*FPOST(36) + param->cy[ 7]*FPOST( 7) + param->cy[13]*FPOST(13) +
        param->cy[20]*FPOST(20) + param->cy[27]*FPOST(27) + param->cy[33]*FPOST(33) +
        param->cy[14]*FPOST(14) + param->cy[21]*FPOST(21) + param->cy[28]*FPOST(28);

    OE =
         0.5*(param->cx[35]*param->cx[35]+param->cy[35]*param->cy[35])*FPOST(35)+
         0.5*(param->cx[31]*param->cx[31]+param->cy[31]*param->cy[31])*FPOST(31)+
         0.5*(param->cx[25]*param->cx[25]+param->cy[25]*param->cy[25])*FPOST(25)+
         0.5*(param->cx[11]*param->cx[11]+param->cy[11]*param->cy[11])*FPOST(11)+
         0.5*(param->cx[ 5]*param->cx[ 5]+param->cy[ 5]*param->cy[ 5])*FPOST( 5)+
         0.5*(param->cx[ 1]*param->cx[ 1]+param->cy[ 1]*param->cy[ 1])*FPOST( 1)+
         0.5*(param->cx[ 2]*param->cx[ 2]+param->cy[ 2]*param->cy[ 2])*FPOST( 2)+
         0.5*(param->cx[ 6]*param->cx[ 6]+param->cy[ 6]*param->cy[ 6])*FPOST( 6)+
         0.5*(param->cx[12]*param->cx[12]+param->cy[12]*param->cy[12])*FPOST(12)+
         0.5*(param->cx[19]*param->cx[19]+param->cy[19]*param->cy[19])*FPOST(19)+ 
         0.5*(param->cx[26]*param->cx[26]+param->cy[26]*param->cy[26])*FPOST(26)+
         0.5*(param->cx[32]*param->cx[32]+param->cy[32]*param->cy[32])*FPOST(32)+
         0.5*(param->cx[36]*param->cx[36]+param->cy[36]*param->cy[36])*FPOST(36)+
         0.5*(param->cx[ 7]*param->cx[ 7]+param->cy[ 7]*param->cy[ 7])*FPOST( 7)+
         0.5*(param->cx[13]*param->cx[13]+param->cy[13]*param->cy[13])*FPOST(13)+
         0.5*(param->cx[20]*param->cx[20]+param->cy[20]*param->cy[20])*FPOST(20)+
         0.5*(param->cx[27]*param->cx[27]+param->cy[27]*param->cy[27])*FPOST(27)+
         0.5*(param->cx[33]*param->cx[33]+param->cy[33]*param->cy[33])*FPOST(33)+
         0.5*(param->cx[14]*param->cx[14]+param->cy[14]*param->cy[14])*FPOST(14)+
         0.5*(param->cx[21]*param->cx[21]+param->cy[21]*param->cy[21])*FPOST(21)+
         0.5*(param->cx[28]*param->cx[28]+param->cy[28]*param->cy[28])*FPOST(28);

    a1 = -26.*(ptildex-Ox)*UNIT;
    b1 = -40.*N*UNIT*UNIT;
    c1 = 47.*(ptildex-Ox)*UNIT*UNIT;
    d1 = 15.*(ptildex-Ox);

    a2 = -26.*(ptildey-Oy)*UNIT-54.*N*UNIT*UNIT;
    c2 = 47.*(ptildey-Oy)*UNIT*UNIT+91.*N*UNIT*UNIT*UNIT;
    d2 = 15.*(ptildey-Oy)+26.*N*UNIT;

    a3 = -26.*(Etilde-OE)*UNIT+91.*N*UNIT*UNIT*UNIT;
    c3 = 47.*(Etilde-OE)*UNIT*UNIT-N*UNIT*UNIT*UNIT*UNIT*367./2.;
    d3 = 15.*(Etilde-OE)-47.*N*UNIT*UNIT;

    py = (-c3*d2+c2*d3)/(-a3*c2+a2*c3);
    px = (a2*c3*d1-a2*c1*d3-a3*c2*d1-c3*a1*d2+c1*a3*d2+a1*c2*d3)/(b1*(a3*c2-a2*c3));

    E  = (-a3*d2+a2*d3)/(a3*c2-a2*c3);

    PHI(17) = 1.+param->cx[17]*px+param->cy[17]*py+0.5*(param->cx[17]*param->cx[17]+param->cy[17]*param->cy[17])*E;
    PHI(16) = 1.+param->cx[16]*px+param->cy[16]*py+0.5*(param->cx[16]*param->cx[16]+param->cy[16]*param->cy[16])*E;
    PHI(15) = 1.+param->cx[15]*px+param->cy[15]*py+0.5*(param->cx[15]*param->cx[15]+param->cy[15]*param->cy[15])*E;
    PHI(24) = 1.+param->cx[24]*px+param->cy[24]*py+0.5*(param->cx[24]*param->cx[24]+param->cy[24]*param->cy[24])*E;
    PHI(23) = 1.+param->cx[23]*px+param->cy[23]*py+0.5*(param->cx[23]*param->cx[23]+param->cy[23]*param->cy[23])*E;
    PHI(22) = 1.+param->cx[22]*px+param->cy[22]*py+0.5*(param->cx[22]*param->cx[22]+param->cy[22]*param->cy[22])*E;
    PHI(10) = 1.+param->cx[10]*px+param->cy[10]*py+0.5*(param->cx[10]*param->cx[10]+param->cy[10]*param->cy[10])*E;
    PHI( 9) = 1.+param->cx[ 9]*px+param->cy[ 9]*py+0.5*(param->cx[ 9]*param->cx[ 9]+param->cy[ 9]*param->cy[ 9])*E;
    PHI( 8) = 1.+param->cx[ 8]*px+param->cy[ 8]*py+0.5*(param->cx[ 8]*param->cx[ 8]+param->cy[ 8]*param->cy[ 8])*E;
    PHI(29) = 1.+param->cx[29]*px+param->cy[29]*py+0.5*(param->cx[29]*param->cx[29]+param->cy[29]*param->cy[29])*E;
    PHI(30) = 1.+param->cx[30]*px+param->cy[30]*py+0.5*(param->cx[30]*param->cx[30]+param->cy[30]*param->cy[30])*E;
    PHI( 3) = 1.+param->cx[ 3]*px+param->cy[ 3]*py+0.5*(param->cx[ 3]*param->cx[ 3]+param->cy[ 3]*param->cy[ 3])*E;
    PHI( 4) = 1.+param->cx[ 4]*px+param->cy[ 4]*py+0.5*(param->cx[ 4]*param->cx[ 4]+param->cy[ 4]*param->cy[ 4])*E;
    PHI(34) = 1.+param->cx[34]*px+param->cy[34]*py+0.5*(param->cx[34]*param->cx[34]+param->cy[34]*param->cy[34])*E;
    PHI( 0) = 1.+param->cx[ 0]*px+param->cy[ 0]*py+0.5*(param->cx[ 0]*param->cx[ 0]+param->cy[ 0]*param->cy[ 0])*E;


    S = PHI(17) + PHI(16) + PHI(15) + PHI(24) + PHI(23) + PHI(22) +
        PHI(10) + PHI( 9) + PHI( 8) + PHI(29) + PHI(30) + PHI( 3) +
        PHI( 4) + PHI(34) + PHI( 0); 

    FPOST(17) = PHI(17)*N/S; 
    FPOST(16) = PHI(16)*N/S;  
    FPOST(15) = PHI(15)*N/S;  

    FPOST(24) = PHI(24)*N/S;  
    FPOST(23) = PHI(23)*N/S;
    FPOST(22) = PHI(22)*N/S;  

    FPOST(10) = PHI(10)*N/S;  
    FPOST( 9) = PHI( 9)*N/S;  
    FPOST( 8) = PHI( 8)*N/S;

    FPOST(29) = PHI(29)*N/S;  
    FPOST(30) = PHI(30)*N/S;  

    FPOST( 3) = PHI( 3)*N/S;  
    FPOST( 4) = PHI( 4)*N/S;  

    FPOST(34) = PHI(34)*N/S;  
    FPOST( 0) = PHI( 0)*N/S;  

    //////////FINE UPPER  LAYER1////////////////////

    iy     = HY+LSIZEY-2;
    idx = ix * NY + iy;
    curY   = iy-HY;
    off    = (LSIZEY - 4) - curY;

    //////////////////////////////////////////////////////////////////////////
    // Compute local u, v, rho e temp.

    rho   = 0.0;
    u     = 0.0;
    v     = 0.0;
    temp  = 0.0;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      rho = rho + LOCALPOP(p);
      u   = u + param->cx[p] * LOCALPOP(p);
      v   = v + param->cy[p] * LOCALPOP(p);
    }
    
    //forcex = FORCEPOIS * DELTAT;
    forcey = GRAVITY * DELTAT * rho;
    
    rhoi  = 1.0 / rho;
    u     = u * rhoi;
    v     = v * rhoi;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      scalar =  (param->cx[p] - u) * (param->cx[p] - u) + (param->cy[p] - v) * (param->cy[p] - v);
      temp   = temp + 0.5 * scalar * LOCALPOP(p);
    }

    temp = temp * rhoi;

    localTempfic = temp + (forcex * forcex + forcey * forcey) * rhoi * rhoi / 8.0;

#ifdef ADIABATIC

    temptilde1 = localTempfic ;
    temptilde2 = localTempfic ;
    temptilde3 = localTempfic ;

#else

    temptilde1 = TEMPWALLUP;
    temptilde2 = TEMPWALLUP;
    temptilde3 = TEMPWALLUP;

    xa[1] = 3.0;  //this point seems crucial! 
    xa[2] = 4.0;   

    ya[1] = TEMPWALLUP;
    ya[2] = localTempfic;

    xa[0] = 2.0;//I want it here
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0]=2.0;//I want it here
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0]=1.0;//I want it here
    _polint(xa,ya);
    temptilde1 = ya[0];

#endif
       
    //x velocity
    utilde1=0.0;
    utilde2=0.0;
    utilde3=0.0;

    ///y velocity
    vtilde1=0.0;
    vtilde2=0.0;

    //////////UPPER LAYER 2/////////////////////////////////

    N = FPRE(21) + FPRE(14) + FPRE(28) +
        FPRE( 7) + FPRE(13) + FPRE(20) +
        FPRE(27) + FPRE(33) ;

    massapost = N +
        FPOST(34) + FPOST(30) + FPOST(24) +
        FPOST(17) + FPOST(10) + FPOST( 4) +
        FPOST( 0) + FPOST(35) + FPOST(31) +
        FPOST(25) + FPOST(11) + FPOST( 5) +
        FPOST( 1) + FPOST( 2) + FPOST( 6) +
        FPOST(12) + FPOST(19) + FPOST(26) +
        FPOST(32) + FPOST(36) + FPOST( 7) +
        FPOST(13) + FPOST(20) + FPOST(27) +
        FPOST(33) + FPOST(14) + FPOST(21) +
        FPOST(28) + FPOST(18);

    ptildex = massapost*(utilde2) - 0.5*FORCEPOIS*DELTAT;
    ptildey = massapost*(vtilde2) - 0.5*GRAVITY*massapost*DELTAT;

    Etilde  = (temptilde2)*massapost + 
          0.5*(ptildex*ptildex+ptildey*ptildey)/massapost;  

    Ox=
      param->cx[34]*FPOST(34) + param->cx[30]*FPOST(30) + param->cx[24]*FPOST(24) +
      param->cx[17]*FPOST(17) + param->cx[10]*FPOST(10) + param->cx[ 4]*FPOST( 4) +
      param->cx[ 0]*FPOST( 0) + param->cx[35]*FPOST(35) + param->cx[31]*FPOST(31) +
      param->cx[25]*FPOST(25) + param->cx[11]*FPOST(11) + param->cx[ 5]*FPOST( 5) +
      param->cx[ 1]*FPOST( 1) + param->cx[ 2]*FPOST( 2) + param->cx[ 6]*FPOST( 6) +
      param->cx[12]*FPOST(12) + param->cx[19]*FPOST(19) + param->cx[26]*FPOST(26) +
      param->cx[32]*FPOST(32) + param->cx[36]*FPOST(36) + param->cx[ 7]*FPOST( 7) +
      param->cx[13]*FPOST(13) + param->cx[20]*FPOST(20) + param->cx[27]*FPOST(27) +
      param->cx[33]*FPOST(33) + param->cx[14]*FPOST(14) + param->cx[21]*FPOST(21) +
      param->cx[28]*FPOST(28);

    Oy=
      param->cy[34]*FPOST(34) + param->cy[30]*FPOST(30) + param->cy[24]*FPOST(24) +
      param->cy[17]*FPOST(17) + param->cy[10]*FPOST(10) + param->cy[ 4]*FPOST( 4) +
      param->cy[ 0]*FPOST( 0) + param->cy[35]*FPOST(35) + param->cy[31]*FPOST(31) +
      param->cy[25]*FPOST(25) + param->cy[11]*FPOST(11) + param->cy[ 5]*FPOST( 5) +
      param->cy[ 1]*FPOST( 1) + param->cy[ 2]*FPOST( 2) + param->cy[ 6]*FPOST( 6) +
      param->cy[12]*FPOST(12) + param->cy[19]*FPOST(19) + param->cy[26]*FPOST(26) +
      param->cy[32]*FPOST(32) + param->cy[36]*FPOST(36) + param->cy[ 7]*FPOST( 7) +
      param->cy[13]*FPOST(13) + param->cy[20]*FPOST(20) + param->cy[27]*FPOST(27) +
      param->cy[33]*FPOST(33) + param->cy[14]*FPOST(14) + param->cy[21]*FPOST(21) +
      param->cy[28]*FPOST(28);

    OE=
      0.5 * (param->cx[34]*param->cx[34] + param->cy[34]*param->cy[34]) * FPOST(34) +
      0.5 * (param->cx[30]*param->cx[30] + param->cy[30]*param->cy[30]) * FPOST(30) +
      0.5 * (param->cx[24]*param->cx[24] + param->cy[24]*param->cy[24]) * FPOST(24) +
      0.5 * (param->cx[17]*param->cx[17] + param->cy[17]*param->cy[17]) * FPOST(17) +
      0.5 * (param->cx[10]*param->cx[10] + param->cy[10]*param->cy[10]) * FPOST(10) +
      0.5 * (param->cx[ 4]*param->cx[ 4] + param->cy[ 4]*param->cy[ 4]) * FPOST( 4) +
      0.5 * (param->cx[ 0]*param->cx[ 0] + param->cy[ 0]*param->cy[ 0]) * FPOST( 0) +
      0.5 * (param->cx[35]*param->cx[35] + param->cy[35]*param->cy[35]) * FPOST(35) +
      0.5 * (param->cx[31]*param->cx[31] + param->cy[31]*param->cy[31]) * FPOST(31) +
      0.5 * (param->cx[25]*param->cx[25] + param->cy[25]*param->cy[25]) * FPOST(25) +
      0.5 * (param->cx[11]*param->cx[11] + param->cy[11]*param->cy[11]) * FPOST(11) +
      0.5 * (param->cx[ 5]*param->cx[ 5] + param->cy[ 5]*param->cy[ 5]) * FPOST( 5) +
      0.5 * (param->cx[ 1]*param->cx[ 1] + param->cy[ 1]*param->cy[ 1]) * FPOST( 1) +
      0.5 * (param->cx[ 2]*param->cx[ 2] + param->cy[ 2]*param->cy[ 2]) * FPOST( 2) +
      0.5 * (param->cx[ 6]*param->cx[ 6] + param->cy[ 6]*param->cy[ 6]) * FPOST( 6) +
      0.5 * (param->cx[12]*param->cx[12] + param->cy[12]*param->cy[12]) * FPOST(12) +
      0.5 * (param->cx[19]*param->cx[19] + param->cy[19]*param->cy[19]) * FPOST(19) + 
      0.5 * (param->cx[26]*param->cx[26] + param->cy[26]*param->cy[26]) * FPOST(26) +
      0.5 * (param->cx[32]*param->cx[32] + param->cy[32]*param->cy[32]) * FPOST(32) +
      0.5 * (param->cx[36]*param->cx[36] + param->cy[36]*param->cy[36]) * FPOST(36) +
      0.5 * (param->cx[ 7]*param->cx[ 7] + param->cy[ 7]*param->cy[ 7]) * FPOST( 7) +
      0.5 * (param->cx[13]*param->cx[13] + param->cy[13]*param->cy[13]) * FPOST(13) +
      0.5 * (param->cx[20]*param->cx[20] + param->cy[20]*param->cy[20]) * FPOST(20) +
      0.5 * (param->cx[27]*param->cx[27] + param->cy[27]*param->cy[27]) * FPOST(27) +
      0.5 * (param->cx[33]*param->cx[33] + param->cy[33]*param->cy[33]) * FPOST(33) +
      0.5 * (param->cx[14]*param->cx[14] + param->cy[14]*param->cy[14]) * FPOST(14) +
      0.5 * (param->cx[21]*param->cx[21] + param->cy[21]*param->cy[21]) * FPOST(21) +
      0.5 * (param->cx[28]*param->cx[28] + param->cy[28]*param->cy[28]) * FPOST(28);
      
    a1 = -19.*(ptildex-Ox)*UNIT;
    b1 = -12.*N*UNIT*UNIT;
    c1 = (59/2.)*(ptildex-Ox)*UNIT*UNIT;
    d1 = 8.*(ptildex-Ox);

    a2 = -19.*(ptildey-Oy)*UNIT-47.*N*UNIT*UNIT;
    c2 = (59/2.)*(ptildey-Oy)*UNIT*UNIT+(147./2.)*N*UNIT*UNIT*UNIT;
    d2 = 8.*(ptildey-Oy)+19.*N*UNIT;

    a3 = -19.*(Etilde-OE)*UNIT+(147./2.)*N*UNIT*UNIT*UNIT;
    c3 = (59./2.)*(Etilde-OE)*UNIT*UNIT-N*UNIT*UNIT*UNIT*UNIT*475./4.;
    d3 = 8.*(Etilde-OE)-(59./2.)*N*UNIT*UNIT;

    py = (-c3*d2+c2*d3)/(-a3*c2+a2*c3);

    px = (a2*c3*d1-a2*c1*d3-a3*c2*d1-c3*a1*d2+c1*a3*d2+a1*c2*d3)/(b1*(a3*c2-a2*c3));

    E  = (-a3*d2+a2*d3)/(a3*c2-a2*c3);

    PHI(16) = 1.+param->cx[16]*px+param->cy[16]*py+0.5*(param->cx[16]*param->cx[16]+param->cy[16]*param->cy[16])*E;
    PHI(15) = 1.+param->cx[15]*px+param->cy[15]*py+0.5*(param->cx[15]*param->cx[15]+param->cy[15]*param->cy[15])*E;
    PHI(23) = 1.+param->cx[23]*px+param->cy[23]*py+0.5*(param->cx[23]*param->cx[23]+param->cy[23]*param->cy[23])*E;
    PHI(22) = 1.+param->cx[22]*px+param->cy[22]*py+0.5*(param->cx[22]*param->cx[22]+param->cy[22]*param->cy[22])*E;
    PHI( 9) = 1.+param->cx[ 9]*px+param->cy[ 9]*py+0.5*(param->cx[ 9]*param->cx[ 9]+param->cy[ 9]*param->cy[ 9])*E;
    PHI( 8) = 1.+param->cx[ 8]*px+param->cy[ 8]*py+0.5*(param->cx[ 8]*param->cx[ 8]+param->cy[ 8]*param->cy[ 8])*E;
    PHI(29) = 1.+param->cx[29]*px+param->cy[29]*py+0.5*(param->cx[29]*param->cx[29]+param->cy[29]*param->cy[29])*E;
    PHI( 3) = 1.+param->cx[ 3]*px+param->cy[ 3]*py+0.5*(param->cx[ 3]*param->cx[ 3]+param->cy[ 3]*param->cy[ 3])*E;    

    S = PHI(16) + PHI(15) + PHI(23) + PHI(22) + PHI( 9) + PHI( 8) + PHI(29) + PHI( 3);

    FPOST(16) = PHI(16)*N/S;  
    FPOST(15) = PHI(15)*N/S;  

    FPOST(23) = PHI(23)*N/S;
    FPOST(22) = PHI(22)*N/S;  

    FPOST( 9) = PHI( 9)*N/S;  
    FPOST( 8) = PHI( 8)*N/S;

    FPOST(29) = PHI(29)*N/S; 
    FPOST( 3) = PHI( 3)*N/S; 

    ////////////END UPPER LAYER 2////////////////////////

    iy     = HY+LSIZEY-3;
    idx = ix * NY + iy;
    curY   = iy-HY;
    off    = (LSIZEY - 4) - curY;

    //////////////////////////////////////////////////////////////////////////
    // Compute local u, v, rho e temp.

    rho   = 0.0;
    u     = 0.0;
    v     = 0.0;
    temp  = 0.0;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      rho = rho + LOCALPOP(p);
      u   = u + param->cx[p] * LOCALPOP(p);
      v   = v + param->cy[p] * LOCALPOP(p);
    }

    //forcex = FORCEPOIS * DELTAT;
    forcey = GRAVITY * DELTAT * rho;

    rhoi  = 1.0 / rho;
    u     = u * rhoi;
    v     = v * rhoi;

#ifdef SPEC_OPENACC
    #pragma acc loop seq independent
#endif
    for (p = 0; p < NPOP; p++) {
      scalar =  (param->cx[p] - u) * (param->cx[p] - u) + (param->cy[p] - v) * (param->cy[p] - v);
      temp   = temp + 0.5 * scalar * LOCALPOP(p);
    }

    temp = temp * rhoi;

    localTempfic = temp + (forcex * forcex + forcey * forcey) * rhoi * rhoi / 8.0;

#ifdef ADIABATIC

    temptilde1 = localTempfic ;
    temptilde2 = localTempfic ;
    temptilde3 = localTempfic ;

#else

    temptilde1 = TEMPWALLUP;
    temptilde2 = TEMPWALLUP;
    temptilde3 = TEMPWALLUP;

    xa[1] = 3.0;  //this point seems crucial! 
    xa[2] = 4.0;   

    ya[1] = TEMPWALLUP;
    ya[2] = localTempfic;

    xa[0] = 2.0;//I want it here
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0]=2.0;//I want it here
    _polint(xa,ya);
    temptilde2 = ya[0];

    xa[0]=1.0;//I want it here
    _polint(xa,ya);
    temptilde1 = ya[0];

#endif
       
    //x velocity
    utilde1=0.0;
    utilde2=0.0;
    utilde3=0.0;

    ///y velocity
    vtilde1=0.0;
    vtilde2=0.0;
      
    //////////////// UPPER LAYER 3////////////////////////

    term1=
      param->cy[29]*FPOST(29) + param->cy[23]*FPOST(23) +
      param->cy[16]*FPOST(16) + param->cy[ 9]*FPOST( 9) +
      param->cy[ 3]*FPOST( 3) + param->cy[34]*FPOST(34) +
      param->cy[30]*FPOST(30) + param->cy[24]*FPOST(24) +
      param->cy[17]*FPOST(17) + param->cy[10]*FPOST(10) +
      param->cy[ 4]*FPOST( 4) + param->cy[ 0]*FPOST( 0) +
      param->cy[35]*FPOST(35) + param->cy[31]*FPOST(31) +
      param->cy[25]*FPOST(25) + param->cy[11]*FPOST(11) +
      param->cy[ 5]*FPOST( 5) + param->cy[ 1]*FPOST( 1) +
      param->cy[ 2]*FPOST( 2) + param->cy[ 6]*FPOST( 6) +
      param->cy[12]*FPOST(12) + param->cy[19]*FPOST(19) +
      param->cy[26]*FPOST(26) + param->cy[32]*FPOST(32) +
      param->cy[36]*FPOST(36) + param->cy[ 7]*FPOST( 7) +
      param->cy[13]*FPOST(13) + param->cy[20]*FPOST(20) +
      param->cy[27]*FPOST(27) + param->cy[33]*FPOST(33) +
      param->cy[14]*FPOST(14) + param->cy[21]*FPOST(21) +
      param->cy[28]*FPOST(28);

    term2=
      FPOST(29) + FPOST(23) + FPOST(16) +
      FPOST( 9) + FPOST( 3) + FPOST(34) +
      FPOST(30) + FPOST(24) + FPOST(17) +
      FPOST(10) + FPOST( 4) + FPOST( 0) +
      FPOST(35) + FPOST(31) + FPOST(25) +
      FPOST(11) + FPOST( 5) + FPOST( 1) +
      FPOST( 2) + FPOST( 6) + FPOST(12) +
      FPOST(19) + FPOST(26) + FPOST(32) +
      FPOST(36) + FPOST( 7) + FPOST(13) +
      FPOST(20) + FPOST(27) + FPOST(33) +
      FPOST(14) + FPOST(21) + FPOST(28) +
      FPOST(18) + 
      (FPRE(21) +  FPRE(14) +  FPRE(28));


    N=-term1/(-3.*UNIT)-0.5*GRAVITY*DELTAT*(term2)/(-3.*UNIT);

    FPOST(18) = FPOST(18) - N + (FPRE(21) + FPRE(14) + FPRE(28));

    massapost = N +
      FPOST(29) + FPOST(23) + FPOST(16) +
      FPOST( 9) + FPOST( 3) + FPOST(34) +
      FPOST(30) + FPOST(24) + FPOST(17) +
      FPOST(10) + FPOST( 4) + FPOST( 0) +
      FPOST(35) + FPOST(31) + FPOST(25) +
      FPOST(11) + FPOST( 5) + FPOST( 1) +
      FPOST( 2) + FPOST( 6) + FPOST(12) +
      FPOST(19) + FPOST(26) + FPOST(32) +
      FPOST(36) + FPOST( 7) + FPOST(13) +
      FPOST(20) + FPOST(27) + FPOST(33) +
      FPOST(14) + FPOST(21) + FPOST(28) +
      FPOST(18);

    ptildex = massapost*(utilde3) - 0.5*FORCEPOIS*DELTAT;

    ptildey = -3.*N*UNIT+
      param->cy[29]*FPOST(29) +
      param->cy[23]*FPOST(23) + param->cy[16]*FPOST(16) +
      param->cy[ 9]*FPOST( 9) + param->cy[ 3]*FPOST( 3) +
      param->cy[34]*FPOST(34) + param->cy[30]*FPOST(30) +
      param->cy[24]*FPOST(24) + param->cy[17]*FPOST(17) +
      param->cy[10]*FPOST(10) + param->cy[ 4]*FPOST( 4) +
      param->cy[ 0]*FPOST( 0) + param->cy[35]*FPOST(35) +
      param->cy[31]*FPOST(31) + param->cy[25]*FPOST(25) +
      param->cy[11]*FPOST(11) + param->cy[ 5]*FPOST( 5) +
      param->cy[ 1]*FPOST( 1) + param->cy[ 2]*FPOST( 2) +
      param->cy[ 6]*FPOST( 6) + param->cy[12]*FPOST(12) +
      param->cy[19]*FPOST(19) + param->cy[26]*FPOST(26) +
      param->cy[32]*FPOST(32) + param->cy[36]*FPOST(36) +
      param->cy[ 7]*FPOST( 7) + param->cy[13]*FPOST(13) +
      param->cy[20]*FPOST(20) + param->cy[27]*FPOST(27) +
      param->cy[33]*FPOST(33) + param->cy[14]*FPOST(14) +
      param->cy[21]*FPOST(21) + param->cy[28]*FPOST(28);

    Etilde=(temptilde3)*massapost+0.5*(ptildex*ptildex+ptildey*ptildey)/massapost;

    Ox=
      param->cx[29]*FPOST(29) + param->cx[23]*FPOST(23) +
      param->cx[16]*FPOST(16) + param->cx[ 9]*FPOST( 9) +
      param->cx[ 3]*FPOST( 3) + param->cx[34]*FPOST(34) +
      param->cx[30]*FPOST(30) + param->cx[24]*FPOST(24) +
      param->cx[17]*FPOST(17) + param->cx[10]*FPOST(10) +
      param->cx[ 4]*FPOST( 4) + param->cx[ 0]*FPOST( 0) +
      param->cx[35]*FPOST(35) + param->cx[31]*FPOST(31) +
      param->cx[25]*FPOST(25) + param->cx[11]*FPOST(11) +
      param->cx[ 5]*FPOST( 5) + param->cx[ 1]*FPOST( 1) +
      param->cx[ 2]*FPOST( 2) + param->cx[ 6]*FPOST( 6) +
      param->cx[12]*FPOST(12) + param->cx[19]*FPOST(19) +
      param->cx[26]*FPOST(26) + param->cx[32]*FPOST(32) +
      param->cx[36]*FPOST(36) + param->cx[ 7]*FPOST( 7) +
      param->cx[13]*FPOST(13) + param->cx[20]*FPOST(20) +
      param->cx[27]*FPOST(27) + param->cx[33]*FPOST(33) +
      param->cx[14]*FPOST(14) + param->cx[21]*FPOST(21) +
      param->cx[28]*FPOST(28);

    OE = 
      0.5*(param->cx[29]*param->cx[29]+param->cy[29]*param->cy[29])*FPOST(29)+
      0.5*(param->cx[23]*param->cx[23]+param->cy[23]*param->cy[23])*FPOST(23)+
      0.5*(param->cx[16]*param->cx[16]+param->cy[16]*param->cy[16])*FPOST(16)+
      0.5*(param->cx[ 9]*param->cx[ 9]+param->cy[ 9]*param->cy[ 9])*FPOST( 9)+
      0.5*(param->cx[ 3]*param->cx[ 3]+param->cy[ 3]*param->cy[ 3])*FPOST( 3)+
      0.5*(param->cx[34]*param->cx[34]+param->cy[34]*param->cy[34])*FPOST(34)+
      0.5*(param->cx[30]*param->cx[30]+param->cy[30]*param->cy[30])*FPOST(30)+
      0.5*(param->cx[24]*param->cx[24]+param->cy[24]*param->cy[24])*FPOST(24)+
      0.5*(param->cx[17]*param->cx[17]+param->cy[17]*param->cy[17])*FPOST(17)+
      0.5*(param->cx[10]*param->cx[10]+param->cy[10]*param->cy[10])*FPOST(10)+
      0.5*(param->cx[ 4]*param->cx[ 4]+param->cy[ 4]*param->cy[ 4])*FPOST( 4)+
      0.5*(param->cx[ 0]*param->cx[ 0]+param->cy[ 0]*param->cy[ 0])*FPOST( 0)+
      0.5*(param->cx[35]*param->cx[35]+param->cy[35]*param->cy[35])*FPOST(35)+
      0.5*(param->cx[31]*param->cx[31]+param->cy[31]*param->cy[31])*FPOST(31)+
      0.5*(param->cx[25]*param->cx[25]+param->cy[25]*param->cy[25])*FPOST(25)+
      0.5*(param->cx[11]*param->cx[11]+param->cy[11]*param->cy[11])*FPOST(11)+
      0.5*(param->cx[ 5]*param->cx[ 5]+param->cy[ 5]*param->cy[ 5])*FPOST( 5)+
      0.5*(param->cx[ 1]*param->cx[ 1]+param->cy[ 1]*param->cy[ 1])*FPOST( 1)+
      0.5*(param->cx[ 2]*param->cx[ 2]+param->cy[ 2]*param->cy[ 2])*FPOST( 2)+
      0.5*(param->cx[ 6]*param->cx[ 6]+param->cy[ 6]*param->cy[ 6])*FPOST( 6)+
      0.5*(param->cx[12]*param->cx[12]+param->cy[12]*param->cy[12])*FPOST(12)+
      0.5*(param->cx[19]*param->cx[19]+param->cy[19]*param->cy[19])*FPOST(19)+ 
      0.5*(param->cx[26]*param->cx[26]+param->cy[26]*param->cy[26])*FPOST(26)+
      0.5*(param->cx[32]*param->cx[32]+param->cy[32]*param->cy[32])*FPOST(32)+
      0.5*(param->cx[36]*param->cx[36]+param->cy[36]*param->cy[36])*FPOST(36)+
      0.5*(param->cx[ 7]*param->cx[ 7]+param->cy[ 7]*param->cy[ 7])*FPOST( 7)+
      0.5*(param->cx[13]*param->cx[13]+param->cy[13]*param->cy[13])*FPOST(13)+
      0.5*(param->cx[20]*param->cx[20]+param->cy[20]*param->cy[20])*FPOST(20)+
      0.5*(param->cx[27]*param->cx[27]+param->cy[27]*param->cy[27])*FPOST(27)+
      0.5*(param->cx[33]*param->cx[33]+param->cy[33]*param->cy[33])*FPOST(33)+
      0.5*(param->cx[14]*param->cx[14]+param->cy[14]*param->cy[14])*FPOST(14)+
      0.5*(param->cx[21]*param->cx[21]+param->cy[21]*param->cy[21])*FPOST(21)+
      0.5*(param->cx[28]*param->cx[28]+param->cy[28]*param->cy[28])*FPOST(28);

    a1 = -2.*N*UNIT*UNIT;
    b1 = (29./2.)*(ptildex-Ox)*UNIT*UNIT;
    d1 = 3.*(ptildex-Ox);

    b2 = (Etilde-OE)*(29./2.)*UNIT*UNIT-(281./4.)*N*UNIT*UNIT*UNIT*UNIT;
    d2 = 3.*(Etilde-OE)-(29./2.)*N*UNIT*UNIT;

    E  = -d2/b2;

    px = (b1*d2-d1*b2)/(a1*b2);

    PHI(15) = 1.+param->cx[15]*px+0.5*(param->cx[15]*param->cx[15]+param->cy[15]*param->cy[15])*E;   
    PHI(22) = 1.+param->cx[22]*px+0.5*(param->cx[22]*param->cx[22]+param->cy[22]*param->cy[22])*E;
    PHI( 8) = 1.+param->cx[ 8]*px+0.5*(param->cx[ 8]*param->cx[ 8]+param->cy[ 8]*param->cy[ 8])*E;

    S=PHI(15) + PHI(22) + PHI( 8);

    FPOST(15) = PHI(15)*N/S;  
    FPOST(22) = PHI(22)*N/S;  
    FPOST( 8) = PHI( 8)*N/S;

  ////////////END UPPER LAYER 3////////////////////////
    

   
  } // this closes the for loop over ix

#undef PHI
#undef LOCALPOP
#undef FPRE
#undef FPOST
 
} // this closes the function body
 

////////////////////////////////////////////////////////////////////////////////

#endif // _BC_H

