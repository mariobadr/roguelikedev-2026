###############################################################################
# Game library
###############################################################################
add_library(rl_game STATIC
    serialize/actor.c
    serialize/actor.h
    serialize/context.c
    serialize/context.h
    serialize/game.c
    serialize/game.h
    serialize/item.c
    serialize/item.h
    serialize/level.c
    serialize/level.h
    serialize/result.h
    serialize/world.c
    serialize/world.h
    actor.c
    actor.h
    actor_def.c
    actor_def.h
    ai.c
    ai.h
    bsp.c
    bsp.h
    combat.c
    combat.h
    command.c
    command.h
    event.h
    fov.c
    fov.h
    game.c
    game.h
    generate.c
    generate.h
    handles.h
    item.c
    item.h
    item_def.c
    item_def.h
    layout.c
    layout.h
    level.c
    level.h
    mechanics.c
    mechanics.h
    pathfinding.c
    pathfinding.h
    spawn.c
    spawn.h
    targeting.c
    targeting.h
    tile.c
    tile.h
    world.c
    world.h
)

add_library(rl::game ALIAS rl_game)

rl_configure_target(rl_game)

target_include_directories(rl_game
    PUBLIC
        "${RL_SOURCE_DIR}"
)

target_link_libraries(rl_game
    PUBLIC
        ginc::core
        ginc::container
        SDL3::SDL3
)
