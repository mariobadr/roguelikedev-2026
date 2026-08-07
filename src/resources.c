#include "resources.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

/**
 * Loads a single PNG resource as a texture.
 *
 * @param base_path Directory containing the res/ folder.
 * @param filename  File name within res/.
 * @param renderer  Renderer used to create the texture.
 *
 * @return The loaded texture, or NULL on failure.
 */
static SDL_Texture*
load_texture(char const* base_path,
             char const* filename,
             SDL_Renderer* renderer)
{
  char* path = NULL;
  if (SDL_asprintf(&path, "%sres/%s", base_path, filename) < 0) {
    SDL_Log("SDL_asprintf failed: %s", SDL_GetError());
    return NULL;
  }

  SDL_Surface* surface = SDL_LoadPNG(path);
  SDL_free(path);
  if (surface == NULL) {
    SDL_Log("SDL_LoadPNG failed: %s", SDL_GetError());
    return NULL;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_DestroySurface(surface);
  if (texture == NULL) {
    SDL_Log("SDL_CreateTextureFromSurface failed: %s", SDL_GetError());
    return NULL;
  }

  return texture;
}

bool
rl_load_resources(struct rl_resources* resources, SDL_Renderer* renderer)
{
  char const* base_path = SDL_GetBasePath();
  if (base_path == NULL) {
    SDL_Log("SDL_GetBasePath failed: %s", SDL_GetError());
    return false;
  }

  resources->font = load_texture(base_path, "dbyte_1x.png", renderer);
  if (resources->font == NULL) {
    return false;
  }

  return true;
}

void
rl_destroy_resources(struct rl_resources* resources)
{
  if (resources == NULL) {
    return;
  }

  SDL_DestroyTexture(resources->font);
}
