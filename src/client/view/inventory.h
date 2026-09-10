/**
 * @file inventory.h
 */
#ifndef GINC_ROGUELIKE_INVENTORY_VIEW_H
#define GINC_ROGUELIKE_INVENTORY_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_font;
struct rl_ribbon;
struct rl_world;

struct rl_inv_view
{
  /** First visible item in the filtered inventory. */
  int first;
  /** Selected item in the filtered inventory. */
  int selected;
  SDL_FRect viewport;
  float line_height;
  int slot_count;
  SDL_FRect slots[8]; // temporary; these are the rects where we draw the text
};

void
rl_init_inv_view(struct rl_inv_view* view,
                 SDL_FRect const* viewport,
                 float line_height);

void
rl_resize_inv_view(struct rl_inv_view* view, SDL_FRect const* viewport);

void
rl_select_inv_view_up(struct rl_inv_view* view, struct rl_world const* world);

void
rl_select_inv_view_down(struct rl_inv_view* view, struct rl_world const* world);

void
rl_select_inv_view_to(struct rl_inv_view* view,
                      struct rl_world const* world,
                      int selected);

void
rl_inv_view_ribbon(struct rl_inv_view const* view, struct rl_ribbon* ribbon);

/** Return the selected item ID, or -1 if no item is selected. */
int
rl_inv_view_selected_item(struct rl_inv_view const* view,
                          struct rl_world const* world);

void
rl_draw_inv_view(struct rl_inv_view const* view,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_world const* world);

#endif // GINC_ROGUELIKE_INVENTORY_VIEW_H
