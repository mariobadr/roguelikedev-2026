/**
 * @file map.h
 */
#ifndef GINC_ROGUELIKE_MAP_VIEW_H
#define GINC_ROGUELIKE_MAP_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_font;
struct rl_fov;
struct rl_world;

struct rl_map_view
{
  SDL_FRect viewport;
  int cell_width;
  int cell_height;
};

void
rl_init_map_view(struct rl_map_view* map,
                 SDL_FRect const* viewport,
                 int cell_width,
                 int cell_height);

void
rl_map_view_size(struct rl_map_view const* map, int* width, int* height);

bool
rl_map_view_cell_at(struct rl_map_view const* map,
                    SDL_FPoint pos,
                    SDL_Point* cell);

void
rl_draw_map_view(struct rl_map_view const* map,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_world const* world,
                 struct rl_fov const* fov);

#endif // GINC_ROGUELIKE_MAP_VIEW_H
