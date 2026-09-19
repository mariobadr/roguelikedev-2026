#include "camera.h"

#include <SDL3/SDL_stdinc.h>

void
gfx_init_camera(struct gfx_camera* camera, SDL_FRect const* viewport)
{
  camera->viewport = *viewport;
  camera->position = (SDL_FPoint){ 0 };
}

SDL_FPoint
gfx_viewport_origin(struct gfx_camera const* camera)
{
  return (SDL_FPoint){ SDL_floorf(camera->viewport.x),
                       SDL_floorf(camera->viewport.y) };
}

SDL_FRect
gfx_camera_world_bounds(struct gfx_camera const* camera)
{
  return (SDL_FRect){ camera->position.x,
                      camera->position.y,
                      camera->viewport.w,
                      camera->viewport.h };
}

SDL_FPoint
gfx_world_to_screen(struct gfx_camera const* camera, SDL_FPoint world)
{
  SDL_FPoint const origin = gfx_viewport_origin(camera);
  return (SDL_FPoint){ world.x - camera->position.x + origin.x,
                       world.y - camera->position.y + origin.y };
}

SDL_FPoint
gfx_screen_to_world(struct gfx_camera const* camera, SDL_FPoint screen)
{
  SDL_FPoint const origin = gfx_viewport_origin(camera);
  return (SDL_FPoint){ screen.x - origin.x + camera->position.x,
                       screen.y - origin.y + camera->position.y };
}
