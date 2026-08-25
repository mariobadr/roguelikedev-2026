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
rl_lerp_colour(SDL_FColor a, SDL_FColor b, float alpha)
{
  alpha = SDL_clamp(alpha, -1.0f, 1.0f);

  return (SDL_FColor){
    .r = b.r + (a.r - b.r) * alpha,
    .g = b.g + (a.g - b.g) * alpha,
    .b = b.b + (a.b - b.b) * alpha,
    .a = a.a,
  };
}

float
rl_lerp_float(float a, float b, float alpha)
{
  alpha = SDL_clamp(alpha, -1.0f, 1.0f);

  return b + (a - b) * alpha;
}
