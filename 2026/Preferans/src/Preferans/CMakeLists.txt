cmake_minimum_required(VERSION 3.14)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("Preferans")

add_executable(Preferans "Preferans.cpp" "Preferans.hpp" "src/Game.cpp" "src/Card.cpp" "include/Card.hpp" "src/Button.cpp" "include/Button.hpp" "src/Player.cpp" "include/Player.hpp" "src/UI.cpp" "include/UI.hpp"  "include/GameConfig.hpp" "src/BiddingGrid.cpp" "include/BiddingGrid.hpp" "src/Minimax.cpp" "include/Minimax.hpp")

if(CMAKE_VERSION VERSION_GREATER 3.14)
  set_property(TARGET Preferans PROPERTY CXX_STANDARD 23)
endif()

if(WIN32)
    set(BUILD_SHARED_LIBS OFF)
    include(FetchContent)
    FetchContent_Declare(SFML
        GIT_REPOSITORY https://github.com/SFML/SFML.git
        GIT_TAG        2.6.2
    )
    FetchContent_MakeAvailable(SFML)
    target_link_libraries(Preferans PRIVATE sfml-graphics sfml-window sfml-system)
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
