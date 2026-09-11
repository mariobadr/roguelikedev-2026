/**
 * @file camera.h
 */
#ifndef GINC_ROGUELIKE_CAMERA_H
#define GINC_ROGUELIKE_CAMERA_H

#include <SDL3/SDL_rect.h>

void
rl_centre_camera_on(SDL_Rect* camera,
                    SDL_Point origin,
                    int level_width,
                    int level_height);

SDL_Rect
rl_visible_world(SDL_Rect const* camera, int level_width, int level_height);

#endif // GINC_ROGUELIKE_CAMERA_H
