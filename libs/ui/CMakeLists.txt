###############################################################################
# UI library
###############################################################################
add_library(ginc_ui STATIC
    anchor.c
    anchor.h
    layout.c
    layout.h
    list.c
    list.h
    rectcut.h
    str_wrap.c
    str_wrap.h
)

add_library(ginc::ui ALIAS ginc_ui)

ginc_configure_target(ginc_ui)

target_include_directories(ginc_ui
    PUBLIC
        "${PROJECT_SOURCE_DIR}"
)

target_link_libraries(ginc_ui
    PUBLIC
        SDL3::SDL3
)
