/**
 * @file map.h
 */
#ifndef GINC_ROGUELIKE_MAP_VIEW_H
#define GINC_ROGUELIKE_MAP_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_fov;
struct rl_view;
struct rl_world;

bool
rl_alloc_map_view(struct rl_view* view,
                  struct rl_world const* world,
                  struct rl_fov const* fov,
                  SDL_FRect const* viewport,
                  int cell_width,
                  int cell_height);

#endif // GINC_ROGUELIKE_MAP_VIEW_H
