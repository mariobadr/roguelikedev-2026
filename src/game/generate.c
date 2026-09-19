#include "generate.h"

#include <SDL3/SDL_assert.h>

#include "layout.h"
#include "level.h"
#include "spawn.h"
#include "world.h"

// the room holding the stairs up; no actors spawn here
#define RL_GEN_ENTRY_ROOM 0

static void
fill_map(grid(rl_tile) * map, SDL_Rect const* rect, enum rl_tile tile)
{
  for (int y = rect->y; y < rect->y + rect->h; ++y) {
    for (int x = rect->x; x < rect->x + rect->w; ++x) {
      *grid_at(map, x, y) = tile;
    }
  }
}

static void
carve_map(grid(rl_tile) * map, struct rl_layout const* layout)
{
  enum rl_tile tile = RL_TILE_FLOOR;

  for (int i = 0; i < array_len(&layout->rooms); i++) {
    fill_map(map, array_at(&layout->rooms, i), tile);
  }

  for (int i = 0; i < array_len(&layout->corridors); i++) {
    struct rl_corridor const* corridor = array_at(&layout->corridors, i);
    for (int j = 0; j < corridor->segment_count; j++) {
      fill_map(map, &corridor->segments[j], tile);
    }
  }
}

static bool
generate_map(struct rl_level* level,
             struct rl_layout* layout,
             struct rand_state* rng)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  // randomly generate the dungeon layout
  if (!rl_init_layout(layout, width, height, rng)) {
    return false;
  }

  // update the tiles in the map based on the layout
  carve_map(&level->map, layout);

  return true;
}

static SDL_Point
centre_of(SDL_Rect const* rect)
{
  SDL_Point const centre = { rect->x + rect->w / 2, rect->y + rect->h / 2 };
  return centre;
}

static bool
place_stairs(struct rl_level* level, struct rl_layout const* layout)
{
  int const room_count = (int)array_len(&layout->rooms);
  if (room_count < 2) {
    return false;
  }

  // the stairs up are at the centre of the first room, the stairs down at the
  // centre of the last
  level->stairs_up = centre_of(array_at(&layout->rooms, RL_GEN_ENTRY_ROOM));
  level->stairs_down = centre_of(array_at(&layout->rooms, room_count - 1));

  *grid_at(&level->map, level->stairs_down.x, level->stairs_down.y) =
    RL_TILE_STAIRS_DOWN;

  // the first level has nothing above it
  if (level->depth > 1) {
    *grid_at(&level->map, level->stairs_up.x, level->stairs_up.y) =
      RL_TILE_STAIRS_UP;
  }

  return true;
}

static bool
populate_level(struct rl_world* world,
               struct rl_level* level,
               struct rl_layout const* layout,
               struct rand_state* rng)
{
  // spawn the actors
  if (!rl_spawn_actors(level, layout, world, RL_GEN_ENTRY_ROOM, rng)) {
    return false;
  }

  // spawn items
  if (!rl_spawn_items(level, layout, world, rng)) {
    return false;
  }

  return true;
}

static bool
generate_level(struct rl_world* world,
               struct rl_level* level,
               struct rl_layout* layout,
               struct rand_state* rng)
{
  if (!generate_map(level, layout, rng)) {
    return false;
  }

  if (!place_stairs(level, layout)) {
    return false;
  }

  return populate_level(world, level, layout, rng);
}

static void
discard_level(struct rl_world* world,
              struct rl_level* level,
              size_t actors_before,
              size_t items_before)
{
  // release what generation spawned, but not what the level held beforehand
  for (size_t i = actors_before; i < alist_len(&level->actors); i++) {
    handle(rl_actor) const h = *alist_at(&level->actors, i);
    pool_release(&world->actors, h);
  }

  for (size_t i = items_before; i < alist_len(&level->items); i++) {
    handle(rl_item) const h = *alist_at(&level->items, i);
    pool_release(&world->items, h);
  }

  rl_free_level(level);
}

bool
rl_gen_level(struct rl_world* world,
             struct rl_level* level,
             struct rand_state* rng)
{
  size_t const actors_before = alist_len(&level->actors);
  size_t const items_before = alist_len(&level->items);

  struct rl_layout layout = { 0 };
  bool const ok = generate_level(world, level, &layout, rng);

  rl_free_layout(&layout);

  if (!ok) {
    discard_level(world, level, actors_before, items_before);
  }

  return ok;
}

bool
rl_push_level(struct rl_world* world,
              int width,
              int height,
              handle(rl_actor) arriving,
              struct rand_state* rng)
{
  if (rl_borrow_actor(world, arriving) == NULL) {
    return false;
  }

  struct rl_level* level = alist_push(&world->levels);
  if (level == NULL) {
    return false;
  }

  // the slot may hold a previously freed level
  *level = (struct rl_level){ 0 };

  int const depth = (int)alist_len(&world->levels);
  if (!rl_alloc_level(level, depth, width, height) ||
      !rl_add_actor(level, arriving)) {
    rl_free_level(level);
    alist_pop(&world->levels);
    return false;
  }

  // the arriving actor is on the level first, so it takes the first turn
  if (!rl_gen_level(world, level, rng)) {
    // rl_gen_level frees the level on failure
    alist_pop(&world->levels);
    return false;
  }

  // generation may have grown the actor pool, invalidating earlier borrows
  struct rl_actor* actor = rl_borrow_mut_actor(world, arriving);
  SDL_assert(actor != NULL);
  actor->pos = level->stairs_up;

  return true;
}
