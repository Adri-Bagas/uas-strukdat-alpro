cmake_minimum_required(VERSION 3.10)
project(NcursesApp)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(json)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

set(CURSES_NEED_WIDE TRUE)
find_package(Curses QUIET)

if(NOT CURSES_FOUND)
    message(FATAL_ERROR 
        "\n"
        "====================================================================\n"
        " ERROR: Ncurses library is missing!\n"
        " Ncurses is a system library and cannot be downloaded automatically.\n"
        " \n"
        " Please install it using your operating system's package manager:\n"
        "   Ubuntu/Debian   : sudo apt-get install libncurses5-dev libncursesw5-dev\n"
        "   Fedora/openSUSE : sudo dnf/zypper install ncurses-devel\n"
        "   Arch Linux      : sudo pacman -S ncurses\n"
        "   macOS (Homebrew): brew install ncurses\n"
        "   Windows         : via MSYS2 or just install linux man...\n"
        "====================================================================\n"
    )
endif()

find_library(MENU_LIBRARY NAMES menuw menu)
if(NOT MENU_LIBRARY)
    message(FATAL_ERROR "Ncurses 'menu' library not found! Please ensure you installed the full ncurses development package.")
endif()

find_library(PANEL_LIBRARY NAMES panelw panel REQUIRED)
if(NOT PANEL_LIBRARY)
    message(FATAL_ERROR "Ncurses 'panel' library not found! Please ensure you installed the full ncurses development package.")
endif()

file(GLOB_RECURSE SOURCES "src/**/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(ftdd src/main.cpp src/GameEngine.cpp src/managers/MusicManager.cpp ${SOURCES})

target_include_directories(ftdd PRIVATE ${CURSES_INCLUDE_DIRS} ${CURSES_INCLUDE_DIRS}/ncursesw ${CURSES_INCLUDE_DIRS}/ncurses)

# Tambahan pthread, dl, dan m untuk dependensi sistem Miniaudio di Linux/WSL
target_link_libraries(ftdd PRIVATE nlohmann_json::nlohmann_json ${CURSES_LIBRARIES} ${MENU_LIBRARY} ${PANEL_LIBRARY} asound pulse pthread dl m)
