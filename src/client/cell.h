/**
 * @file cell.h
 */
#ifndef GINC_ROGUELIKE_CELL_H
#define GINC_ROGUELIKE_CELL_H

#include <SDL3/SDL_rect.h>

/**
 * @return the width of a cell in logical pixels.
 */
int
rl_cell_width(void);

/**
 * @return the height of a cell in logical pixels.
 */
int
rl_cell_height(void);

/**
 * @return the position in logical pixels.
 */
SDL_FPoint
rl_cell_point_to_pixels(SDL_Point const* point);

/**
 * @return the cell containing the given position in logical pixels.
 */
SDL_Point
rl_cell_point_from_pixels(SDL_FPoint const* pixels);

/**
 * @return the rectangle in logical pixels.
 */
SDL_FRect
rl_cell_rect_to_pixels(SDL_Rect const* rect);

#endif // GINC_ROGUELIKE_CELL_H
