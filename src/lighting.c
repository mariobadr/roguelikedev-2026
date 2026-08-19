#include "lighting.h"

float
rl_calculate_brightness(SDL_Point source, SDL_Point tile, float radius)
{
  float const dx = (float)(tile.x - source.x);
  float const dy = (float)(tile.y - source.y);
  float const distance = SDL_sqrtf(dx * dx + dy * dy);

  return SDL_clamp(1.0f - distance / radius, 0.0f, 1.0f);
}

SDL_FColor
rl_apply_brightness(SDL_FColor brightest, SDL_FColor darkest, float brightness)
{
  brightness = SDL_clamp(brightness, -1.0f, 1.0f);

  return (SDL_FColor){
    .r = darkest.r + (brightest.r - darkest.r) * brightness,
    .g = darkest.g + (brightest.g - darkest.g) * brightness,
    .b = darkest.b + (brightest.b - darkest.b) * brightness,
    .a = brightest.a,
  };
}