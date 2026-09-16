###############################################################################
# Input library
###############################################################################
add_library(ginc_input STATIC
    input.c
    input.h
    input_event.c
    input_event.h
)

add_library(ginc::input ALIAS ginc_input)

ginc_configure_target(ginc_input)

target_include_directories(ginc_input
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_input
    PUBLIC
        SDL3::SDL3
)
