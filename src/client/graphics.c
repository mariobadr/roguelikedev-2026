#include "graphics.h"

#include "game/actor.h"
#include "game/item.h"

#include "palette.h"

struct tile_gfx_desc
{
  Uint8 glyph;
  SDL_FColor const* fg_colour;
  Uint8 fg_index;
  SDL_FColor const* bg_colour;
  Uint8 bg_index;
};

static struct rl_gfx_tile
lookup_gfx_tile(struct tile_gfx_desc const* table, int index)
{
  struct tile_gfx_desc const* desc = &table[index];

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
  return lookup_gfx_tile(tile_gfx_table, tile);
}

static struct tile_gfx_desc const item_gfx_table[] = {
  [RL_ITEM_POTION_HEALTH_MINOR] = {
    .glyph = '!',
    .fg_colour = RL_COLOUR_YELLOW,
    .fg_index = 2,
    .bg_colour = NULL,
  },
};

struct rl_gfx_tile
rl_get_item_gfx(struct rl_item const* item)
{
  return lookup_gfx_tile(item_gfx_table, item->itype);
}

static struct tile_gfx_desc const actor_gfx_table[] = {
  [RL_ACTOR_ROGUE] = {
    .glyph = '@',
    .fg_colour = RL_COLOUR_GRAY,
    .fg_index = 0,
    .bg_colour = NULL,
  },
  [RL_ACTOR_RAT] = {
    .glyph = 'r',
    .fg_colour = RL_COLOUR_ORANGE,
    .fg_index = 9,
    .bg_colour = NULL,
  },
};

struct rl_gfx_tile
rl_get_actor_gfx(struct rl_actor const* actor)
{
  return lookup_gfx_tile(actor_gfx_table, actor->type);
}

struct rl_gfx_tile
rl_get_text_gfx(enum rl_text_style style)
{
  struct rl_gfx_tile gfx = { 0 };
  gfx.fg = RL_COLOUR_GRAY[5];
  gfx.bg = RL_COLOUR_BLACK;

  switch (style) {
    case RL_TEXT_PLAYER:
      gfx.fg = RL_COLOUR_GRAY[0];
      break;
    case RL_TEXT_ENEMY:
      gfx.fg = RL_COLOUR_ORANGE[9];
      break;
    case RL_TEXT_ITEM:
      gfx.fg = RL_COLOUR_YELLOW[2];
      break;
    case RL_TEXT_NORMAL:
    default:
      break;
  }

  return gfx;
}
