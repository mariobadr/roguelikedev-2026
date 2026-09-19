#include "camera.h"

#include <SDL3/SDL_stdinc.h>

void
rl_init_camera(struct rl_camera* camera,
               SDL_FRect const* viewport,
               int cell_width,
               int cell_height)
{
  camera->viewport = *viewport;
  camera->cell_width = cell_width;
  camera->cell_height = cell_height;

  camera->bounds.x = 0;
  camera->bounds.y = 0;
  camera->bounds.w = (int)viewport->w / cell_width;
  camera->bounds.h = (int)viewport->h / cell_height;
}

void
rl_centre_camera_on(struct rl_camera* camera,
                    SDL_Point origin,
                    int level_width,
                    int level_height)
{
  int const max_x = SDL_max(0, level_width - camera->bounds.w);
  int const max_y = SDL_max(0, level_height - camera->bounds.h);

  camera->bounds.x = SDL_clamp(origin.x - camera->bounds.w / 2, 0, max_x);
  camera->bounds.y = SDL_clamp(origin.y - camera->bounds.h / 2, 0, max_y);
}

SDL_Rect
rl_visible_world(struct rl_camera const* camera,
                 int level_width,
                 int level_height)
{
  SDL_Rect const level_rect = { 0, 0, level_width, level_height };

  SDL_Rect visible = { 0 };
  SDL_GetRectIntersection(&camera->bounds, &level_rect, &visible);

  return visible;
}

SDL_FPoint
rl_viewport_origin(struct rl_camera const* camera)
{
  SDL_FPoint origin = { 0 };
  origin.x = SDL_floorf(camera->viewport.x);
  origin.y = SDL_floorf(camera->viewport.y);

  return origin;
}

SDL_FPoint
rl_world_to_screen(struct rl_camera const* camera, SDL_Point cell)
{
  SDL_FPoint const origin = rl_viewport_origin(camera);
  SDL_Point const local = rl_world_to_grid(camera, cell);

  SDL_FPoint pixels = { 0 };
  pixels.x = origin.x + (float)(local.x * camera->cell_width);
  pixels.y = origin.y + (float)(local.y * camera->cell_height);

  return pixels;
}

SDL_Point
rl_world_to_grid(struct rl_camera const* camera, SDL_Point world)
{
  SDL_Point local = { 0 };
  local.x = world.x - camera->bounds.x;
  local.y = world.y - camera->bounds.y;

  return local;
}

bool
rl_get_world_cell(struct rl_camera const* camera,
                  SDL_FPoint pos,
                  SDL_Point* out)
{
  if (!SDL_PointInRectFloat(&pos, &camera->viewport)) {
    return false;
  }

  SDL_FPoint const origin = rl_viewport_origin(camera);

  SDL_Point world = { 0 };
  world.x = (int)SDL_floorf((pos.x - origin.x) / (float)camera->cell_width) +
            camera->bounds.x;
  world.y = (int)SDL_floorf((pos.y - origin.y) / (float)camera->cell_height) +
            camera->bounds.y;

  *out = world;
  return true;
}
