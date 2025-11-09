# CmakeLists.txt byl vytvoren pomoci AI

cmake_minimum_required(VERSION 3.16)
project(GameOfLife LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# If raylib is installed via Homebrew, add its opt path to CMAKE_PREFIX_PATH so find_package(... CONFIG) can find it
if (EXISTS "/opt/homebrew/opt/raylib")
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/raylib")
endif()
if (EXISTS "/usr/local/opt/raylib")
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/opt/raylib")
endif()

# Try to find raylib: prefer a CMake config provided by Homebrew, then pkg-config, then Homebrew prefix fallback
set(_hb_paths "/opt/homebrew/opt/raylib" "/usr/local/opt/raylib")
find_package(raylib QUIET CONFIG PATHS ${_hb_paths} NO_DEFAULT_PATH)
if (raylib_FOUND)
    message(STATUS "Found raylib via CMake config (raylib::raylib)")
else()
    # Try pkg-config (useful for some installs)
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(RAYLIB QUIET raylib)
    endif()
    if (RAYLIB_FOUND)
        add_library(raylib::raylib INTERFACE IMPORTED)
        target_include_directories(raylib::raylib INTERFACE ${RAYLIB_INCLUDE_DIRS})
        target_link_libraries(raylib::raylib INTERFACE ${RAYLIB_LIBRARIES})
        message(STATUS "Found raylib via pkg-config: ${RAYLIB_LIBRARIES}")
    else()
        # Last-resort: ask Homebrew where raylib lives and create an imported target from its include/lib
        execute_process(COMMAND brew --prefix raylib
                        RESULT_VARIABLE _brew_res
                        OUTPUT_VARIABLE _brew_prefix
                        OUTPUT_STRIP_TRAILING_WHITESPACE)
        if (_brew_res EQUAL 0 AND EXISTS "${_brew_prefix}")
            set(_hb_inc "${_brew_prefix}/include")
            set(_hb_lib "${_brew_prefix}/lib/libraylib.dylib")
            if (NOT EXISTS "${_hb_lib}")
                file(GLOB _alt_libs "${_brew_prefix}/lib/libraylib*")
                list(GET _alt_libs 0 _hb_lib)
            endif()
            if (EXISTS "${_hb_inc}/raylib.h" AND EXISTS "${_hb_lib}")
                add_library(raylib::raylib INTERFACE IMPORTED)
                set_target_properties(raylib::raylib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_hb_inc}" INTERFACE_LINK_LIBRARIES "${_hb_lib}")
                message(STATUS "Configured raylib::raylib from Homebrew prefix: ${_hb_lib}")
            endif()
        endif()

        if (NOT TARGET raylib::raylib)
            message(FATAL_ERROR "raylib not found. Install via Homebrew (brew install raylib) or set CMAKE_PREFIX_PATH to raylib's install folder (e.g. /opt/homebrew/opt/raylib).")
        endif()
    endif()
endif()

add_executable(GameOfLife
    main.cpp
)

# If raylib wasn't found earlier, try Homebrew prefix directly and create imported target
if (NOT TARGET raylib::raylib)
    execute_process(COMMAND brew --prefix raylib
                    RESULT_VARIABLE _brew_res
                    OUTPUT_VARIABLE _brew_prefix
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (_brew_res EQUAL 0 AND EXISTS "${_brew_prefix}")
        set(_hb_inc "${_brew_prefix}/include")
        set(_hb_lib_dylib "${_brew_prefix}/lib/libraylib.dylib")
        set(_hb_lib_a "${_brew_prefix}/lib/libraylib.a")
        if (EXISTS "${_hb_lib_dylib}")
            set(_hb_lib "${_hb_lib_dylib}")
        elseif (EXISTS "${_hb_lib_a}")
            set(_hb_lib "${_hb_lib_a}")
        endif()
        if (DEFINED _hb_lib)
            add_library(raylib::raylib INTERFACE IMPORTED)
            set_target_properties(raylib::raylib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${_hb_inc}" INTERFACE_LINK_LIBRARIES "${_hb_lib}")
            message(STATUS "Configured raylib::raylib from Homebrew prefix: ${_hb_lib}")
        endif()
    endif()
endif()

# Prefer higher optimization in Release
if (CMAKE_BUILD_TYPE MATCHES "^Release$")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang|GNU")
        target_compile_options(GameOfLife PRIVATE -O3)
    endif()
endif()

# Link raylib (prefer imported target, fallback to found variables)
if (TARGET raylib::raylib)
    # Raylib provided as a CMake target (preferred)
    target_link_libraries(GameOfLife PRIVATE raylib::raylib)
else()
    # Fallbacks: pkg-config or manual find populated these variables earlier
    if (DEFINED RAYLIB_LIBRARIES AND RAYLIB_LIBRARIES)
        target_include_directories(GameOfLife PRIVATE ${RAYLIB_INCLUDE_DIRS})
        target_link_libraries(GameOfLife PRIVATE ${RAYLIB_LIBRARIES})
    elseif (DEFINED RAYLIB_LIBRARY AND RAYLIB_LIBRARY)
        target_include_directories(GameOfLife PRIVATE ${RAYLIB_INCLUDE_DIR})
        target_link_libraries(GameOfLife PRIVATE ${RAYLIB_LIBRARY})
    else()
        message(FATAL_ERROR "raylib target not available and no RAYLIB_* variables set. Install raylib (brew install raylib) or set CMAKE_PREFIX_PATH to raylib's install folder.")
    endif()
endif()

# On macOS, ensure we have the correct frameworks if using a non-config raylib linkage
if(APPLE)
    # Many Homebrew raylib builds already carry these in the raylib target; this is a safe fallback for pkg-config case.
    target_link_libraries(GameOfLife PRIVATE
        "-framework Cocoa" "-framework IOKit" "-framework CoreVideo" "-framework OpenGL" "-framework CoreAudio" "-framework AudioToolbox"
    )
endif()
