cmake_minimum_required(VERSION 3.28)
project(fusionquest)

# Set the output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set(CMAKE_OSX_ARCHITECTURES "arm64")
## Boost
#set(BOOST_INCLUDE_LIBRARIES thread filesystem system program_options asio date_time)
#set(BOOST_ENABLE_CMAKE ON)

include(FetchContent)

#FetchContent_Declare(
#        Boost
#        GIT_REPOSITORY https://github.com/boostorg/boost.git
#        GIT_PROGRESS TRUE
#        GIT_TAG boost-1.85.0 # Update to the required Boost version
#)
#FetchContent_MakeAvailable(Boost)

# FTXUI
FetchContent_Declare(
        ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v6.0.0  # Important: Specify a version or a commit hash here.
)
FetchContent_MakeAvailable(ftxui)

# Include Boost and FTXUI headers
#include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ftxui_SOURCE_DIR}/include)

# Add source files to the project
set(SOURCES
        src/main.cpp
        src/functions.cpp
        src/ui.cpp
        src/main_menu.cpp
        src/page_one.cpp
        src/page_two.cpp
        src/character_creation.cpp
        src/stats_page.cpp
        src/load_page.cpp
        src/core/engine_core.cpp
)

set(HEADERS
        src/banner.h
        src/functions.h
        src/character.h
        src/ui.h
        src/progress_bar.h
        src/logger.h
        src/globals.h
        src/core/engine_core.h
        src/core/engine_systems.h
        src/core/engine_components.h
)

# Add executable
add_executable(fusionquest ${SOURCES} ${HEADERS})

# Link FTXUI libraries
target_link_libraries(fusionquest
        PRIVATE ftxui::screen
        PRIVATE ftxui::dom
        PRIVATE ftxui::component
)

## Link Boost libraries
#target_link_libraries(fusionquest
#        PRIVATE Boost::asio
#        PRIVATE Boost::filesystem
#        PRIVATE Boost::thread
#        PRIVATE Boost::program_options
#        PRIVATE Boost::system
#)
# Set C++ standard
if (CMAKE_VERSION VERSION_GREATER 3.24)
    set_property(TARGET fusionquest PROPERTY CXX_STANDARD 20)
endif()
## Ensure Boost is properly found
#if (NOT TARGET Boost::asio OR NOT TARGET Boost::filesystem OR NOT TARGET Boost::thread OR NOT TARGET Boost::program_options)
#    message(FATAL_ERROR "Required Boost libraries not found!")
#endif()

# Ensure FTXUI is properly found
if (NOT TARGET ftxui::screen OR NOT TARGET ftxui::dom)
    message(FATAL_ERROR "FTXUI libraries not found!")
endif()