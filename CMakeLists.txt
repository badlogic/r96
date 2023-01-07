cmake_minimum_required(VERSION 3.21.1)
project(r96)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
FetchContent_Declare(minifb GIT_REPOSITORY https://github.com/badlogic/minifb GIT_TAG dos-pr-master)
set(MINIFB_BUILD_EXAMPLES CACHE INTERNAL FALSE)
FetchContent_MakeAvailable(minifb)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-implicit-fallthrough)

add_library(r96 "src/r96/r96.c")
add_executable(r96_00_basic_window "src/00_basic_window.c")
add_executable(r96_01_drawing_a_pixel "src/01_drawing_a_pixel.c")

add_custom_target(r96_web_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/web
    $<TARGET_FILE_DIR:r96_00_basic_window>
)

get_property(targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
list(REMOVE_ITEM targets minifb r96 r96_assets r96_web_assets)
foreach(target IN LISTS targets)
    target_link_libraries(${target} LINK_PUBLIC minifb r96)    
    if(EMSCRIPTEN)
        add_dependencies(${target} r96_web_assets)
        target_link_options(${target} PRIVATE
                "-sSTRICT=1"
                "-sENVIRONMENT=web"
                "-sLLD_REPORT_UNDEFINED"
                "-sMODULARIZE=1"
                "-sALLOW_MEMORY_GROWTH=1"
                "-sALLOW_TABLE_GROWTH"
                "-sMALLOC=emmalloc"
                "-sEXPORT_ALL=1"
                "-sEXPORTED_FUNCTIONS=[\"_malloc\",\"_free\",\"_main\"]"
                "-sASYNCIFY"
                "--no-entry"
                "-sEXPORT_NAME=${target}"
        )
    endif()
endforeach()
