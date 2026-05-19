cmake_minimum_required(VERSION 3.10)
project(audio_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(audio_project main.cpp)

# Find RtAudio (assumes RtAudio is in ../rtaudio or system path)
find_path(RTAUDIO_INCLUDE_DIR RtAudio.h
  PATHS ${CMAKE_SOURCE_DIR}/../rtaudio
)
find_library(RTAUDIO_LIBRARY NAMES rtaudio
  PATHS ${CMAKE_SOURCE_DIR}/../rtaudio
)

if (RTAUDIO_INCLUDE_DIR AND RTAUDIO_LIBRARY)
  target_include_directories(audio_project PRIVATE ${RTAUDIO_INCLUDE_DIR})
  target_link_libraries(audio_project PRIVATE ${RTAUDIO_LIBRARY})
else()
  message(FATAL_ERROR "RtAudio not found. Set RTAUDIO_INCLUDE_DIR and RTAUDIO_LIBRARY.")
endif()

# Extra warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(audio_project PRIVATE -Wall -Wextra -pedantic)
endif()
