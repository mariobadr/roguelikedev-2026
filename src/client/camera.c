#include "camera.h"

#include <SDL3/SDL_stdinc.h>

void
rl_centre_camera_on(SDL_Rect* camera,
                    SDL_Point origin,
                    int level_width,
                    int level_height)
{
  int const max_x = SDL_max(0, level_width - camera->w);
  int const max_y = SDL_max(0, level_height - camera->h);

  camera->x = SDL_clamp(origin.x - camera->w / 2, 0, max_x);
  camera->y = SDL_clamp(origin.y - camera->h / 2, 0, max_y);
}

SDL_Rect
rl_visible_world(SDL_Rect const* camera, int level_width, int level_height)
{
  SDL_Rect const level_rect = { 0, 0, level_width, level_height };

  SDL_Rect visible = { 0 };
  SDL_GetRectIntersection(camera, &level_rect, &visible);

  return visible;
}
