###############################################################################
# Container library
###############################################################################
add_library(ginc_container STATIC
    alist.c
    alist.h
    array.h
    grid.h
    pool.c
    pool.h
)

add_library(ginc::container ALIAS ginc_container)

ginc_configure_target(ginc_container)

target_include_directories(ginc_container
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_container
    PUBLIC
        ginc::core
        SDL3::SDL3
)
