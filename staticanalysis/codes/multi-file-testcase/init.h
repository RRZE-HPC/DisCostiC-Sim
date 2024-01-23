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

#ifndef _INIT_H
#define _INIT_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"

#ifdef SPEC_RAND
#include "specrand/specrand.h"
#endif

//////////////////////////////////////////////////////////////////////
// Prototypes.

void dellar ( par_t * param );
void inithydro ( uvrt_t * uvrt );
void equili ( pop_t * cf, uvrt_t * uvrt, par_t * param );
void projection ( pop_t * cf, par_t * param );

///////////////////////////////////////////////////////////////////////////////
// Init function.

void init ( par_t * param, pop_t * cf, long int seed ) {
  uvrt_t *uvrt;
  int ii;

  // MPI processes at given X use the same seed
#ifdef SPEC_RAND
  spec_srand(seed);
#else
  srand48((long)seed+mpi_rank_x);
#endif   

  // Init is made on temporary structures.
  uvrt = (uvrt_t *) malloc( NX*NY*sizeof(uvrt_t) );
  
  dellar ( param );
  
  inithydro ( uvrt );
    
  equili ( cf, uvrt, param );
    
  projection ( cf, param );
  
  // Lattice top and bottom borders are copied to halos.
  for(ii = HX; ii < LSIZEX + HX; ii++){
    memcpy(cf + ii*NY + HY-3    , cf + ii*NY + HY+LSIZEY-3, 3*sizeof(pop_t));
    memcpy(cf + ii*NY + HY+LSIZEY, cf + ii*NY + HY        , 3*sizeof(pop_t));
  }

  // update left and right halos
  memcpy(cf + (HX-3)*NY,     cf + (HX+LSIZEX-3)*NY, 3*NY*sizeof(pop_t));
  memcpy(cf + (HX+LSIZEX)*NY, cf + (HX*NY)        , 3*NY*sizeof(pop_t));
  
  free(uvrt);
}

///////////////////////////////////////////////////////////////////////////////
// WARNING: description missing!
void dellar ( par_t * param ) {
  
  int k;
  
  data_t deltaxx, deltayy, deltaxy;
  
  /////////////////////WEIGHTS/////////////////////////////
  
  param->ww[11]=0.10730609154221900241246; 	//262./1785.; //w1
  param->ww[19]=0.10730609154221900241246; 	//262./1785.;
  param->ww[25]=0.10730609154221900241246; 	//262./1785.;
  param->ww[17]=0.10730609154221900241246; 	//262./1785.;
  
  param->ww[12]=0.05766785988879488203006; 	//93./1190.; //w2
  param->ww[26]=0.05766785988879488203006; 	//93./1190.;
  param->ww[24]=0.05766785988879488203006; 	//93./1190.;
  param->ww[10]=0.05766785988879488203006; 	//93./1190.;
  
  param->ww[ 5]=0.01420821615845075026469; 	//7./340.; //w4
  param->ww[20]=0.01420821615845075026469; 	//7./340.;
  param->ww[31]=0.01420821615845075026469; 	//7./340.;
  param->ww[16]=0.01420821615845075026469; 	//7./340.;
  
  param->ww[ 7]=0.00101193759267357547541; 	//9./9520.; //w8
  param->ww[33]=0.00101193759267357547541; 	//9./9520.;
  param->ww[29]=0.00101193759267357547541; 	//9./9520.;
  param->ww[ 3]=0.00101193759267357547541; 	//9./9520.;
  
  param->ww[ 1]=0.00024530102775771734547; 	//2./5355.;//w9
  param->ww[21]=0.00024530102775771734547; 	//2./5355.;
  param->ww[35]=0.00024530102775771734547; 	//2./5355.;
  param->ww[15]=0.00024530102775771734547; 	//2./5355.;
  
  param->ww[ 2]=0.00028341425299419821740; 	//1./7140.; //w10
  param->ww[14]=0.00028341425299419821740; 	//1./7140.;
  param->ww[28]=0.00028341425299419821740; 	//1./7140.;
  param->ww[36]=0.00028341425299419821740; 	//1./7140.;
  param->ww[34]=0.00028341425299419821740; 	//1./7140.;
  param->ww[22]=0.00028341425299419821740; 	//1./7140.;
  param->ww[ 8]=0.00028341425299419821740; 	//1./7140.;
  param->ww[ 0]=0.00028341425299419821740; 	//1./7140.;
  
  param->ww[ 6]=0.00535304900051377523273; 	//6./595.; //w5
  param->ww[13]=0.00535304900051377523273; 	//6./595.;
  param->ww[27]=0.00535304900051377523273; 	//6./595.;
  param->ww[32]=0.00535304900051377523273; 	//6./595.;
  param->ww[30]=0.00535304900051377523273; 	//6./595.;
  param->ww[23]=0.00535304900051377523273; 	//6./595.;
  param->ww[ 9]=0.00535304900051377523273; 	//6./595.;
  param->ww[ 4]=0.00535304900051377523273; 	//6./595.;
  
  param->ww[18]=0.23315066913235250228650; 	//1.-4.*ww[ 1]-4.*ww[ 5]-4.*ww[ 9]-4.*ww[13]-4.*ww[17]-8.*ww[21]-8.*ww[22];
  
  /////////////////////LATTICE SPEEDS/////////////////////
  
  param->cx[18] =  0.             ; param->cy[18] =  0.	           ;
  param->cx[11] =  UNIT           ; param->cy[11] =  0.	           ;
  param->cx[19] =  0.             ; param->cy[19] =  UNIT	         ;
  param->cx[25] = -param->cx[11]  ; param->cy[25] = -param->cy[11] ;
  param->cx[17] = -param->cx[19]  ; param->cy[17] = -param->cy[19] ; // for(i=3;i<=4;i++){ params->cx[i]=-params->cx[i-2]; params->cy[i]=-params->cy[i-2];}
  param->cx[12] =  UNIT           ; param->cy[12] =  UNIT	         ;
  param->cx[26] = -UNIT           ; param->cy[26] =  UNIT	         ;
  param->cx[24] = -param->cx[12]  ; param->cy[24] = -param->cy[12] ;
  param->cx[10] = -param->cx[26]  ; param->cy[10] = -param->cy[26] ; // for(i=7;i<=8;i++){ params->cx[i]=-params->cx[i-2]; params->cy[i]=-params.cy[i-2];}
  param->cx[ 5] =  2.*UNIT        ; param->cy[ 5] =  0.0	         ;
  param->cx[20] =  0.0            ; param->cy[20] =  2.*UNIT       ;
  param->cx[31] = -2.*UNIT        ; param->cy[31] =  0.0	         ;
  param->cx[16] =  0.0            ; param->cy[16] = -2.*UNIT       ;
  param->cx[ 7] =  2.*UNIT        ; param->cy[ 7] =  2.*UNIT       ;
  param->cx[33] = -2.*UNIT        ; param->cy[33] =  2.*UNIT       ;
  param->cx[29] = -2.*UNIT        ; param->cy[29] = -2.*UNIT       ;
  param->cx[ 3] =  2.*UNIT        ; param->cy[ 3] = -2.*UNIT       ;
  param->cx[ 1] =  3.*UNIT        ; param->cy[ 1] =  0.0	         ;
  param->cx[21] =  0.0            ; param->cy[21] =  3.*UNIT       ;
  param->cx[35] = -3.*UNIT        ; param->cy[35] =  0.0	         ;
  param->cx[15] =  0.0            ; param->cy[15] = -3.*UNIT	     ;
  param->cx[ 2] =  3.*UNIT        ; param->cy[ 2] =  1.*UNIT	     ;
  param->cx[ 6] =  2.*UNIT        ; param->cy[ 6] =  1.*UNIT	     ;
  param->cx[13] =  1.*UNIT        ; param->cy[13] =  2.*UNIT	     ;
  param->cx[14] =  1.*UNIT        ; param->cy[14] =  3.*UNIT	     ;
  param->cx[28] = -1.*UNIT        ; param->cy[28] =  3.*UNIT	     ;
  param->cx[27] = -1.*UNIT        ; param->cy[27] =  2.*UNIT	     ;
  param->cx[32] = -2.*UNIT        ; param->cy[32] =  1.*UNIT	     ;
  param->cx[36] = -3.*UNIT        ; param->cy[36] =  1.*UNIT	     ;
  param->cx[34] = -3.*UNIT        ; param->cy[34] = -1.*UNIT	     ;
  param->cx[30] = -2.*UNIT        ; param->cy[30] = -1.*UNIT	     ;
  param->cx[23] = -1.*UNIT        ; param->cy[23] = -2.*UNIT	     ;
  param->cx[22] = -1.*UNIT        ; param->cy[22] = -3.*UNIT	     ;
  param->cx[ 8] =  1.*UNIT        ; param->cy[ 8] = -3.*UNIT	     ;
  param->cx[ 9] =  1.*UNIT        ; param->cy[ 9] = -2.*UNIT	     ;
  param->cx[ 4] =  2.*UNIT        ; param->cy[ 4] = -1.*UNIT	     ;
  param->cx[ 0] =  3.*UNIT        ; param->cy[ 0] = -1.*UNIT	     ;
  
  deltaxx = 1.0;
  deltayy = 1.0;
  deltaxy = 0.0;
  
  for ( k = 0 ; k < NPOP ; k++ ) {
    
    param->H0[k] = 1.0;
    
    param->H1x[k] = param->cx[k];
    param->H1y[k] = param->cy[k];
    
    param->H2xx[k] = param->cx[k]*param->cx[k]-deltaxx;
    param->H2xy[k] = param->cx[k]*param->cy[k]-deltaxy;
    param->H2yy[k] = param->cy[k]*param->cy[k]-deltayy;
    
    param->H3xxx[k] =  param->cx[k]*param->cx[k]*param->cx[k] -
            	 	       deltaxx*param->cx[k] - deltaxx*param->cx[k] - deltaxx*param->cx[k];
    
    param->H3xxy[k] =  param->cx[k]*param->cx[k]*param->cy[k] - 
	 	                   deltaxx*param->cy[k] - deltaxy*param->cx[k] - deltaxy*param->cx[k];
    
    param->H3xyy[k] =  param->cx[k]*param->cy[k]*param->cy[k] - 
            	 	       deltaxy*param->cy[k] - deltaxy*param->cy[k] - deltayy*param->cx[k];
    
    param->H3yyy[k] =  param->cy[k]*param->cy[k]*param->cy[k] - 
	 	                   deltayy*param->cy[k] - deltayy*param->cy[k] - deltayy*param->cy[k];
    
    param->H4xxxx[k] = param->cx[k]*param->cx[k]*param->cx[k]*param->cx[k] - 
	 	                   deltaxx*param->H2xx[k] - deltaxx*param->H2xx[k] - 
	 	                   deltaxx*param->H2xx[k] - deltaxx*param->H2xx[k] - 
            	 	       deltaxx*param->H2xx[k] - deltaxx*param->H2xx[k] - 
	 	                   deltaxx*deltaxx - deltaxx*deltaxx - deltaxx*deltaxx;
    
    param->H4xxxy[k] = param->cx[k]*param->cx[k]*param->cx[k]*param->cy[k] - 
	 	                   deltaxx*param->H2xy[k] - deltaxx*param->H2xy[k] - 
	 	                   deltaxy*param->H2xx[k] - deltaxx*param->H2xy[k] - 
            	 	       deltaxy*param->H2xx[k] - deltaxy*param->H2xx[k] - 
            	 	       deltaxx*deltaxy - deltaxx*deltaxy - deltaxy*deltaxx;
    
    param->H4xxyy[k] = param->cx[k]*param->cx[k]*param->cy[k]*param->cy[k] - 
	 	                   deltaxx*param->H2yy[k] - deltaxy*param->H2xy[k] - 
            	 	       deltaxy*param->H2xy[k] - deltaxy*param->H2xy[k] - 
            	 	       deltaxy*param->H2xy[k] - deltayy*param->H2xx[k] - 
            	 	       deltaxx*deltayy - deltaxy*deltaxy - deltaxy*deltaxy;
    
    param->H4xyyy[k] = param->cx[k]*param->cy[k]*param->cy[k]*param->cy[k] - 
	           	      	deltaxy*param->H2yy[k] - deltaxy*param->H2yy[k] - 
	          	      	deltaxy*param->H2yy[k] - deltayy*param->H2xy[k] - 
	      	          	deltayy*param->H2xy[k] - deltayy*param->H2xy[k] - 
	      	          	deltaxy*deltayy - deltaxy*deltayy - deltaxy*deltayy;
    
    param->H4yyyy[k] = param->cy[k]*param->cy[k]*param->cy[k]*param->cy[k] - 
                       deltayy*param->H2yy[k] - deltayy*param->H2yy[k] - 
                       deltayy*param->H2yy[k] - deltayy*param->H2yy[k] - 
                       deltayy*param->H2yy[k] - deltayy*param->H2yy[k] - 
                       deltayy*deltayy - deltayy*deltayy - deltayy*deltayy;
  }
  
} // Routine dellar end.


////////////////////////////////////////////////////////////////////////////////
// Routine inithydro start.

void inithydro ( uvrt_t * uvrt ) {

  //int x, y, idx1, kk;
  //data_t di, dj, y_cfase, rho, rho_prv=0, temp, temp_prv=0;

  int x;
#ifdef SPEC_OPENMP
  #pragma omp parallel for 
#endif
  for ( x = HX; x < (HX+LSIZEX) ; x++ ) {

    int y, idx1, kk, yoff;

    data_t di, dj, y_cfase, rho, rho_prv=0, temp, temp_prv=0;
    
    di = (data_t)((x-HX) + (LSIZEX*mpi_rank_x));

    y_cfase = (data_t)GSIZEY/2.;

    for ( kk = K_MIN ; kk <= K_MAX ; kk++ ) {
#ifdef SPEC_RAND
      y_cfase += WIDTH/sqrt((data_t)K_MAX)*cos(di*kk*2.*PI/(data_t)GSIZEX + 2.*PI*(data_t)spec_rand());
#else
      y_cfase += WIDTH/sqrt((data_t)K_MAX)*cos(di*kk*2.*PI/(data_t)GSIZEX + 2.*PI*(data_t)drand48());
#endif
    }

    yoff = mpi_rank_y * (GSIZEY/n_mpi_y);

    for ( y = 0; y < (yoff+LSIZEY); y++ ) {

      dj = (data_t)(y);

      temp = TEMPWALLDOWN + (TEMPWALLUP-TEMPWALLDOWN) * (tanh((dj-y_cfase)/EPS)+1.0) * 0.5;

      if ( y == 0 ) {
      	rho =  exp (( -GRAVITY*(data_t)GSIZEY/2. )/TEMPWALLDOWN );
      } else  {
      	rho = ( rho_prv / temp ) * ( temp_prv + GRAVITY );
      }

      if ( y >= yoff ) {
     
      	idx1 = (x*NY) + HY + (y-yoff);

      	uvrt[idx1].temp = temp;
      	uvrt[idx1].u    = 0.0;
      	uvrt[idx1].v    = 0.0;
      	uvrt[idx1].rho  = rho;

      }

      temp_prv = temp;
      rho_prv = rho;

    } // End for y.

  } // End for x.

}

///////////////////////////////////////////////////////////////////////////////
// Routine equili start.

void equili ( pop_t * cf, uvrt_t * uvrt, par_t * param ) {
  
  //data_t scalar,modu2,theta,modc2,rho;
  //int i,j,k,idx0;
  
  int i;

#ifdef SPEC_OPENMP
  #pragma omp parallel for 
#endif
  for ( i = HX; i < (HX+LSIZEX); i++ ) {

    data_t scalar,modu2,theta,modc2,rho;
    int j,k,idx0;

    for ( j = HY; j < (HY+LSIZEY) ; j++ ) {
      
      idx0=(i*NY)+j;
      
      theta = uvrt[idx0].temp;
      rho   = uvrt[idx0].rho;
      
      modu2=uvrt[idx0].u*uvrt[idx0].u+uvrt[idx0].v*uvrt[idx0].v;
      
      for ( k = 0 ; k < NPOP ; k++ ) {
        
        scalar=(param->cx[k]*uvrt[idx0].u+param->cy[k]*uvrt[idx0].v);  
        
        modc2=(param->cx[k]*param->cx[k]+param->cy[k]*param->cy[k]);
        
        cf[idx0].p[k] = param->ww[k]*rho*(1.0+scalar+0.5*(scalar*scalar-modu2)
            +(theta-1.)*(modc2-2.)/2+scalar*(scalar*scalar-3.*modu2+3.*(theta-1.0)*(modc2-4.))/6.
            +(scalar*scalar*scalar*scalar-6.*scalar*scalar*modu2+3.*modu2*modu2)/24.
            +((theta-1.0)/4.)*((modc2-4.)*(scalar*scalar-modu2)-2.*scalar*scalar)
            +((theta-1.0)*(theta-1.0)/8.)*(modc2*modc2-8.*modc2+8.));
        
      } // End for k

    } // End for j
  } // End for i

} // Routine equili end.

///////////////////////////////////////////////////////////////////////////////
// Routine projection start.

void projection ( pop_t * cf, par_t * param ) {

/*  
  data_t projection0;
  data_t projection1x,projection1y;
  data_t projection2xx,projection2xy,projection2yy;
  data_t projection3xxx,projection3xxy,projection3xyy,projection3yyy;
  data_t projection4xxxx,projection4xxxy,projection4xxyy,projection4xyyy,projection4yyyy;
  data_t Hermite0,Hermite1,Hermite2,Hermite3,Hermite4;  
  int i,j,k,idx0;
*/

  int i;

#ifdef SPEC_OPENMP
  #pragma omp parallel for 
#endif 
  for ( i = HX; i < (HX+LSIZEX); i++ ) {

    data_t projection0;
    data_t projection1x,projection1y;
    data_t projection2xx,projection2xy,projection2yy;
    data_t projection3xxx,projection3xxy,projection3xyy,projection3yyy;
    data_t projection4xxxx,projection4xxxy,projection4xxyy,projection4xyyy,projection4yyyy;
    data_t Hermite0,Hermite1,Hermite2,Hermite3,Hermite4;  
    int j,k,idx0;

    for ( j = HY; j < (HY+LSIZEY); j++ ) { 

      idx0=(i*NY)+j;
      
      projection0=0.0;
      
      projection1x=0.0;
      projection1y=0.0;
      
      projection2xx=0.0;
      projection2xy=0.0;
      projection2yy=0.0;
      
      projection3xxx=0.0;
      projection3xxy=0.0;
      projection3xyy=0.0;
      projection3yyy=0.0;
      
      projection4xxxx=0.0;
      projection4xxxy=0.0;
      projection4xxyy=0.0;
      projection4xyyy=0.0;
      projection4yyyy=0.0;
      
      for ( k = 0 ; k < NPOP ; k++ ) {
        
        projection0+=param->H0[k]*cf[idx0].p[k];
        
        projection1x+=param->H1x[k]*cf[idx0].p[k];
        projection1y+=param->H1y[k]*cf[idx0].p[k];
        
        projection2xx+=param->H2xx[k]*cf[idx0].p[k];
        projection2xy+=param->H2xy[k]*cf[idx0].p[k];
        projection2yy+=param->H2yy[k]*cf[idx0].p[k];
        
        projection3xxx+=param->H3xxx[k]*cf[idx0].p[k];
        projection3xxy+=param->H3xxy[k]*cf[idx0].p[k];
        projection3xyy+=param->H3xyy[k]*cf[idx0].p[k];
        projection3yyy+=param->H3yyy[k]*cf[idx0].p[k];
        
        
        projection4xxxx+=param->H4xxxx[k]*cf[idx0].p[k];
        projection4xxxy+=param->H4xxxy[k]*cf[idx0].p[k];
        projection4xxyy+=param->H4xxyy[k]*cf[idx0].p[k];
        projection4xyyy+=param->H4xyyy[k]*cf[idx0].p[k];
        projection4yyyy+=param->H4yyyy[k]*cf[idx0].p[k];
        
        
      } // End for k.
      
      for ( k = 0 ; k < NPOP ; k++ ) {
        
        Hermite0=param->H0[k]*projection0;
        
        Hermite1=param->H1x[k]*projection1x+param->H1y[k]*projection1y;
        
        Hermite2=param->H2xx[k]*projection2xx+2.*param->H2xy[k]*projection2xy+param->H2yy[k]*projection2yy;
        Hermite3=param->H3xxx[k]*projection3xxx+3.*param->H3xxy[k]*projection3xxy+3.*param->H3xyy[k]*projection3xyy+param->H3yyy[k]*projection3yyy;
        
        Hermite4=param->H4xxxx[k]*projection4xxxx+4.*param->H4xxxy[k]*projection4xxxy+6.*param->H4xxyy[k]*projection4xxyy+4.*param->H4xyyy[k]*projection4xyyy+param->H4yyyy[k]*projection4yyyy;
        
        cf[idx0].p[k]=param->ww[k]*(Hermite0+Hermite1+Hermite2/2.+Hermite3/6.+Hermite4/24.);
        
      } // End for k.
      
    } // End for j.
  } // End for i.
  
  
} // Routine projection end.

///////////////////////////////////////////////////////////////////////////////

#endif

