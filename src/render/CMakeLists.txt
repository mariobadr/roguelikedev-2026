###############################################################################
# Render library
###############################################################################
add_library(rl_render STATIC
    border.c
    border.h
    graphics.c
    graphics.h
    lighting.c
    lighting.h
    world.c
    world.h
    palette.h
)

add_library(rl::render ALIAS rl_render)

rl_configure_target(rl_render)

target_include_directories(rl_render
    PUBLIC
        "${RL_SOURCE_DIR}"
)

target_link_libraries(rl_render
    PUBLIC
        rl::game
        ginc::container
        ginc::cp437
        ginc::graphics
        SDL3::SDL3
)
