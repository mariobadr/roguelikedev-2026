###############################################################################
# Graphics library
###############################################################################
add_library(ginc_graphics STATIC
    console.c
    console.h
    tileset.c
    tileset.h
)

add_library(ginc::graphics ALIAS ginc_graphics)

ginc_configure_target(ginc_graphics)

target_include_directories(ginc_graphics
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_graphics
    PUBLIC
        ginc::container
        SDL3::SDL3
)
