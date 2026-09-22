###############################################################################
# Spatial library
###############################################################################
add_library(ginc_spatial STATIC
    bsp.c
    bsp.h
    dijkstra.c
    dijkstra.h
    fov.c
    fov.h
)

add_library(ginc::spatial ALIAS ginc_spatial)

ginc_configure_target(ginc_spatial)

target_include_directories(ginc_spatial
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_spatial
    PUBLIC
        ginc::container
        ginc::core
        SDL3::SDL3
)
