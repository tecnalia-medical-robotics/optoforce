#
# Official dependency number 0: CMake.
# Obviously, we must have it if we are here but test version.
#
CMAKE_MINIMUM_REQUIRED( VERSION 2.6 FATAL_ERROR )
#
# Enter some information about the project
# This is the only configurable part of the build system
#
# project name
PROJECT( optoforce_driver )

# Turn on the ability to create folders to organize projects (.vcproj)
# It creates "CMakePredefinedTargets" folder by default and adds CMake
# defined projects like INSTALL.vcproj and ZERO_CHECK.vcproj
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set compiler flags and options. 
# Here it is setting the Visual Studio warning level to 4
# todo only authotize this on windows
#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")

# Command to output information to the console
# Useful for displaying errors, warnings, and debugging
#message ("cxx Flags: " ${CMAKE_CXX_FLAGS})

#SET ( CMAKE_VERBOSE_MAKEFILE OFF )
SET ( CMAKE_BUILD_TYPE Release )
#SET ( CMAKE_BUILD_TYPE Debug )

# Sub-directories where more CMakeLists.txt exist
add_subdirectory(omd)
add_subdirectory(optoforce)
