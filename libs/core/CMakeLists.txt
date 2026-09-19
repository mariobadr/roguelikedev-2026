###############################################################################
# Core library
###############################################################################
add_library(ginc_core STATIC
    handle.h
    rand.c
    rand.h
    string.h
)

add_library(ginc::core ALIAS ginc_core)

ginc_configure_target(ginc_core)

target_include_directories(ginc_core
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_core
    PUBLIC
        SDL3::SDL3
)
