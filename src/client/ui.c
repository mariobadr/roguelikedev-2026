#include "ui.h"

#include "cell.h"

/** The margin (in tiles) between UI panels. */
#define RL_UI_MARGIN 1

void
rl_init_ui_layout(struct rl_ui_layout* layout)
{
  int const width = RL_UI_WIDTH;
  int const height = RL_UI_HEIGHT;
  int const margin = RL_UI_MARGIN;

  layout->main_panel.x = margin;
  layout->main_panel.y = 1 + margin;
  layout->main_panel.w = 64;
  layout->main_panel.h = 36;

  layout->top_panel.x = layout->main_panel.x;
  layout->top_panel.y = 0;
  layout->top_panel.w = layout->main_panel.w;
  layout->top_panel.h = layout->main_panel.y - layout->top_panel.y - margin;

  layout->bottom_panel.x = layout->main_panel.x;
  layout->bottom_panel.y = layout->main_panel.y + layout->main_panel.h + margin;
  layout->bottom_panel.w = layout->main_panel.w;
  layout->bottom_panel.h = height - layout->bottom_panel.y;

  layout->right_panel.x = layout->main_panel.x + layout->main_panel.w + margin;
  layout->right_panel.y = 0;
  layout->right_panel.h = height;
  layout->right_panel.w = width - layout->right_panel.x;

  // The current layout has no left panel.
  layout->left_panel.x = 0;
  layout->left_panel.y = 0;
  layout->left_panel.w = 0;
  layout->left_panel.h = 0;
}

SDL_Point
rl_panel_to_cell(SDL_Rect const* panel, SDL_Point local)
{
  SDL_Point point = { 0 };
  point.x = panel->x + local.x;
  point.y = panel->y + local.y;

  return point;
}

SDL_FPoint
rl_panel_to_pixels(SDL_Rect const* panel, SDL_Point local)
{
  SDL_Point const screen = rl_panel_to_cell(panel, local);
  return rl_cell_point_to_pixels(&screen);
}

bool
rl_cell_to_panel(SDL_Rect const* panel, SDL_Point at, SDL_Point* local)
{
  if (!SDL_PointInRect(&at, panel)) {
    return false;
  }

  local->x = at.x - panel->x;
  local->y = at.y - panel->y;

  return true;
}

SDL_Rect
rl_panel_clip_rect(SDL_Rect const* panel)
{
  SDL_FRect const px = rl_cell_rect_to_pixels(panel);
  return (SDL_Rect){ (int)px.x, (int)px.y, (int)px.w, (int)px.h };
}
