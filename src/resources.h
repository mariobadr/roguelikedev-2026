/**
 * @file resources.h
 */
#ifndef GINC_ROGUELIKE_RESOURCES_H
#define GINC_ROGUELIKE_RESOURCES_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

/**
 * Resources used by the game.
 */
struct rl_resources
{
  /** Texture used for the bitmap font. */
  SDL_Texture* font;
};

/**
 * Loads the resources required to render the game.
 *
 * @param resources Resources to populate.
 * @param renderer  Renderer used to create the textures.
 *
 * @return true if all resources were loaded successfully, false otherwise.
 */
bool
rl_load_resources(struct rl_resources* resources, SDL_Renderer* renderer);

/**
 * Destroys previously loaded resources.
 *
 * @param resources Resources to destroy.
 */
void
rl_destroy_resources(struct rl_resources* resources);

#endif // GINC_ROGUELIKE_RESOURCES_H
