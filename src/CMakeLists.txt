###############################################################################
# Source files
###############################################################################
set(GAME_SOURCES
    client/client.c
    client/client.h
    client/controls.c
    client/controls.h
    client/game_log.c
    client/game_log.h
    client/graphics.c
    client/graphics.h
    client/input.c
    client/input.h
    client/lighting.c
    client/lighting.h
    client/palette.h
    client/render.c
    client/render.h
    client/resources.c
    client/resources.h
    client/text.h
    client/ui.h
    client/view.c
    client/view.h
    container/alist.c
    container/alist.h
    container/array.h
    container/grid.h
    game/action.h
    game/actor.c
    game/actor.h
    game/command.c
    game/command.h
    game/event.h
    game/fov.c
    game/fov.h
    game/game_state.c
    game/game_state.h
    game/item.c
    game/item.h
    game/pathfinding.c
    game/pathfinding.h
    game/spawn.c
    game/spawn.h
    game/tile_map.c
    game/tile_map.h
    game/tile.c
    game/tile.h
    game/world.c
    game/world.h
    procgen/bsp.c
    procgen/bsp.h
    procgen/layout.c
    procgen/layout.h
    procgen/rand.c
    procgen/rand.h
    main.c
)


###############################################################################
# Game target
###############################################################################
set(GAME_TARGET ${PROJECT_NAME})
add_executable(${GAME_TARGET} ${GAME_SOURCES})

target_include_directories(${GAME_TARGET}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${GAME_TARGET}
    PRIVATE
        SDL3::SDL3
)

target_compile_options(${GAME_TARGET}
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/W4 /WX>
        $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Werror>
)

set_target_properties(${GAME_TARGET}
    PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
)


###############################################################################
# Platform-specific build rules
###############################################################################
if(WIN32)
    # Copy SDL3's DLL
    add_custom_command(
        TARGET ${GAME_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:${GAME_TARGET}>
        VERBATIM)

    # Copy assets next to the executable as "res"
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
        add_custom_command(
            TARGET ${GAME_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
            "${CMAKE_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${GAME_TARGET}>/res"
            VERBATIM)
    endif()
elseif(EMSCRIPTEN)
    set_target_properties(${GAME_TARGET} PROPERTIES
        OUTPUT_NAME "index"
        SUFFIX ".html"
    )

    target_link_options(${GAME_TARGET} PRIVATE
        "--shell-file" "${CMAKE_SOURCE_DIR}/emscripten/shell.html"
        "-sALLOW_MEMORY_GROWTH=1"
    )

    # Preload assets into the virtual filesystem as "res"
    if(EXISTS "${CMAKE_SOURCE_DIR}/assets")
        target_link_options(${GAME_TARGET} PRIVATE
            "--preload-file" "${CMAKE_SOURCE_DIR}/assets@res"
        ) 
    endif()
endif()
