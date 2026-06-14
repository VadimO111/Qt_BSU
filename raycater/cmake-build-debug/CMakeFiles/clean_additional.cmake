# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles/raycater_autogen.dir/AutogenUsed.txt"
  "CMakeFiles/raycater_autogen.dir/ParseCache.txt"
  "raycater_autogen"
  )
endif()
