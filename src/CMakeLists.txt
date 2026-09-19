###############################################################################
# CMake functions
###############################################################################
function(rl_configure_target target)
    set_target_properties(${target} PROPERTIES
        C_STANDARD 11
        C_STANDARD_REQUIRED ON
        COMPILE_WARNING_AS_ERROR ON
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall)
    endif()
endfunction()


###############################################################################
# Libraries
###############################################################################
set(RL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory(game)
add_subdirectory(save)
add_subdirectory(render)
add_subdirectory(client)


###############################################################################
# Game target
###############################################################################
set(GAME_TARGET ${PROJECT_NAME})
add_executable(${GAME_TARGET} main.c)

target_link_libraries(${GAME_TARGET}
    PRIVATE
        rl::client
        ginc::input
        SDL3::SDL3
)

rl_configure_target(${GAME_TARGET})


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
