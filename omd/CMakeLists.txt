project(omd)

include_directories(include)

# Compatiblity with ROS
#catkin_package(INCLUDE_DIRS include)

#list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(omd)

# Would have thought this solution would be enough .. to be investigated
#message("omd path defined ${CMAKE_CURRENT_LIST_DIR}")
#set(omd_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/lib/linux/libOMD.so")
#set(omd_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/include")
