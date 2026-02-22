find_path(SFML_INCLUDE_DIR
  NAMES SFML/Graphics.hpp
  PATHS /usr/include /usr/include/SFML
)

find_library(SFML_SYSTEM_LIBRARY
  NAMES sfml-system
  PATHS /usr/lib /usr/lib64
)

find_library(SFML_WINDOW_LIBRARY
  NAMES sfml-window
  PATHS /usr/lib /usr/lib64
)

find_library(SFML_GRAPHICS_LIBRARY
  NAMES sfml-graphics
  PATHS /usr/lib /usr/lib64
)

find_library(SFML_AUDIO_LIBRARY
  NAMES sfml-audio
  PATHS /usr/lib /usr/lib64
)

if(NOT SFML_INCLUDE_DIR)
  message(FATAL_ERROR "Could not find SFML include directory")
endif()

if(NOT SFML_SYSTEM_LIBRARY OR NOT SFML_WINDOW_LIBRARY OR NOT SFML_GRAPHICS_LIBRARY OR NOT SFML_AUDIO_LIBRARY)
  message(FATAL_ERROR "Could not find SFML libraries")
endif()

set(SFML_INCLUDE_DIRS ${SFML_INCLUDE_DIR})
set(SFML_LIBRARIES ${SFML_SYSTEM_LIBRARY} ${SFML_WINDOW_LIBRARY} ${SFML_GRAPHICS_LIBRARY} ${SFML_AUDIO_LIBRARY})
