cmake_minimum_required(VERSION 3.10)
project(audio_project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable Qt MOC for Q_OBJECT macro
set(CMAKE_AUTOMOC ON)

# Set include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/widgets)
include_directories(${CMAKE_SOURCE_DIR}/utils)


set(SRC_FILES
  src/main.cpp
  src/Metronome.cpp
  src/ChordProgression.cpp
  src/ChordAudioEngine.cpp
  src/ChordProgressionManager.cpp
  widgets/ChordTrackWidget.cpp
  widgets/GuitarFretboardWidget.cpp
  widgets/HorizontalFretboardWidget.cpp
  widgets/NoteMapFretboardWidget.cpp
  widgets/BeatVisualizerWidget.cpp
  widgets/ChordProgressionWidget.cpp
  utils/NoteMapUtils.cpp
  strumming/StrummingPattern.cpp
)

# Place all Qt autogen files in a dedicated 'qt' folder in the build directory
set(CMAKE_AUTOGEN_OUTPUT_DIR ${CMAKE_BINARY_DIR}/qt)

add_executable(audio_project ${SRC_FILES})
set_target_properties(audio_project PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src
  AUTOGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qt/audio_project_autogen
)

# Find Qt5 Widgets
find_package(Qt5 COMPONENTS Widgets REQUIRED)
target_link_libraries(audio_project PRIVATE Qt5::Widgets)

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


# Option to build tests (default ON)
option(BUILD_TESTS "Build test and demo executables" ON)

if(BUILD_TESTS)

    add_executable(test_metronome_beats tests/test_metronome_beats.cpp src/Metronome.cpp src/ChordProgressionManager.cpp)
    set_target_properties(test_metronome_beats PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
    if (RTAUDIO_INCLUDE_DIR)
      target_include_directories(test_metronome_beats PRIVATE ${RTAUDIO_INCLUDE_DIR})
    endif()
    if (RTAUDIO_LIBRARY)
      target_link_libraries(test_metronome_beats PRIVATE ${RTAUDIO_LIBRARY})
    endif()
    target_link_libraries(test_metronome_beats PRIVATE Qt5::Widgets)

    add_executable(test_chord_strong_beat
      tests/test_chord_strong_beat.cpp
      src/ChordAudioEngine.cpp
      src/ChordProgression.cpp
      src/Metronome.cpp
      strumming/StrummingPattern.cpp
    )
    if (RTAUDIO_INCLUDE_DIR)
      target_include_directories(test_chord_strong_beat PRIVATE ${RTAUDIO_INCLUDE_DIR})
    endif()
    if (RTAUDIO_LIBRARY)
      target_link_libraries(test_chord_strong_beat PRIVATE ${RTAUDIO_LIBRARY})
    endif()
    target_link_libraries(test_chord_strong_beat PRIVATE Qt5::Widgets)
    set_target_properties(test_chord_strong_beat PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

        add_executable(test_metronome_visual tests/test_metronome_visual.cpp widgets/BeatVisualizerWidget.cpp src/Metronome.cpp)
        set_target_properties(test_metronome_visual PROPERTIES
          RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
        )
        if (RTAUDIO_INCLUDE_DIR)
          target_include_directories(test_metronome_visual PRIVATE ${RTAUDIO_INCLUDE_DIR})
        endif()
        if (RTAUDIO_LIBRARY)
          target_link_libraries(test_metronome_visual PRIVATE ${RTAUDIO_LIBRARY})
        endif()
        target_link_libraries(test_metronome_visual PRIVATE Qt5::Widgets)
    add_executable(test_chord_shape_to_freq tests/test_chord_shape_to_freq.cpp)
    set_target_properties(test_chord_shape_to_freq PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

    add_executable(test_open_string_freqs tests/test_open_string_freqs.cpp)
    set_target_properties(test_open_string_freqs PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )

    add_executable(test_fretboard tests/test_fretboard.cpp widgets/GuitarFretboardWidget.cpp)
    set_target_properties(test_fretboard PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
      AUTOGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qt/test_fretboard_autogen
    )

    add_executable(test_chord_strumming
      tests/test_chord_strumming.cpp
      src/ChordAudioEngine.cpp
      src/ChordProgression.cpp
      src/Metronome.cpp
      strumming/StrummingPattern.cpp
      # Add any other needed sources here
    )
    if (RTAUDIO_INCLUDE_DIR)
      target_include_directories(test_chord_strumming PRIVATE ${RTAUDIO_INCLUDE_DIR})
    endif()
    if (RTAUDIO_LIBRARY)
      target_link_libraries(test_chord_strumming PRIVATE ${RTAUDIO_LIBRARY})
    endif()
    target_link_libraries(test_chord_strumming PRIVATE Qt5::Widgets)
    set_target_properties(test_chord_strumming PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    )
  target_link_libraries(test_fretboard Qt5::Widgets)

  add_executable(test_horizontal_fretboard tests/test_horizontal_fretboard.cpp widgets/HorizontalFretboardWidget.cpp)
  set_target_properties(test_horizontal_fretboard PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    AUTOGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qt/test_horizontal_fretboard_autogen
  )
  target_link_libraries(test_horizontal_fretboard Qt5::Widgets)

  add_executable(test_note_map tests/test_note_map.cpp widgets/NoteMapFretboardWidget.cpp utils/NoteColorUtils.h)
  set_target_properties(test_note_map PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
    AUTOGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qt/test_note_map_autogen
  )
  target_link_libraries(test_note_map Qt5::Widgets)
endif()
