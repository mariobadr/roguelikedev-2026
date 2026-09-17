/**
 * @file camera.h
 */
#ifndef GINC_ROGUELIKE_CAMERA_H
#define GINC_ROGUELIKE_CAMERA_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

struct rl_camera
{
  SDL_Rect bounds;
  SDL_FRect viewport;
  int cell_width;
  int cell_height;
};

void
rl_init_camera(struct rl_camera* camera,
               SDL_FRect const* viewport,
               int cell_width,
               int cell_height);

void
rl_centre_camera_on(struct rl_camera* camera,
                    SDL_Point origin,
                    int level_width,
                    int level_height);

SDL_Rect
rl_visible_world(struct rl_camera const* camera,
                 int level_width,
                 int level_height);

SDL_FPoint
rl_viewport_origin(struct rl_camera const* camera);

SDL_FPoint
rl_world_to_screen(struct rl_camera const* camera, SDL_Point cell);

SDL_Point
rl_world_to_grid(struct rl_camera const* camera, SDL_Point world);

bool
rl_get_world_cell(struct rl_camera const* camera,
                   SDL_FPoint pos,
                   SDL_Point* out);

#endif // GINC_ROGUELIKE_CAMERA_H
