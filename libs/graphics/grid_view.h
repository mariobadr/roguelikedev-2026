/**
 * @file grid_view.h
 */
#ifndef GINC_GRAPHICS_GRID_VIEW_H
#define GINC_GRAPHICS_GRID_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_camera;

/** 
 * A grid of fixed-size cells laid over world space.
 */
struct gfx_grid_view
{
  /** The width of one cell, in world units. */
  int cell_width;
  /** The height of one cell, in world units. */
  int cell_height;
};

/**
 * @return the world position of the top-left corner of cell
 */
SDL_FPoint
gfx_cell_to_world(struct gfx_grid_view const* view, SDL_Point cell);

/**
 * @return the cell containing the world position
 */
SDL_Point
gfx_world_to_cell(struct gfx_grid_view const* view, SDL_FPoint world);

/**
 * @return the screen position of the top-left corner of cell
 */
SDL_FPoint
gfx_cell_to_screen(struct gfx_grid_view const* view,
                   struct gfx_camera const* camera,
                   SDL_Point cell);

/**
 * @param out receives the cell under screen
 * 
 * @return false if screen is outside the camera viewport
 */
bool
gfx_screen_to_cell(struct gfx_grid_view const* view,
                   struct gfx_camera const* camera,
                   SDL_FPoint screen,
                   SDL_Point* out);

/**
 * @return the cells the camera overlaps, which may extend past any map
 */
SDL_Rect
gfx_grid_view_bounds(struct gfx_grid_view const* view,
                     struct gfx_camera const* camera);

/**
 * @return the cells the camera overlaps within a map of columns by rows
 */
SDL_Rect
gfx_visible_cells(struct gfx_grid_view const* view,
                  struct gfx_camera const* camera,
                  int columns,
                  int rows);

#endif // GINC_GRAPHICS_GRID_VIEW_H
