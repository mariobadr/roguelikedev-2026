/**
 * @file map_view.h
 */
#ifndef GINC_ROGUELIKE_MAP_VIEW_H
#define GINC_ROGUELIKE_MAP_VIEW_H

#include <SDL3/SDL_rect.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_font;
struct rl_fov;
struct rl_world;

/**
 * Measure a map viewport in whole cells.
 *
 * @return the number of cells that fit horizontally (x) and vertically (y).
 */
SDL_Point
rl_map_viewport_size(SDL_FRect const* viewport);

/**
 * Find the map cell under a position given in logical pixels.
 *
 * @return true if the position lies within the viewport, false otherwise.
 */
bool
rl_map_cell_from_pixels(SDL_FRect const* viewport,
                        SDL_FPoint at,
                        SDL_Point* cell);

void
rl_draw_map(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_FRect const* viewport,
            struct rl_world const* world,
            struct rl_fov const* fov);

#endif // GINC_ROGUELIKE_MAP_VIEW_H
