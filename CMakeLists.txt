# CMakeList.txt : CMake project for Lullabies, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_RELEASE "-O1" CACHE STRING "C++ compiler flags for Release build" FORCE)

# Add source to this project's executable.
add_executable (Lullabies "Lullabies.cpp" "AudioPlayer.cpp" "MusicController.cpp")

# TODO: Add tests and install targets if needed.
