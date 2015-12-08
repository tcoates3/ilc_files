########################################################
# cmake file for building Marlin example Package
# @author Jan Engels, Desy IT
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
########################################################


# project name
PROJECT( SignalSeparator )


# project version
SET( ${PROJECT_NAME}_VERSION_MAJOR 0 )
SET( ${PROJECT_NAME}_VERSION_MINOR 1 )
SET( ${PROJECT_NAME}_VERSION_PATCH 0 )



### DEPENDENCIES ############################################################

FIND_PACKAGE( ILCUTIL REQUIRED COMPONENTS ILCSOFT_CMAKE_MODULES )

# load default settings from ILCSOFT_CMAKE_MODULES
INCLUDE( ilcsoft_default_settings )


FIND_PACKAGE( Marlin 1.0 REQUIRED ) # minimum required Marlin version
INCLUDE_DIRECTORIES( ${Marlin_INCLUDE_DIRS} )
LINK_LIBRARIES( ${Marlin_LIBRARIES} )
ADD_DEFINITIONS( ${Marlin_DEFINITIONS} )

# optional package
FIND_PACKAGE( AIDA )
IF( AIDA_FOUND )
    INCLUDE_DIRECTORIES( ${AIDA_INCLUDE_DIRS} )
    LINK_LIBRARIES( ${AIDA_LIBRARIES} )
    ADD_DEFINITIONS( "-DMARLIN_USE_AIDA" )
    MESSAGE( STATUS "AIDA -- found" )
ELSE()
    MESSAGE( STATUS "AIDA -- not found" )
ENDIF()

#export ROOTCFLAGS="/cvmfs/ilc.desy.de/sw/x86_64_gcc44_sl6/v01-17-07/root/5.34.30/include"
#export ROOTLIBS="/cvmfs/ilc.desy.de/sw/x86_64_gcc44_sl6/v01-17-07/root/5.34.30/lib"

FIND_PACKAGE( ROOT REQUIRED )
INCLUDE_DIRECTORIES (/cvmfs/ilc.desy.de/sw/x86_64_gcc44_sl6/v01-17-07/root/5.34.30/include )
ADD_DEFINITIONS( "-pthread -m64" )
ADD_DEFINITIONS( "-lCore -lCint -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lm -ldl -rdynamic" )
LINK_LIBRARIES ( /cvmfs/ilc.desy.de/sw/x86_64_gcc44_sl6/v01-17-07/root/5.34.30/lib )


### DOCUMENTATION ###########################################################

OPTION( INSTALL_DOC "Set to OFF to skip build/install Documentation" OFF )

IF( INSTALL_DOC AND EXISTS "${PROJECT_SOURCE_DIR}/doc/CMakeLists.txt" )
    ADD_SUBDIRECTORY( ./doc )
ENDIF()



### LIBRARY #################################################################

# definitions to pass to the compiler
ADD_DEFINITIONS( "-Wall -ansi -pedantic" )
ADD_DEFINITIONS( "-Wno-long-long" )

# include directories
INCLUDE_DIRECTORIES( ./include )
#INSTALL_DIRECTORY( ./include DESTINATION . FILES_MATCHING PATTERN "*.h" )

# add library
AUX_SOURCE_DIRECTORY( ./src library_sources )
ADD_SHARED_LIBRARY( ${PROJECT_NAME} ${library_sources} )
INSTALL_SHARED_LIBRARY( ${PROJECT_NAME} DESTINATION lib )

# display some variables and write them to cache
DISPLAY_STD_VARIABLES()

