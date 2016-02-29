set(omd_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../lib/linux/libOMD.so")
set(omd_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../include")

include(FindPackageHandleStandardArgs)
# Handle the QUIETLY and REQUIRED arguments and set the LM_SENSORS_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(omd DEFAULT_MSG
                                  omd_LIBRARIES omd_INCLUDE_DIRS)

mark_as_advanced(omd_INCLUDE_DIRS omd_LIBRARIES)