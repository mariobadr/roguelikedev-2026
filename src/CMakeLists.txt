###############################################################################
# Source files
###############################################################################
set(GAME_SOURCES
    action.h
    game.c
    game.h
    input.c
    input.h
    main.c
    render.c
    render.h
    resources.c
    resources.h
)


###############################################################################
# Game target
###############################################################################
set(GAME_TARGET ${PROJECT_NAME})
add_executable(${GAME_TARGET} ${GAME_SOURCES})

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
