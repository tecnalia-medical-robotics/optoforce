project(omd)

message ("********************************************")
message ("starting cmake management of ${PROJECT_NAME}")
include_directories(include)


# Compatiblity with ROS
if(DEFINED ENV{ROS_ROOT})
  message ("ROS defined -- using catkin ")
  find_package(catkin REQUIRED)
  catkin_package(INCLUDE_DIRS include)
else()
  message ("ROS not defined -- not using catkin ")
endif()


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
# this line was uncommented.
#find_package(omd)

# Would have thought this solution would be enough .. to be investigated
#message("omd path defined ${CMAKE_CURRENT_LIST_DIR}")
#set(omd_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/lib/linux/libOMD.so")
#set(omd_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")
# Define headers for this library. PUBLIC headers are used for
# compiling the library, and will be added to consumers' build
# paths.

# target_include_directories(omd PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:include>
#     PRIVATE src)

# install(TARGETS omd
#     ARCHIVE  DESTINATION lib
#     LIBRARY  DESTINATION lib
#     RUNTIME  DESTINATION bin)  # This is for Windows
# install(DIRECTORY include/ DESTINATION include)

# # This makes the project importable from the build directory
# export(TARGETS omd FILE omdConfig.cmake)

message ("********************************************")
message ("cmake management of ${PROJECT_NAME} done")