#include "ui.h"

#include "cell.h"

/** The horizontal margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_X 6.0f
/** The vertical margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_Y 8.0f

void
rl_init_ui_layout(struct rl_ui_layout* layout)
{
  float const width = (float)RL_UI_WIDTH;
  float const height = (float)RL_UI_HEIGHT;
  float const margin_x = RL_UI_MARGIN_X;
  float const margin_y = RL_UI_MARGIN_Y;

  layout->main_panel.x = margin_x;
  layout->main_panel.y = margin_y * 2.0f;
  // Must stay a whole number of cells while the map view is cell-based.
  layout->main_panel.w = 384.0f;
  layout->main_panel.h = 288.0f;

  layout->top_panel.x = layout->main_panel.x;
  layout->top_panel.y = 0.0f;
  layout->top_panel.w = layout->main_panel.w;
  layout->top_panel.h = layout->main_panel.y - layout->top_panel.y - margin_y;

  layout->bottom_panel.x = layout->main_panel.x;
  layout->bottom_panel.y = layout->main_panel.y + layout->main_panel.h + margin_y;
  layout->bottom_panel.w = layout->main_panel.w;
  layout->bottom_panel.h = height - layout->bottom_panel.y;

  layout->right_panel.x = layout->main_panel.x + layout->main_panel.w + margin_x;
  layout->right_panel.y = 0.0f;
  layout->right_panel.h = height;
  layout->right_panel.w = width - layout->right_panel.x;

  // The current layout has no left panel.
  layout->left_panel.x = 0.0f;
  layout->left_panel.y = 0.0f;
  layout->left_panel.w = 0.0f;
  layout->left_panel.h = 0.0f;
}

SDL_FPoint
rl_panel_to_pixels(SDL_FRect const* panel, SDL_Point cell)
{
  SDL_FPoint const offset = rl_cell_point_to_pixels(&cell);

  SDL_FPoint pixels = { 0 };
  pixels.x = panel->x + offset.x;
  pixels.y = panel->y + offset.y;

  return pixels;
}

bool
rl_pixels_to_panel(SDL_FRect const* panel, SDL_FPoint at, SDL_FPoint* local)
{
  if (!SDL_PointInRectFloat(&at, panel)) {
    return false;
  }

  local->x = at.x - panel->x;
  local->y = at.y - panel->y;

  return true;
}

SDL_Rect
rl_panel_clip_rect(SDL_FRect const* panel)
{
  return (SDL_Rect){ (int)panel->x, (int)panel->y, (int)panel->w, (int)panel->h };
}
