project(ssl-vision)
cmake_minimum_required(VERSION 2.6)

#defines
add_definitions(-D UNIX)
add_definitions(-D LINUX)
add_definitions(-D _GNU_SOURCE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#basler
include_directories(
	$ENV{PYLON_ROOT}/include 
	$ENV{PYLON_GENICAM_ROOT}/library/CPP/include
)
link_directories(
	$ENV{PYLON_ROOT}/lib
	$ENV{PYLON_GENICAM_ROOT}/library/CPP/lib/Linux32_i86
	$ENV{PYLON_GENICAM_ROOT}/bin/Linux32_i86
	$ENV{PYLON_GENICAM_ROOT}/bin/Linux32_i86/GenApi/Generic
)

set(PYLON_INCLUDE
    $ENV{PYLON_ROOT}/include/ 
    $ENV{GENICAM_ROOT_V2_3}/library/CPP/include/
)

set(PYLON_LIBS
    $ENV{PYLON_ROOT}/lib/libpylonbase.so
    $ENV{PYLON_ROOT}/lib/libpylonutility.so
    $ENV{PYLON_ROOT}/lib/libXerces-C_gcc40_v2_7.so
    $ENV{PYLON_ROOT}/lib64/pylon/tl/pyloncamemu.so
    $ENV{GENICAM_ROOT_V2_3}/bin/Linux32_i86/libGCBase_gcc40_v2_3.so
    $ENV{GENICAM_ROOT_V2_3}/bin/Linux32_i86/libGenApi_gcc40_v2_3.so
    $ENV{GENICAM_ROOT_V2_3}/bin/Linux32_i86/libMathParser_gcc40_v2_3.so
    $ENV{GENICAM_ROOT_V2_3}/bin/Linux32_i86/libLog_gcc40_v2_3.so
    $ENV{GENICAM_ROOT_V2_3}/bin/Linux32_i86/liblog4cpp_gcc40_v2_3.so
)
#/opt/pylon4/genicam/bin/Linux32_i86
# handle the QUIETLY and REQUIRED arguments and set PYLON_FOUND to TRUE if
# all listed variables are TRUE

#FIND_PACKAGE_HANDLE_STANDARD_ARGS(PYLON REQUIRED_VARS PYLON_INCLUDE PYLON_LIBS)
find_library(BASLER_LIBS libpylonbase.so
						 libGenApi_gcc40_v2_3.so
						 libGCBase_gcc40_v2_3.so 
						 libLog_gcc40_v2_3.so 
						 libMathParser_gcc40_v2_3.so 
						 libXerces-C_gcc40_v2_7_1.so 
						 liblog4cpp_gcc40_v2_3.so
						 PATHS $ENV{PYLON_ROOT}/lib 
						 	   $ENV{PYLON_GENICAM_ROOT}/bin/Linux32_i86
						 	   #$ENV{PYLON_GENICAM_ROOT}/bin/Linux32_i86/GenApi/Generic

)
if(PYLON_FOUND)
  set( PYLON_INCLUDE_DIRS ${PYLON_INCLUDE} )
  set( PYLON_LIBRARIES ${PYLON_LIBS} )
  message(WARNING "PYLON found fdjfh $ENV{PYLON_ROOT} $ENV{GENICAM_ROOT_V2_3} ${BASLER_LIBS}")
else()
  message(WARNING "PYLON NOT found fdjfh $ENV{PYLON_ROOT} $ENV{GENICAM_ROOT_V2_3} ${BASLER_LIBS}")
endif()

# Find third-party libraries
find_package(OpenGL REQUIRED)
find_package(Qt4 4.8.0 COMPONENTS QtCore QtGui QtOpenGL QtNetwork REQUIRED)
find_package(Threads REQUIRED)

find_package(Protobuf REQUIRED)
if(EXISTS ${PROTOBUF_PROTOC_EXECUTABLE})
	message(STATUS "Found PROTOBUF Compiler: ${PROTOBUF_PROTOC_EXECUTABLE}")
else()
	message(FATAL_ERROR "Could not find PROTOBUF Compiler")
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(Eigen3 REQUIRED eigen3)
pkg_check_modules(DC1394 REQUIRED libdc1394-2)

include_directories(
	${PROJECT_BINARY_DIR}
	${Eigen3_INCLUDE_DIRS}
	${PROTOBUF_INCLUDE_DIRS}
)

include ( ${QT_USE_FILE} )

find_package(mvIMPACT QUIET)

include(src/shared/CMakeLists.txt.inc)

if(${mvIMPACT_FOUND} MATCHES "TRUE")
  include_directories(${mvIMPACT_INCLUDE_DIRS})
  add_definitions(-D MVIMPACT)
  set (SHARED_SRCS ${SHARED_SRCS} ${shared_dir}/capture/capture_bluefox2.cpp)
  set (SHARED_HEADERS ${SHARED_HEADERS} ${shared_dir}/capture/capture_bluefox2.h)
  message(STATUS "mvIMPACT found => BlueFox2 support enabled")
else()
  message(STATUS "mvIMPACT NOT found => BlueFox2 support disabled")
endif()

find_package( OpenCV 2 QUIET )
if(OpenCV_FOUND)
  include_directories(${OpenCV_INCLUDE_DIRS})
  add_definitions(-D OPENCV)
  set (OPTIONAL_SRCS ${OPTIONAL_SRCS} src/app/gui/neurowidget.cpp src/app/plugins/plugin_neuralcolorcalib.cpp)
  set (OPTIONAL_HEADERS ${OPTIONAL_HEADERS} src/app/gui/neurowidget.h)
  set (OPTIONAL_UI_SRCS ${OPTIONAL_UI_SRCS} src/app/gui/neurowidget.ui)
  message(STATUS "OpenCV found")
else()
  message(WARNING "OpenCV NOT found => Neural Calibration not available")
endif()

include_directories(${PROJECT_SOURCE_DIR}/src/app)
include_directories(${PROJECT_SOURCE_DIR}/src/app/gui)
include_directories(${PROJECT_SOURCE_DIR}/src/app/plugins)
include_directories(${PROJECT_SOURCE_DIR}/src/app/stacks)
include_directories($(PYLON_ROOT)/include)
include_directories($(PYLON_GENICAM_ROOT)/library/CPP/include)
include_directories($(PYLON_ROOT)/lib)
include_directories($(PYLON_GENICAM_ROOT)/library/CPP/lib/Linux32_i86)

set (SRCS
	src/app/capture_thread.cpp
	src/app/framedata.cpp
	src/app/main.cpp

	src/app/gui/cameracalibwidget.cpp
	src/app/gui/colorpicker.cpp
	src/app/gui/glLUTwidget.cpp
	src/app/gui/glwidget.cpp
	src/app/gui/lutwidget.cpp
	src/app/gui/mainwindow.cpp
	src/app/gui/realtimedisplaywidget.cpp
	src/app/gui/renderoptions.cpp
	src/app/gui/videowidget.cpp
	src/app/gui/jog_dial.cpp

	src/app/plugins/plugin_cameracalib.cpp
	src/app/plugins/plugin_colorcalib.cpp
	src/app/plugins/plugin_colorthreshold.cpp
	src/app/plugins/plugin_detect_balls.cpp
	src/app/plugins/plugin_detect_robots.cpp
	src/app/plugins/plugin_find_blobs.cpp
	src/app/plugins/plugin_publishgeometry.cpp
	src/app/plugins/plugin_legacypublishgeometry.cpp
	src/app/plugins/plugin_runlength_encode.cpp
	src/app/plugins/plugin_sslnetworkoutput.cpp
	src/app/plugins/plugin_legacysslnetworkoutput.cpp
	src/app/plugins/plugin_visualize.cpp
	src/app/plugins/plugin_dvr.cpp
	src/app/plugins/visionplugin.cpp

	src/app/stacks/multistack_robocup_ssl.cpp
	src/app/stacks/multivisionstack.cpp
	src/app/stacks/stack_robocup_ssl.cpp
	src/app/stacks/visionstack.cpp

	${OPTIONAL_SRCS}
)

qt4_wrap_cpp (MOC_SRCS
	src/app/capture_thread.h

	src/app/gui/cameracalibwidget.h
	src/app/gui/glLUTwidget.h
	src/app/gui/glwidget.h
	src/app/gui/lutwidget.h
	src/app/gui/mainwindow.h
	src/app/gui/videowidget.h
	src/app/gui/jog_dial.h
	src/shared/util/lut3d.h
	
	src/app/plugins/plugin_dvr.h
	src/app/plugins/plugin_publishgeometry.h
	src/app/plugins/plugin_legacypublishgeometry.h
	src/app/plugins/visionplugin.h
	
	src/app/stacks/multistack_robocup_ssl.h

	${OPTIONAL_HEADERS}
)

qt4_wrap_ui (UI_SRCS
	src/app/gui/mainwindow.ui
	src/app/gui/videowidget.ui
	${OPTIONAL_UI_SRCS}
)

qt4_add_resources(RC_SRCS 
	src/app/gui/icons/icons_gui.qrc
	${SHARED_RC}
)

## generate moc files for graphicalClient
qt4_wrap_cpp(LCLIENT_MOC_SRCS
    src/logClient/CentralWindow.h
    src/logClient/ClientThreading.h
    src/logClient/GraphicsPrimitives.h
    src/logClient/LogControl.h
)

## generate moc files for graphicalClient2
qt4_wrap_cpp(GCLIENT_MOC_SRCS
    src/graphicalClient/soccerview.h
)

## shared qt wrappings

qt4_wrap_cpp (SHARED_MOC_SRCS
	${SHARED_HEADERS}
)

qt4_add_resources(SHARED_RC_SRCS 
	${SHARED_RC}
)

set (arch ${CMAKE_SYSTEM_PROCESSOR})

message (STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message (STATUS "Arch: ${arch}")

if (${arch} MATCHES "x86_64")
	#set compiler flags here for x86_64
	set (cc_flags "")
elseif (${arch} MATCHES "x86")
	#set compiler flags here for x86
	set (cc_flags "")
endif()

# make sure to use C++98 and enable warnings
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi")
# enable warnings
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#flags to set in debug mode
set (CMAKE_CXX_FLAGS_DEBUG "-g -Wl,--no-as-needed")

#flags to set in release mode
set (CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -O3 -march=native -Wl,--no-as-needed")

message( STATUS "CC Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message (STATUS "Custom CC Flags: ${cc_flags}")

add_definitions(${cc_flags})

## build the common code
add_library(sslvision ${SHARED_MOC_SRCS} ${SHARED_RC_SRCS} ${CC_PROTO} ${SHARED_SRCS})
add_dependencies(sslvision GenerateProto)

set (libs ${QT_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${OPENGL_gl_LIBRARY}
	${OPENGL_glu_LIBRARY}
	${PROTOBUF_LIBRARIES}
	${DC1394_LIBRARIES}
	${mvIMPACT_LIBRARIES}
	${OpenCV_LIBS}
	${BASLER_LIBS}
	${PYLON_LIBS}
	sslvision
)

## build the main app
set (target vision)
add_executable(${target} ${UI_SRCS} ${MOC_SRCS} ${RC_SRCS} ${SRCS})
target_link_libraries(${target} ${libs})

##build non graphical client
set (client client)
add_executable(${client} src/client/main.cpp )
target_link_libraries(${client} ${libs})

##build logging client
set (lclient logClient)
add_executable(${lclient} ${LCLIENT_MOC_SRCS}
	src/logClient/main.cpp 
	src/logClient/CentralWindow.cpp
	src/logClient/ClientThreading.cpp
	src/logClient/GraphicsPrimitives.cpp
	src/logClient/LogControl.cpp
)
target_link_libraries(${lclient} ${libs})

##build graphical client
set (gclient graphicalClient)
add_executable(${gclient} ${GCLIENT_MOC_SRCS}
  src/graphicalClient/main.cpp
  src/graphicalClient/soccerview.cpp
  src/graphicalClient/gltext.cpp
)
target_link_libraries(${gclient} ${libs})




mark_as_advanced(PYLON_INCLUDE PYLON_LIBS)