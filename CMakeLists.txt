cmake_minimum_required(VERSION 3.21.1)
project(r96)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

include(FetchContent)
FetchContent_Declare(minifb GIT_REPOSITORY https://github.com/badlogic/minifb GIT_TAG dos-pr-master)
set(MINIFB_BUILD_EXAMPLES CACHE INTERNAL FALSE)
FetchContent_MakeAvailable(minifb)

if(CMAKE_BUILD_TYPE MATCHES Release)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()
endif()

if (NOT DJGPP)
    add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wno-implicit-fallthrough)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-implicit-fallthrough)
endif()

add_library(r96 "src/r96/r96.c" "src/r96/stb_image.c")
add_executable(r96_00_basic_window "src/00_basic_window.c")
add_executable(r96_01_drawing_a_pixel "src/01_drawing_a_pixel.c")
add_executable(r96_02_image "src/02_image.c")
add_executable(r96_03_clear_profiling "src/03_clear_profiling.c")
add_executable(r96_04_hline "src/04_hline.c")
add_executable(r96_05_hline_opt "src/05_hline_opt.c")
add_executable(r96_06_rect "src/06_rect.c")
add_executable(r96_07_rect_opt "src/07_rect_opt.c")
add_executable(r96_08_image_file "src/08_image_file.c")
add_executable(r96_09_blit "src/09_blit.c")
add_executable(r96_10_blit_keyed "src/10_blit_keyed.c")
add_executable(r96_11_blit_perf "src/11_blit_perf.c")
add_executable(r96_12_hello_dos "src/12_hello_dos.c")
add_executable(r96_13_blit_region "src/13_blit_region.c")
add_executable(r96_14_fonts "src/14_fonts.c")
add_executable(r96_15_font_fun "src/15_font_fun.c")
add_executable(r96_16_naive_line "src/16_naive_line.c")
add_executable(r96_17_fixed_point_line "src/17_fixed_point_line.c")
add_executable(r96_18_line_perf "src/18_line_perf.c")
add_executable(r96_19_lissajous "src/19_lissajous.c")

add_custom_target(r96_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:r96_00_basic_window>/assets
)

add_custom_target(r96_web_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/web
    $<TARGET_FILE_DIR:r96_00_basic_window>
)

get_property(targets DIRECTORY "${_dir}" PROPERTY BUILDSYSTEM_TARGETS)
list(REMOVE_ITEM targets minifb r96 r96_assets r96_web_assets)
foreach(target IN LISTS targets)
    target_link_libraries(${target} LINK_PUBLIC minifb r96)    
    add_dependencies(${target} r96_assets)
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
