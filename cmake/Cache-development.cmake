SET(CMAKE_C_COMPILER "/usr/bin/clang" CACHE STRING "" FORCE)
SET(CMAKE_CXX_COMPILER "/usr/bin/clang++" CACHE STRING "" FORCE)
#SET(CMAKE_C_FLAGS "-m32" CACHE STRING "" FORCE)
#SET(CMAKE_CXX_FLAGS "-m32" CACHE STRING "" FORCE)

SET(BUILD_UNIT_TESTS ON CACHE BOOL "" FORCE)
SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "" FORCE)
SET(MARSHMALLOW_DEBUG ON CACHE BOOL "" FORCE)
SET(MARSHMALLOW_DEBUG_VERBOSITY "1" CACHE STRING "" FORCE)
