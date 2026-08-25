#include "graphics.h"

#include "entity.h"
#include "palette.h"

struct tile_gfx_desc
{
  Uint8 glyph;
  SDL_FColor const* fg_colour;
  Uint8 fg_index;
  SDL_FColor const* bg_colour;
  Uint8 bg_index;
};

static struct tile_gfx_desc const tile_gfx_table[] = {
  [RL_TILE_WALL] = {
    .glyph = '#',
    .fg_colour = RL_COLOUR_GRAY,
    .fg_index = 4,
    .bg_colour = NULL,
  },
  [RL_TILE_FLOOR] = {
    .glyph = ' ',
    .fg_colour = NULL,
    .bg_colour = RL_COLOUR_GRAY,
    .bg_index = 9,
  },
};

struct rl_gfx_tile
rl_get_tile_gfx(enum rl_tile tile)
{
  struct tile_gfx_desc const* desc = &tile_gfx_table[tile];

  struct rl_gfx_tile gfx = { 0 };
  gfx.glyph = desc->glyph;

  if (desc->fg_colour == NULL) {
    gfx.fg = RL_COLOUR_BLACK;
  } else {
    gfx.fg = desc->fg_colour[desc->fg_index];
  }

  if (desc->bg_colour == NULL) {
    gfx.bg = RL_COLOUR_BLACK;
  } else {
    gfx.bg = desc->bg_colour[desc->bg_index];
  }

  return gfx;
}

static struct tile_gfx_desc const entity_gfx_table[] = {
  [RL_ENTITY_ROGUE] = {
    .glyph = '@',
    .fg_colour = RL_COLOUR_GRAY,
    .fg_index = 0,
    .bg_colour = NULL,
  },
  [RL_ENTITY_RAT] = {
    .glyph = 'r',
    .fg_colour = RL_COLOUR_GREEN,
    .fg_index = 9,
    .bg_colour = NULL,
  },
};

struct rl_gfx_tile
rl_get_entity_gfx(struct rl_entity const* entity)
{
  struct tile_gfx_desc const* desc = &entity_gfx_table[entity->type];

  struct rl_gfx_tile gfx = { 0 };
  gfx.glyph = desc->glyph;

  if (desc->fg_colour == NULL) {
    gfx.fg = RL_COLOUR_BLACK;
  } else {
    gfx.fg = desc->fg_colour[desc->fg_index];
  }

  if (desc->bg_colour == NULL) {
    gfx.bg = RL_COLOUR_BLACK;
  } else {
    gfx.bg = desc->bg_colour[desc->bg_index];
  }

  return gfx;
}
