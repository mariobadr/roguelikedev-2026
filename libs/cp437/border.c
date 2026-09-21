#include "border.h"

#include "character.h"

/**
 * The neighbouring cells connected to a border cell.
 */
enum cp437_border_connection
{
  BORDER_NORTH = 1 << 0,
  BORDER_EAST = 1 << 1,
  BORDER_SOUTH = 1 << 2,
  BORDER_WEST = 1 << 3,
};

/**
 * CP437 glyph indices indexed by horizontal/vertical doubling and connections.
 *
 * CP437 has no end caps, so one-connection cells use straight strokes.
 */
static Uint8 const BORDER_GLYPHS[2][2][16] = {
  [0][0] = {
    [BORDER_EAST] = CP437_BOX_HORIZONTAL,
    [BORDER_WEST] = CP437_BOX_HORIZONTAL,
    [BORDER_NORTH] = CP437_BOX_VERTICAL,
    [BORDER_SOUTH] = CP437_BOX_VERTICAL,
    [BORDER_EAST | BORDER_WEST] = CP437_BOX_HORIZONTAL,
    [BORDER_NORTH | BORDER_SOUTH] = CP437_BOX_VERTICAL,
    [BORDER_EAST | BORDER_SOUTH] = CP437_BOX_TOP_LEFT,
    [BORDER_SOUTH | BORDER_WEST] = CP437_BOX_TOP_RIGHT,
    [BORDER_NORTH | BORDER_EAST] = CP437_BOX_BOTTOM_LEFT,
    [BORDER_NORTH | BORDER_WEST] = CP437_BOX_BOTTOM_RIGHT,
    [BORDER_EAST | BORDER_SOUTH | BORDER_WEST] = CP437_BOX_TEE_TOP,
    [BORDER_NORTH | BORDER_EAST | BORDER_WEST] = CP437_BOX_TEE_BOTTOM,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH] = CP437_BOX_TEE_LEFT,
    [BORDER_NORTH | BORDER_SOUTH | BORDER_WEST] = CP437_BOX_TEE_RIGHT,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_CROSS,
  },
  [1][1] = {
    [BORDER_EAST] = CP437_BOX_DBL_HORIZONTAL,
    [BORDER_WEST] = CP437_BOX_DBL_HORIZONTAL,
    [BORDER_NORTH] = CP437_BOX_DBL_VERTICAL,
    [BORDER_SOUTH] = CP437_BOX_DBL_VERTICAL,
    [BORDER_EAST | BORDER_WEST] = CP437_BOX_DBL_HORIZONTAL,
    [BORDER_NORTH | BORDER_SOUTH] = CP437_BOX_DBL_VERTICAL,
    [BORDER_EAST | BORDER_SOUTH] = CP437_BOX_DBL_TOP_LEFT,
    [BORDER_SOUTH | BORDER_WEST] = CP437_BOX_DBL_TOP_RIGHT,
    [BORDER_NORTH | BORDER_EAST] = CP437_BOX_DBL_BOTTOM_LEFT,
    [BORDER_NORTH | BORDER_WEST] = CP437_BOX_DBL_BOTTOM_RIGHT,
    [BORDER_EAST | BORDER_SOUTH | BORDER_WEST] = CP437_BOX_DBL_TEE_TOP,
    [BORDER_NORTH | BORDER_EAST | BORDER_WEST] = CP437_BOX_DBL_TEE_BOTTOM,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH] = CP437_BOX_DBL_TEE_LEFT,
    [BORDER_NORTH | BORDER_SOUTH | BORDER_WEST] = CP437_BOX_DBL_TEE_RIGHT,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_DBL_CROSS,
  },
  [1][0] = {
    [BORDER_EAST] = CP437_BOX_DBL_HORIZONTAL,
    [BORDER_WEST] = CP437_BOX_DBL_HORIZONTAL,
    [BORDER_NORTH] = CP437_BOX_VERTICAL,
    [BORDER_SOUTH] = CP437_BOX_VERTICAL,
    [BORDER_EAST | BORDER_WEST] = CP437_BOX_DBL_HORIZONTAL,
    [BORDER_NORTH | BORDER_SOUTH] = CP437_BOX_VERTICAL,
    [BORDER_EAST | BORDER_SOUTH] = CP437_BOX_MIX_TOP_LEFT_DBL_H,
    [BORDER_SOUTH | BORDER_WEST] = CP437_BOX_MIX_TOP_RIGHT_DBL_H,
    [BORDER_NORTH | BORDER_EAST] = CP437_BOX_MIX_BOTTOM_LEFT_DBL_H,
    [BORDER_NORTH | BORDER_WEST] = CP437_BOX_MIX_BOTTOM_RIGHT_DBL_H,
    [BORDER_EAST | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_MIX_TEE_TOP_DBL_H,
    [BORDER_NORTH | BORDER_EAST | BORDER_WEST] =
      CP437_BOX_MIX_TEE_BOTTOM_DBL_H,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH] =
      CP437_BOX_MIX_TEE_LEFT_DBL_H,
    [BORDER_NORTH | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_MIX_TEE_RIGHT_DBL_H,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_MIX_CROSS_DBL_H,
  },
  [0][1] = {
    [BORDER_EAST] = CP437_BOX_HORIZONTAL,
    [BORDER_WEST] = CP437_BOX_HORIZONTAL,
    [BORDER_NORTH] = CP437_BOX_DBL_VERTICAL,
    [BORDER_SOUTH] = CP437_BOX_DBL_VERTICAL,
    [BORDER_EAST | BORDER_WEST] = CP437_BOX_HORIZONTAL,
    [BORDER_NORTH | BORDER_SOUTH] = CP437_BOX_DBL_VERTICAL,
    [BORDER_EAST | BORDER_SOUTH] = CP437_BOX_MIX_TOP_LEFT_DBL_V,
    [BORDER_SOUTH | BORDER_WEST] = CP437_BOX_MIX_TOP_RIGHT_DBL_V,
    [BORDER_NORTH | BORDER_EAST] = CP437_BOX_MIX_BOTTOM_LEFT_DBL_V,
    [BORDER_NORTH | BORDER_WEST] = CP437_BOX_MIX_BOTTOM_RIGHT_DBL_V,
    [BORDER_EAST | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_MIX_TEE_TOP_DBL_V,
    [BORDER_NORTH | BORDER_EAST | BORDER_WEST] =
      CP437_BOX_MIX_TEE_BOTTOM_DBL_V,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH] =
      CP437_BOX_MIX_TEE_LEFT_DBL_V,
    [BORDER_NORTH | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_MIX_TEE_RIGHT_DBL_V,
    [BORDER_NORTH | BORDER_EAST | BORDER_SOUTH | BORDER_WEST] =
      CP437_BOX_MIX_CROSS_DBL_V,
  },
};

struct cp437_border_cell
cp437_box_cell(SDL_Rect const* box,
               struct cp437_border_style const* style,
               int x,
               int y)
{
  struct cp437_border_cell cell = { 0 };

  int const left = box->x;
  int const top = box->y;
  int const right = box->x + box->w - 1;
  int const bottom = box->y + box->h - 1;

  if (x < left || x > right || y < top || y > bottom) {
    return cell;
  }

  if (y == top) {
    cell.horizontal = style->top;
  } else if (y == bottom) {
    cell.horizontal = style->bottom;
  }

  if (x == left) {
    cell.vertical = style->left;
  } else if (x == right) {
    cell.vertical = style->right;
  }

  if (cell.horizontal != CP437_BORDER_NONE) {
    if (x > left) {
      cell.connections |= BORDER_WEST;
    }
    if (x < right) {
      cell.connections |= BORDER_EAST;
    }
  }

  if (cell.vertical != CP437_BORDER_NONE) {
    if (y > top) {
      cell.connections |= BORDER_NORTH;
    }
    if (y < bottom) {
      cell.connections |= BORDER_SOUTH;
    }
  }

  return cell;
}

void
cp437_add_hline(grid(cp437_border) * borders,
                int x,
                int y,
                int length,
                enum cp437_border_weight weight)
{
  if (weight == CP437_BORDER_NONE) {
    return;
  }

  for (int i = 0; i < length; ++i) {
    struct cp437_border_cell* cell = grid_at(borders, x + i, y);

    if (i > 0) {
      cell->connections |= BORDER_WEST;
    }
    if (i + 1 < length) {
      cell->connections |= BORDER_EAST;
    }

    cell->horizontal = SDL_max(cell->horizontal, weight);
  }
}

void
cp437_add_vline(grid(cp437_border) * borders,
                int x,
                int y,
                int length,
                enum cp437_border_weight weight)
{
  if (weight == CP437_BORDER_NONE) {
    return;
  }

  for (int i = 0; i < length; ++i) {
    struct cp437_border_cell* cell = grid_at(borders, x, y + i);

    if (i > 0) {
      cell->connections |= BORDER_NORTH;
    }
    if (i + 1 < length) {
      cell->connections |= BORDER_SOUTH;
    }

    cell->vertical = SDL_max(cell->vertical, weight);
  }
}

void
cp437_add_box(grid(cp437_border) * borders,
              SDL_Rect const* box,
              struct cp437_border_style const* style)
{
  int const right = box->x + box->w - 1;
  int const bottom = box->y + box->h - 1;

  cp437_add_hline(borders, box->x, box->y, box->w, style->top);
  cp437_add_hline(borders, box->x, bottom, box->w, style->bottom);
  cp437_add_vline(borders, box->x, box->y, box->h, style->left);
  cp437_add_vline(borders, right, box->y, box->h, style->right);
}

Uint8
cp437_border_glyph(struct cp437_border_cell const* cell)
{
  Uint8 const connections = cell->connections & 0x0F;

  if (connections == 0) {
    return CP437_SPACE;
  }

  int const horizontal_double = cell->horizontal == CP437_BORDER_DOUBLE;
  int const vertical_double = cell->vertical == CP437_BORDER_DOUBLE;

  return BORDER_GLYPHS[horizontal_double][vertical_double][connections];
}
