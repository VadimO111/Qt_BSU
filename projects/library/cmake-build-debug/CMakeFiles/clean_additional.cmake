# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/library_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/library_autogen.dir/ParseCache.txt"
  "library_autogen"
  )
endif()
