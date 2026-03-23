# CMakeList.txt : CMake project for Lullabies, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_FLAGS_RELEASE "-O2" CACHE STRING "C++ compiler flags for Release build" FORCE)

# Split core library and audio implementation so tests can provide a mock AudioPlayer.
set(LULLABY_SOURCES
	"src/MusicController.cpp"
	"src/MusicRepository.cpp"
	"src/MusicService.cpp"
	"src/MusicSerializer.cpp"
)

add_library(lullaby_core STATIC ${LULLABY_SOURCES})
target_include_directories(lullaby_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_library(audio_impl STATIC src/AudioPlayer.cpp)
target_include_directories(audio_impl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Lullabies "src/Lullabies.cpp")
target_link_libraries(Lullabies PRIVATE lullaby_core audio_impl)

if (WIN32)
	add_library(sqlite3 src/third_party/winlibs/sqlite3.c)
	target_include_directories(sqlite3 PUBLIC src/third_party/winlibs)
	target_link_libraries(lullaby_core PRIVATE sqlite3)
	target_link_libraries(Lullabies PRIVATE sqlite3)
else()
	find_package(SQLite3 REQUIRED)
	target_link_libraries(lullaby_core PRIVATE SQLite::SQLite3)
	target_link_libraries(Lullabies PRIVATE SQLite::SQLite3)
endif()

option(BUILD_TESTS "Build unit tests" ON)
if(BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
