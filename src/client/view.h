/**
 * @file view.h
 */
#ifndef GINC_ROGUELIKE_VIEW_H
#define GINC_ROGUELIKE_VIEW_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_client;

void
rl_render_game(SDL_Renderer* renderer, struct rl_client* client);

#endif // GINC_ROGUELIKE_VIEW_H