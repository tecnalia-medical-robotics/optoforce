cmake_minimum_required (VERSION 2.6)
project(optoforce)

#todo check if this is the good place for this
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
 
find_package( Boost REQUIRED COMPONENTS program_options system thread date_time chrono)
include_directories( ${Boost_INCLUDE_DIR} )

set(SOURCES "src/optoforce_driver.cpp"
  "include/optoforce/optoforce_driver.hpp"
  "src/optoforce_array_driver.cpp" 
  "include/optoforce/optoforce_array_driver.hpp"
  "src/optoforce_acquisition.cpp"
  "include/optoforce/optoforce_acquisition.hpp")
# Create named folders for the sources within the .vcproj
# Empty name lists them directly under the .vcproj
source_group("optoforce" FILES ${SOURCES})
# Properties->C/C++->General->Additional Include Directories
# todo how to skip this, and get omd include  directly from the dependencies
include_directories(include)

list(APPEND CMAKE_MODULE_PATH "${omd_SOURCE_DIR}/cmake")
find_package(omd)

#message("[${PROJECT_NAME}] Looking at omd include dirs ${omd_INCLUDE_DIRS}")
#message("[${PROJECT_NAME}] Looking at omd include dirs ${omd_INCLUDES}")
#message("[${PROJECT_NAME}] Looking at omd source ${omd_SOURCE_DIR}")
#message("[${PROJECT_NAME}] Looking at omd library ${omd_LIBRARIES}")
#message("[${PROJECT_NAME}] Looking at omd library ${omd_LIBS}")

add_library(optoforce STATIC ${SOURCES})

include_directories(${omd_INCLUDE_DIRS})

target_link_libraries(optoforce ${omd_LIBRARIES})

# adding the examples executables. todo: make this compilation optional
add_executable(bin_array_opto_force examples/test_opto_device_array.cpp)
target_link_libraries(bin_array_opto_force optoforce )

add_executable(sample_force_acq examples/example_acquisition.cpp)
target_link_libraries(sample_force_acq optoforce ${Boost_LIBRARIES})

add_executable(sample_config_force_acq examples/example_config_acquisition.cpp)
target_link_libraries(sample_config_force_acq optoforce  ${Boost_LIBRARIES} yaml-cpp)

add_executable(test_opto_device_array examples/test_opto_device_array.cpp)
target_link_libraries(test_opto_device_array optoforce  ${Boost_LIBRARIES} yaml-cpp)

# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.
target_include_directories(optoforce PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

install(TARGETS optoforce
    ARCHIVE  DESTINATION lib
    LIBRARY  DESTINATION lib
    RUNTIME  DESTINATION bin)  # This is for Windows
install(DIRECTORY include/ DESTINATION include)

# This makes the project importable from the build directory
export(TARGETS optoforce FILE optoforceConfig.cmake)

# Compatiblity with ROS
if(DEFINED ENV{ROS_ROOT})
  #message ("[${PROJECT_NAME}] ROS defined -- using catkin ")
  find_package(catkin REQUIRED)
  #message( "[${PROJECT_NAME}] checking in: ${CMAKE_CURRENT_BINARY_DIR}")
  find_library(LIBOPTOFORCE NAMES "optoforce" PATHS "${CMAKE_CURRENT_BINARY_DIR}")
  #message ("[${PROJECT_NAME}] Found ${LIBOPTOFORCE}") 

 catkin_package(INCLUDE_DIRS include
    #LIBRARIES ${CMAKE_CURRENT_BINARY_DIR}/libdummy2.a
    LIBRARIES
    CFG_EXTRAS my-extras.cmake
    )
  set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    PREFIX "lib"
    LIBRARY_OUTPUT_DIRECTORY ${CATKIN_DEVEL_PREFIX}/${CATKIN_PACKAGE_PYTHON_DESTINATION}
    )
  message("[${PROJECT_NAME}] other library site would be:  ${CATKIN_PACKAGE_LIB_DESTINATION}")
else()
  message ("[${PROJECT_NAME}] ROS not defined -- not using catkin ")
endif()


# Creates folder "libraries" and adds target project (math.vcproj)
#todo check it effects under linux / windows
#set_property(TARGET optoforce PROPERTY FOLDER "libraries")

# Adds logic to INSTALL.vcproj to copy math.dll to destination directory
# todo check its effect under windows
#install (TARGETS optoforce
#  ARCHIVE DESTINATION ${PROJECT_BINARY_DIR}/lib
#  LIBRARY DESTINATION ${PROJECT_BINARY_DIR}/lib		
#  RUNTIME DESTINATION ${PROJECT_BINARY_DIR}/bin)


#The following lines may be only necessary on Windows ?

#file(COPY ${omd_SOURCE_DIR}/lib/linux/OMD.dll DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug )
#file(COPY ${omd_SOURCE_DIR}/lib/linux/libOMD.so.1.5.0 DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug )
#file(COPY ${omd_SOURCE_DIR}/lib/linux/libOMD.so.1.5 DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug )
#file(COPY ${omd_SOURCE_DIR}/lib/linux/libOMD.so.1 DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug )
#file(COPY ${omd_SOURCE_DIR}/lib/linux/libOMD.so DESTINATION ${CMAKE_BINARY_DIR}/bin/Debug )
message ("[${PROJECT_NAME}] ********************************************")
message ("[${PROJECT_NAME}] cmake management of ${PROJECT_NAME} done")
