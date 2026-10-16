///    \file
///    HELPER_HPP
///
///    \Copyright © 2026 HPC, FAU Erlangen-Nuremberg. All rights reserved.
///
/// Purpose: Print helpers for pisolver-triad-bench.
///
#ifndef HELPER_HPP
#define HELPER_HPP

#pragma once

#include <vector>

	/*
	* \brief return the print of the 1 dimensional vector
	* \return print 1D vector
	*/
	#define print_1D(vec){                                          \
        std::cout << " {";                                        \
    	for ( int i = 0; i < vec.size(); i++ )												\
        	std::cout <<  (i ? " x " : "") << vec[i] ;										\
    	std::cout << " }" << std::endl;														\
	}

	/*
	* \brief return the print of the 2 dimensional vector
	* \return print 2D vector
	*/
	#define print_2D(vec){																    \
    	for ( int i = 0; i < vec[0].size(); i++ ){											\
    		for ( int j = 0; j < vec[i].size(); j++ ){										\
				std::cout << "[" << j << "][" << i << "]=" <<  vec[j][i] << std::endl;		\
        	}																				\
    	}																					\
	}

	/*
 	 * \brief Return the print of the 3 dimensional vector
 	 * \return print 3D vector
 	*/
	#define print_3D(vec){                                                                    \
    	for ( int i = 0; i < vec[0].size(); i++ ){                                            \
        	for ( int j = 0; j < vec[i].size(); j++ ){                                        \
            	for ( int k = 0; j < vec[j].size(); j++ ){                                    \
                	std::cout << "[" << k << "][" << j << "][" << i << "]=" <<  vec[k][j][i] << std::endl;    \
            	}                                                                             \
        	}                                                                                 \
    	}                                                                                     \
	}


	/**
 	* \brief the error codes returned by MPI are left entirely to the implementation (with the exception of MPI_SUCCESS). This is done to allow an implementation to provide as much information as possible in the error code (for use with MPI_ERROR_STRING).
 	* \return a string for a given error code with file name and line number
 	*/
	#define mpiErrorCheck(errorCheck){														\
    	if ( errorCheck != MPI_SUCCESS ) {													\
        	int len;																		\
        	char estring[MPI_MAX_ERROR_STRING];												\
        	MPI_Error_string( errorCheck , estring, &len);									\
        	std::stringstream oss;															\
        	oss << "MPI error on file " << __FILE__ << " line " << __LINE__ << std::endl;	\
        	oss << estring;																	\
        	throw std::runtime_error(  oss.str() );											\
    	}																					\
	}
#endif 	//!< end HELPER_HPP
