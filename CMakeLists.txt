cmake_minimum_required (VERSION 3.23.1)
project (DisCostiC)

#include(GNUInstallDirs)

set (Test on)
set (DisCostiC_VERSION_IDENTIFIER)
set (DisCostiC_VERSION "0.1.0.0")
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_BUILD_TYPE Release)
set (CMAKE_C_COMPILER icx)
set (CMAKE_CXX_COMPILER icpx)


message (STATUS "===== FIND LIBRARIES =====")

find_package(MPI REQUIRED)
if(MPI_FOUND)
  message (STATUS "MPI package is found ${MPI_INCLUDE_PATH}")
  include_directories(${MPI_INCLUDE_PATH})
  set(LIBS  ${MPI_LIBRARIES})
else()
  message (STATUS "MPI package is NOT found")
endif()

find_package(Python COMPONENTS Interpreter REQUIRED)

# detect virtualenv and set Pip args accordingly
if(DEFINED ENV{VIRTUAL_ENV} OR DEFINED ENV{CONDA_PREFIX})
  set(_pip_args)
else()
  set(_pip_args "--user")
endif()

execute_process(
  COMMAND python -c "from distutils import sysconfig as sc;print(sc.get_python_lib(prefix='', plat_specific=True))"
  OUTPUT_VARIABLE PYTHON_SITE)


set(PYTHON_SITE "$HOME/.local/${PYTHON_SITE}" )
message(STATUS "PYTHON_SITE : ${PYTHON_SITE}" )

find_program(KERNCRAFT kerncraft)
if(NOT KERNCRAFT)
	message(STATUS "KERNCRAFT not found. Trying to install !")
	execute_process(COMMAND ${Python_EXECUTABLE} -m pip install ${_pip_args} kernraft)
else()
	message(STATUS "KERNCRAFT found at : ${KERNCRAFT}")
endif()

execute_process(
		COMMAND pip show kerncraft
		RESULT_VARIABLE KERNCRAFT_EXIT_CODE
		OUTPUT_QUIET
)

if (NOT ${KERNCRAFT_EXIT_CODE} EQUAL 0)
	message("The python module kerncraft is not installed. Attempting to install them." )
	execute_process(
		COMMAND pip install ${_pip_args} kerncraft
		RESULT_VARIABLE SIX_INSTALL_CODE)
	if (NOT ${KERMCRAFT_INSTALL_CODE} EQUAL 0)
		message(FATAL_ERROR "The python module kerncraft installation failed" )
	else()
		message(STATUS "kerncraft python package installed")
	endif()

endif()

set (DisCostiC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
message (STATUS "DisCostiC: DisCostiC_DIR is ${DisCostiC_DIR}")
set (DisCostiC_SRC_DIR ${DisCostiC_DIR}/src)
set (DisCostiC_INC_DIR ${DisCostiC_DIR}/include)
include_directories("${DisCostiC_DIR}")
include_directories("${DisCostiC_INC_DIR}")
include_directories("${DisCostiC_DIR}/nodelevel/include")
include_directories("${DisCostiC_DIR}/visualization")

message (STATUS "===== LISTING INCLUDE DIRECTORIES =====")

get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
foreach(dir ${dirs})
  message(STATUS "dir='${dir}'")
endforeach()




set (DisCostiC_SOURCE 	
		${DisCostiC_DIR}/nodelevel/src/NodeLvlScg.cpp
		${DisCostiC_DIR}/src/DisCosTiC.cpp
		)



set (DisCostiC_HEADERS
	${DisCostiC_INC_DIR}/AST.hpp
		${DisCostiC_INC_DIR}/CompModel.hpp
		${DisCostiC_INC_DIR}/DataStruct.hpp
		${DisCostiC_INC_DIR}/DataType.hpp
		${DisCostiC_INC_DIR}/Grid.hpp
		${DisCostiC_INC_DIR}/GridInit.hpp
		${DisCostiC_DIR}/visualization/TimeRankOP.hpp
		${DisCostiC_INC_DIR}/enum.hpp
		${DisCostiC_DIR}/visualization/json.hpp
		${DisCostiC_INC_DIR}/macro.hpp
		${DisCostiC_DIR}/nodelevel/include/NodeLvlScg.hpp
		${DisCostiC_DIR}/nodelevel/include/NodeModel.hpp
		)


message (STATUS "===== INSTALL TARGET =====")
set(INCLUDE_INSTALL_DIR include)
install ( FILES ${DisCostiC_DIR}/visualization/json.hpp DESTINATION ${INCLUDE_INSTALL_DIR} )
install ( FILES ${DisCostiC_DIR}/visualization/TimeRankOP.hpp DESTINATION ${INCLUDE_INSTALL_DIR} )
install ( DIRECTORY ${DisCostiC_INC_DIR} DESTINATION ${INCLUDE_INSTALL_DIR} )
install ( FILES ${DisCostiC_DIR}/nodelevel/include/NodeLvlScg.hpp DESTINATION ${INCLUDE_INSTALL_DIR} )
install ( FILES ${DisCostiC_DIR}/nodelevel/include/NodeModel.hpp DESTINATION ${INCLUDE_INSTALL_DIR} )


add_executable(discostic ${DisCostiC_SOURCE} ${DisCostiC_HEADERS})
target_compile_options(discostic PRIVATE -O3 -w )
target_link_libraries(discostic MPI::MPI_CXX)

set_target_properties(discostic
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

message (STATUS "===== UNINSTALL TARGET =====")
configure_file(
				    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
						 "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
	    				IMMEDIATE @ONLY)

add_custom_target(uninstall
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND make clear
		COMMAND make clean
		COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
		COMMAND rm -rf CMakeFiles
		COMMAND rm -rf DisCosTiC
		COMMAND rm -rf CMakeCache.txt
		COMMAND rm -rf CTestTestfile.cmake
		COMMAND rm -rf install_manifest.txt
		COMMAND rm -rf cmake_install.cmake
		COMMAND rm -rf cmake_uninstall.cmake
		COMMAND rm -f otf2.* config.log config.status config.summary
		COMMAND rm -f user_provided_configure_args otf2-mini.*
		COMMAND rm -rf Makefile
		)


add_custom_target(clear
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND make clean
		COMMAND bash -c [[rm -f discostic.* *.o *.csv *.out *.viz *.dms Exec.* *.stf *.prot]])

add_custom_target(trace_MPI
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND mpiicc -c -O3 -trace nodelevel/src/NodeLvlScg.cpp -o CMakeFiles/discostic.dir/nodelevel/src/NodeLvlScg.cpp.o
		COMMAND mpiicc -c -O3 -trace src/DisCosTiC.cpp -o CMakeFiles/discostic.dir/src/DisCosTiC.cpp.o
		COMMAND mpiicc -trace CMakeFiles/discostic.dir/src/DisCosTiC.cpp.o CMakeFiles/discostic.dir/nodelevel/src/NodeLvlScg.cpp.o -o discostic)


add_custom_target(trace_all
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND mpiicc -c -O3 -trace -tcollect nodelevel/src/NodeLvlScg.cpp -o CMakeFiles/discostic.dir/nodelevel/src/NodeLvlScg.cpp.o
		COMMAND mpiicc -c -O3 -trace -tcollect src/DisCosTiC.cpp -o CMakeFiles/discostic.dir/src/DisCosTiC.cpp.o
		COMMAND mpiicc -trace -tcollect CMakeFiles/discostic.dir/src/DisCosTiC.cpp.o CMakeFiles/discostic.dir/nodelevel/src/NodeLvlScg.cpp.o -o discostic)

add_custom_target(otf2
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		COMMAND python visualization/chrome2otf2.py -i DisCosTiC.dms -o DisCosTiC -c)	

execute_process(
	COMMAND pip show six
	RESULT_VARIABLE SIX_EXIT_CODE
	OUTPUT_QUIET
)

if (NOT ${SIX_EXIT_CODE} EQUAL 0)
	message("The python module six is not installed. Attempting to install them." )
	execute_process(
		COMMAND pip install ${_pip_args} six
		RESULT_VARIABLE SIX_INSTALL_CODE)
	if (NOT ${SIX_EXITSIX_INSTALL_CODE_CODE} EQUAL 0)
		message(FATAL_ERROR "The python module six installation failed" )
	else()
		message(STATUS "six package installed")
	endif()
endif()

execute_process(
		COMMAND pip show otf2
		RESULT_VARIABLE OTF2_EXIT_CODE
		OUTPUT_QUIET
)

if (NOT ${OTF2_EXIT_CODE} EQUAL 0)
	message("The python module otf2 is not installed. Attempting to install them." )
	execute_process(
		COMMAND pip install ${_pip_args} otf2
		RESULT_VARIABLE SIX_INSTALL_CODE)
	if (NOT ${OTF2_INSTALL_CODE} EQUAL 0)
		message(FATAL_ERROR "The python module otf2 installation failed" )
	else()
		message(STATUS "otf2 package installed")
	endif()

endif()

# cmake -DCMAKE_INSTALL_PREFIX=~/.local . && make all install
