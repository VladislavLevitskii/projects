cmake_minimum_required(VERSION 3.8)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Preferans")

add_executable(Preferans "Preferans.cpp" "Preferans.h" "src/Game.cpp" "src/Card.cpp" "src/Card.h" "src/Button.cpp" "src/Button.h" "src/Player.cpp" "src/Player.h" "src/UI.cpp" "src/UI.h"  "src/GameConfig.h")

if(CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Preferans PROPERTY CXX_STANDARD 20)
endif()

if(WIN32)
    set(SFML_DIR "${CMAKE_SOURCE_DIR}/SFML-2.5.1/lib/cmake/SFML")
    find_package(SFML 2.5.1 COMPONENTS graphics window system REQUIRED)
    target_link_libraries(Preferans PRIVATE sfml-graphics sfml-window sfml-system)

    add_custom_command(TARGET Preferans POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${SFML_DIR}/../../../bin" $<TARGET_FILE_DIR:Preferans>)
else()
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules" ${CMAKE_MODULE_PATH})
    find_package(SFML 2 REQUIRED COMPONENTS graphics window system audio)
    target_link_libraries(Preferans PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
endif()
    add_custom_command(TARGET Preferans POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/images"
    $<TARGET_FILE_DIR:Preferans>/images)

    add_custom_command(TARGET Preferans POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/fonts"
    $<TARGET_FILE_DIR:Preferans>/fonts)
