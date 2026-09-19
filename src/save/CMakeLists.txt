###############################################################################
# Save library
###############################################################################
add_library(rl_save STATIC
    actor.c
    actor.h
    context.c
    context.h
    format.c
    format.h
    game.c
    game.h
    item.c
    item.h
    level.c
    level.h
    result.h
    save.c
    save.h
    world.c
    world.h
)

add_library(rl::save ALIAS rl_save)

rl_configure_target(rl_save)

target_include_directories(rl_save
    PUBLIC
        "${RL_SOURCE_DIR}"
)

target_link_libraries(rl_save
    PUBLIC
        rl::game
        ginc::container
        SDL3::SDL3
)
