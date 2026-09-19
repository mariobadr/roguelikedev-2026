#include "grid_view.h"

#include "camera.h"

SDL_FPoint
gfx_cell_to_world(struct gfx_grid_view const* view, SDL_Point cell)
{
  return (SDL_FPoint){ (float)cell.x * view->cell_width,
                       (float)cell.y * view->cell_height };
}

SDL_Point
gfx_world_to_cell(struct gfx_grid_view const* view, SDL_FPoint world)
{
  return (SDL_Point){ (int)SDL_floorf(world.x / (float)view->cell_width),
                      (int)SDL_floorf(world.y / (float)view->cell_height) };
}

SDL_FPoint
gfx_cell_to_screen(struct gfx_grid_view const* view,
                   struct gfx_camera const* camera,
                   SDL_Point cell)
{
  return gfx_world_to_screen(camera, gfx_cell_to_world(view, cell));
}

bool
gfx_screen_to_cell(struct gfx_grid_view const* view,
                   struct gfx_camera const* camera,
                   SDL_FPoint screen,
                   SDL_Point* out)
{
  if (!SDL_PointInRectFloat(&screen, &camera->viewport)) {
    return false;
  }

  *out = gfx_world_to_cell(view, gfx_screen_to_world(camera, screen));
  return true;
}

SDL_Rect
gfx_grid_view_bounds(struct gfx_grid_view const* view,
                     struct gfx_camera const* camera)
{
  SDL_FRect const world = gfx_camera_world_bounds(camera);
  SDL_Point const first = gfx_world_to_cell(view, camera->position);
  int const end_x =
    (int)SDL_ceilf((world.x + world.w) / (float)view->cell_width);
  int const end_y =
    (int)SDL_ceilf((world.y + world.h) / (float)view->cell_height);

  return (SDL_Rect){ first.x,
                     first.y,
                     world.w > 0.0f ? end_x - first.x : 0,
                     world.h > 0.0f ? end_y - first.y : 0 };
}

SDL_Rect
gfx_visible_cells(struct gfx_grid_view const* view,
                  struct gfx_camera const* camera,
                  int columns,
                  int rows)
{
  SDL_Rect const bounds = gfx_grid_view_bounds(view, camera);
  SDL_Rect const map = { 0, 0, columns, rows };
  SDL_Rect visible = { 0 };
  SDL_GetRectIntersection(&bounds, &map, &visible);
  return visible;
}
