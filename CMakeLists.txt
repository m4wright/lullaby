# CMakeList.txt : CMake project for Lullabies, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_RELEASE "-O1" CACHE STRING "C++ compiler flags for Release build" FORCE)

# Add source to this project's executable.
add_executable (Lullabies "Lullabies.cpp" "AudioPlayer.cpp" "MusicController.cpp" "Song.h" "MusicRepository.cpp" "MusicService.cpp" "MusicSerializer.h" "MusicSerializer.cpp")

if (WIN32)
	add_library(sqlite3 third_party/winlibs/sqlite3.c)

	target_include_directories(sqlite3 PUBLIC winlibs)

	target_link_libraries(Lullabies PRIVATE sqlite3)

else()
	find_package(SQLite3 REQUIRED)

	target_link_libraries(Lullabies PRIVATE SQLite::SQLite3)
endif()
