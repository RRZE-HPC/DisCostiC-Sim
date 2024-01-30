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

#ifndef _COLLIDE_H
#define _COLLIDE_H

////////////////////////////////////////////////////////////////////////

#define PROPAGATECOLLIDE( _nxt, _prv, _startX, _endX, _startY, _endY, _yoff, _queue ) \
  c_kernel ( _nxt, _prv, param, OFF, _startX, _endX, _startY, _endY, _yoff, _queue )


#define COLLIDE( _nxt, _prv, _startX, _endX, _startY, _endY, _yoff, _queue ) \
  c_kernel ( _nxt, _prv, param, OFF0, _startX, _endX, _startY, _endY, _yoff, _queue )

////////////////////////////////////////////////////////////////////////
   
void c_kernel (       data_t * const __restrict__ nxt, 
                      data_t * const __restrict__ prv,
                const par_t  * const __restrict__ param, 
      	        const size_t * const __restrict__ offset, 
      	      	const int startX, const int endX, const int startY, const int endY, const int yoff, queue_t queue ) {

  int ix, iy, p;

  //////////////////////////////////////////////////////////////////////////////

#ifdef SPEC_OPENACC
  #pragma acc kernels present(prv, nxt, param, offset) async(queue)
  #pragma acc loop independent
#elif defined(SPEC_OPENMP)
  #pragma omp parallel for collapse(2) private(ix, iy, p)
#elif defined(USE_TARGET)
  #pragma omp target teams OMP_SCHED_OUTER private(ix, iy, p)
#endif
  for ( ix = startX; ix < endX; ix++) {
#ifdef SPEC_OPENACC
    #pragma acc loop vector independent
#elif defined(USE_TARGET)
  #pragma omp OMP_SCHED_INNER private(iy,p) 
#endif
    for ( iy = startY; iy < endY; iy++) {

      //////////////////////////////////////////////////////////////////////////////

      size_t site_i;
      
      data_t projection2xx,   projection2xy,   projection2yy,
             projection3xxx,  projection3xxy,  projection3xyy,
             projection3yyy,  projection4xxxx, projection4xxxy,
             projection4xxyy, projection4xyyy, projection4yyyy;
      
      data_t projection2xy_t_2,   projection3xxy_t_3,  projection3xyy_t_3,
             projection4xxxy_t_4, projection4xxyy_t_6, projection4xyyy_t_4;
      
      data_t finalProj0,     finalProj1x,    finalProj1y,
             finalProj2xx,   finalProj2xy,   finalProj2yy,
             finalProj3xxx,  finalProj3xxy,  finalProj3xyy,
             finalProj3yyy,  finalProj4xxxx, finalProj4xxxy,
             finalProj4xxyy, finalProj4xyyy, finalProj4yyyy;
      
      data_t finalProj2xy_t_2,   finalProj3xxy_t_3,  finalProj3xyy_t_3,
             finalProj4xxxy_t_4, finalProj4xxyy_t_6, finalProj4xyyy_t_4;
      
      data_t  rhoi, scalar, temprhoi, tempi, theta, forcey, omeganow,
              modc2, modu2, scalar2, theta_1, theta_1_3, theta_1_04, theta_1_2_08;
      
      data_t  tau1, tau2, tau3;
      data_t  tau1i, tau2i, tau3i;
      
      data_t  Hermite0,   Hermite1, Hermite2,   Hermite3, Hermite4;
      data_t  eqHermite2, eqHermite3, eqHermite4, Collisional;
      
      data_t  isBoundary, isBulk;  
      
      data_t  rho, v, u, temp;
      
      data_t popTemp;
      
      //////////////////////////////////////////////////////////////////////////////

      site_i = ix*NY + iy;

      isBoundary = (data_t)(((( (yoff+iy-HY) - 3 ) >> bits ) & 1) | ((( GSIZEY-3-1 - (yoff+iy-HY) ) >> bits ) & 1));

      isBulk = 1.0 - isBoundary;

      //////////////////////////////////////////////////////////////////////////////

      rho = VZERO; v = VZERO; u = VZERO;

      for( p = 0; p < NPOP; p++ ) {
        popTemp = prv[ (p*NX*NY) + site_i + offset[p] ];

      	rho = rho + popTemp;
      	u   = u   + popTemp * param->cx[p];
    	  v   = v   + popTemp * param->cy[p];
      }
      
      //////////////////////////////////////////////////////////////////////////////
      
      rhoi = VONES / rho;
      u    = u * rhoi;
      v    = v * rhoi;
      
      //////////////////////////////////////////////////////////////////////////////
      
      projection2xx   = VZERO;
      projection2xy   = VZERO;
      projection2yy   = VZERO;
      projection3xxx  = VZERO;
      projection3xxy  = VZERO;
      projection3xyy  = VZERO;
      projection3yyy  = VZERO;
      projection4xxxx = VZERO;
      projection4xxxy = VZERO;
      projection4xxyy = VZERO;
      projection4xyyy = VZERO;
      projection4yyyy = VZERO;
      
      temp = VZERO;

      for ( p = 0; p < NPOP; p++ ) {
 
        popTemp = prv[ (p*NX*NY) + site_i + offset[p] ];

      	scalar =  (param->cx[p] - u) * (param->cx[p] - u) +
      		  (param->cy[p] - v) * (param->cy[p] - v);
    	
      	temp = temp + VHALF*scalar*popTemp;

      	projection2xx	+= popTemp * param->H2xx[p];
      	projection2xy	+= popTemp * param->H2xy[p];
      	projection2yy	+= popTemp * param->H2yy[p];

      	projection3xxx  += popTemp * param->H3xxx[p];
      	projection3xxy  += popTemp * param->H3xxy[p];
       	projection3xyy  += popTemp * param->H3xyy[p];
       	projection3yyy  += popTemp * param->H3yyy[p];

    	  projection4xxxx += popTemp * param->H4xxxx[p];
    	  projection4xxxy += popTemp * param->H4xxxy[p];
    	  projection4xxyy += popTemp * param->H4xxyy[p];
    	  projection4xyyy += popTemp * param->H4xyyy[p];
    	  projection4yyyy += popTemp * param->H4yyyy[p];

      }
      
      temp = temp * rhoi;

      //////////////////////////////////////////////////////////////////////////////
      // WARNING: Here we assume that tau's are the same at all boundary sites ...
      
      tempi	 = VONES / temp;
      temprhoi   = rhoi * tempi ;
      
      tau1  = (VHALF + VCONST1 * temprhoi) * isBulk + VTAU1B * isBoundary;
      tau1i = VHALF / tau1;
      
      tau2  = (VHALF + VCONST2 * temprhoi) * isBulk + VTAU2B * isBoundary;
      tau2i = _06 / tau2 ;
      
      tau3  = ( VHALF + VCONST3 ) * isBulk + VTAU3B * isBoundary;
      tau3i = _24 / tau3 ;
      
      //----- replacement for SHIFT ------------------------------------------------
      omeganow = tau1 * rhoi;
      
      forcey = GRAVITY * DELTAT * rho;
      
      u = u + forcex * omeganow;
      v = v + forcey * omeganow;

      //----- replacement for adjustTemp -------------------------------------------
      temp = temp + VHALF*(VONES-tau1)*tau1*(forcex*forcex+forcey*forcey) * rhoi*rhoi;

      //---- replacement for equili ------------------------------------------------
      //---- WARNING: equili results are NOT used -> no replacement...
      
      //////////////////////////////////////////////////////////////////////////////
      // Following variables needed for the final projection.
      
      modu2 = u*u + v*v;
      
      theta = temp;
      theta_1 = theta - VONES;
      theta_1_3 = theta_1 * VTHRE;
      theta_1_04 = theta_1 * _04;
      theta_1_2_08 = theta_1 * theta_1 * _08;

      projection2xy_t_2   = projection2xy   * VTWOS;
      projection3xxy_t_3  = projection3xxy  * VTHRE;
      projection3xyy_t_3  = projection3xyy  * VTHRE;
      projection4xxxy_t_4 = projection4xxxy * VFOUR;
      projection4xxyy_t_6 = projection4xxyy * VSIXS;
      projection4xyyy_t_4 = projection4xyyy * VFOUR;

      for( p = 0; p < NPOP; p++ ) {

      	scalar  = (param->cx[p] * u + param->cy[p]  * v);
       	scalar2 = scalar * scalar;
      	modc2	= (param->cx[p] * param->cx[p] + param->cy[p] * param->cy[p]);
    	
      	Hermite2 = projection2xx     * param->H2xx[p] +
    		           projection2xy_t_2 * param->H2xy[p] +
    		           projection2yy     * param->H2yy[p];
    	
      	Hermite3 = projection3xxx     * param->H3xxx[p] +
    			         projection3xxy_t_3 * param->H3xxy[p] +
    			         projection3xyy_t_3 * param->H3xyy[p] +
    		           projection3yyy     * param->H3yyy[p];
    	
      	Hermite4 = projection4xxxx * param->H4xxxx[p]     +
    			         projection4xxxy_t_4 * param->H4xxxy[p] +
    			         projection4xxyy_t_6 * param->H4xxyy[p] +
    		           projection4xyyy_t_4 * param->H4xyyy[p] +
    		           projection4yyyy * param->H4yyyy[p];
    	
      	eqHermite2 = rho * 
    		     (
    		       (scalar2 - modu2 ) +
    		       theta_1 * (modc2 - VTWOS)
    		     );
    	
      	eqHermite3 = rho *
    		     ( scalar *
    		     ( scalar2 -
    		       VTHRE  * modu2 +
    		       theta_1_3 * (modc2 - VFOUR)
    		     )
    		     );
    	
      	eqHermite4 = VTWO4 * rho *
    		    (
    		    ( scalar2 * scalar2 -
    		      VSIXS  * scalar2 * modu2 +
    		      VTHRE  * modu2 * modu2
    		    ) *
    		    ( _24 ) +
    		    ( theta_1_04 ) *
    		    ( (modc2 - VFOUR) * (scalar2 - modu2) -
    		       VTWOS * scalar2
    		    ) +
    		    ( theta_1_2_08 ) *
    		     ( modc2 * modc2 - VEIGH * modc2 + VEIGH )
    		    );
    	
      	Collisional = -forcex * param->cx[p] -
    		       forcey * param->cy[p] +
    		      (Hermite2 - eqHermite2) * ( tau1i ) +
    		      (Hermite3 - eqHermite3) * ( tau2i ) +
    		      (Hermite4 - eqHermite4) * ( tau3i );
    	
      	prv[ (p*NX*NY) + site_i + offset[p] ] = prv[ (p*NX*NY) + site_i + offset[p] ] - Collisional * param->ww[p];

      }

      //////////////////////////////////////////////////////////////////////////////
      
      finalProj0     = VZERO; finalProj1x    = VZERO; finalProj1y    = VZERO;
      finalProj2xx   = VZERO; finalProj2xy   = VZERO; finalProj2yy   = VZERO;
      finalProj3xxx  = VZERO; finalProj3xxy  = VZERO; finalProj3xyy  = VZERO;
      finalProj3yyy  = VZERO; finalProj4xxxx = VZERO; finalProj4xxxy = VZERO;
      finalProj4xxyy = VZERO; finalProj4xyyy = VZERO; finalProj4yyyy = VZERO;
      
      for( p = 0; p < NPOP; p++ ) {

      	popTemp = prv[ (p*NX*NY) + site_i + offset[p] ];
      
      	finalProj0     += popTemp * param->H0[p];
    	
      	finalProj1x    += popTemp * param->H1x[p];
      	finalProj1y    += popTemp * param->H1y[p];
    	
      	finalProj2xx   += popTemp * param->H2xx[p];
      	finalProj2xy   += popTemp * param->H2xy[p];
      	finalProj2yy   += popTemp * param->H2yy[p];
    	
      	finalProj3xxx  += popTemp * param->H3xxx[p];
      	finalProj3xxy  += popTemp * param->H3xxy[p];
      	finalProj3xyy  += popTemp * param->H3xyy[p];
      	finalProj3yyy  += popTemp * param->H3yyy[p];
      	
      	finalProj4xxxx += popTemp * param->H4xxxx[p];
      	finalProj4xxxy += popTemp * param->H4xxxy[p];
      	finalProj4xxyy += popTemp * param->H4xxyy[p];
      	finalProj4xyyy += popTemp * param->H4xyyy[p];
      	finalProj4yyyy += popTemp * param->H4yyyy[p];

      }
      
      finalProj2xy_t_2 = finalProj2xy * VTWOS;
      finalProj3xxy_t_3 = finalProj3xxy * VTHRE;
      finalProj3xyy_t_3 = finalProj3xyy * VTHRE;
      finalProj4xxxy_t_4 = finalProj4xxxy * VFOUR;
      finalProj4xxyy_t_6 = finalProj4xxyy * VSIXS;
      finalProj4xyyy_t_4 = finalProj4xyyy * VFOUR;

      //////////////////////////////////////////////////////////////////////////////

      for( p = 0; p < NPOP; p++ ) {

      	Hermite0 =  finalProj0 * param->H0[p];
    	
      	Hermite1 =  finalProj1x * param->H1x[p] +
    		            finalProj1y * param->H1y[p];
    	
      	Hermite2 =  finalProj2xx     * param->H2xx[p] +
    		            finalProj2xy_t_2 * param->H2xy[p] +
    		            finalProj2yy     * param->H2yy[p];
    	
      	Hermite3 =  finalProj3xxx     * param->H3xxx[p] +
    		            finalProj3xxy_t_3 * param->H3xxy[p] +
    		            finalProj3xyy_t_3 * param->H3xyy[p] +
    		            finalProj3yyy     * param->H3yyy[p];
    	
      	Hermite4 =  finalProj4xxxx     * param->H4xxxx[p] +
    		            finalProj4xxxy_t_4 * param->H4xxxy[p] +
    		            finalProj4xxyy_t_6 * param->H4xxyy[p] +
    		            finalProj4xyyy_t_4 * param->H4xyyy[p] +
    		            finalProj4yyyy     * param->H4yyyy[p];

      	nxt[(p*NX*NY) + site_i] = ( Hermite0 + Hermite1 + Hermite2 * ( _02 ) +
    		                            Hermite3 * ( _06 )  + Hermite4 * ( _24 )
    	                            ) * param->ww[p];

      }

      
    } // for iy
  } // for ix

}

#endif /* _COLLIDE_H */
