#include "resources.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>

/**
 * Convert all pixels in the DINOBYTE font to pure white.
 *
 * This function exists for some silly reasons: (1) I don't want to bundle the
 * font with the repo, and (2) the font is in a beige colour. Since everything
 * is beige, SDL_SetTextureColorMod tints won't work how I want them to. So...
 * we convert all pixels to white.
 */
static void
whiten_dbyte_font(SDL_Surface* surface)
{
  SDL_assert(SDL_BYTESPERPIXEL(surface->format) == 4);

  SDL_PixelFormatDetails const* format =
    SDL_GetPixelFormatDetails(surface->format);

  SDL_LockSurface(surface);
  Uint32* pixels = (Uint32*)surface->pixels;
  int const pixel_count = surface->w * surface->h;

  for (int i = 0; i < pixel_count; ++i) {
    // leave alpha's bits untouched
    pixels[i] |= (format->Rmask | format->Gmask | format->Bmask);
  }
  SDL_UnlockSurface(surface);
}

/**
 * Loads the DINOBYTE font.
 *
 * @param base_path Directory containing the res/ folder.
 * @param renderer  Renderer used to create the texture.
 *
 * @return The loaded texture, or NULL on failure.
 */
static SDL_Texture*
load_dbyte_font(char const* base_path, SDL_Renderer* renderer)
{
  char* path = NULL;
  if (SDL_asprintf(&path, "%sres/dbyte_1x.png", base_path) < 0) {
    SDL_Log("SDL_asprintf failed: %s", SDL_GetError());
    return NULL;
  }

  SDL_Surface* surface = SDL_LoadPNG(path);
  SDL_free(path);
  if (surface == NULL) {
    SDL_Log("SDL_LoadPNG failed: %s", SDL_GetError());
    return NULL;
  }

  // changes the actual pixel values
  whiten_dbyte_font(surface);

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

  resources->font = load_dbyte_font(base_path, renderer);
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
