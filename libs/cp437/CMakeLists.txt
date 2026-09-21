###############################################################################
# cp437 library
###############################################################################
add_library(ginc_cp437 STATIC
    border.c
    border.h
    character.h
)

add_library(ginc::cp437 ALIAS ginc_cp437)

ginc_configure_target(ginc_cp437)

target_include_directories(ginc_cp437
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_cp437
    PUBLIC
        ginc::container
        SDL3::SDL3
)
