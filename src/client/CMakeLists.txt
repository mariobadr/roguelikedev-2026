###############################################################################
# Client library
###############################################################################
add_library(rl_client STATIC
    screen/game_over.c
    screen/game_over.h
    screen/gameplay.c
    screen/gameplay.h
    screen/main_menu.c
    screen/main_menu.h
    screen/save_files.c
    screen/save_files.h
    view/in_sight.c
    view/in_sight.h
    view/inventory.c
    view/inventory.h
    view/log.c
    view/log.h
    view/map.c
    view/map.h
    action.c
    action.h
    camera.c
    camera.h
    client.c
    client.h
    controls.c
    controls.h
    font.c
    font.h
    game_log.c
    game_log.h
    graphics.c
    graphics.h
    lighting.c
    lighting.h
    palette.h
    render.c
    render.h
    ribbon.c
    ribbon.h
    run.c
    run.h
    save.c
    save.h
    save_format.c
    save_format.h
    screen.h
    text.c
    text.h
    view.h
)

add_library(rl::client ALIAS rl_client)

rl_configure_target(rl_client)

target_include_directories(rl_client
    PUBLIC
        "${RL_SOURCE_DIR}"
)

target_link_libraries(rl_client
    PUBLIC
        rl::game
        ginc::core
        ginc::container
        ginc::graphics
        SDL3::SDL3
    PRIVATE
        ginc::input
        ginc::ui
)
