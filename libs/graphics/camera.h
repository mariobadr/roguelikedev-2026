/**
 * @file camera.h
 */
#ifndef GINC_GRAPHICS_CAMERA_H
#define GINC_GRAPHICS_CAMERA_H

#include <SDL3/SDL_rect.h>

/**
 * A window onto world space, drawn into a region of the screen.
 */
struct gfx_camera
{
  /** The region of the screen the camera draws into, in pixels. */
  SDL_FRect viewport;
  /** The world position shown at the top-left of the viewport. */
  SDL_FPoint position;
};

/**
 * Initialise camera to draw into viewport, positioned at the world origin.
 */
void
gfx_init_camera(struct gfx_camera* camera, SDL_FRect const* viewport);

/**
 * @return the screen position of the viewport's top-left corner, rounded down
 * to a whole pixel.
 */
SDL_FPoint
gfx_viewport_origin(struct gfx_camera const* camera);

/**
 * @return the region of world space the camera sees.
 */
SDL_FRect
gfx_camera_world_bounds(struct gfx_camera const* camera);

/**
 * @return the screen position of the world position.
 */
SDL_FPoint
gfx_world_to_screen(struct gfx_camera const* camera, SDL_FPoint world);

/**
 * @return the world position at the screen position.
 */
SDL_FPoint
gfx_screen_to_world(struct gfx_camera const* camera, SDL_FPoint screen);

#endif // GINC_GRAPHICS_CAMERA_H
