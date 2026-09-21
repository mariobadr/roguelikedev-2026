###############################################################################
# Games in C - common libraries
###############################################################################
project(
    ginc
    LANGUAGES C
)


###############################################################################
# Common configuration options / properties
###############################################################################
function(ginc_configure_target target)
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
add_subdirectory(core)
add_subdirectory(container)
add_subdirectory(cp437)
add_subdirectory(graphics)
add_subdirectory(input)
add_subdirectory(ui)
