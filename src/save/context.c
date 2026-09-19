#include "context.h"

#include "result.h"

#include "game/world.h"

void
rl_free_reader(struct rl_reader* r)
{
  array_free(&r->actor_handles);
  array_free(&r->item_handles);
}

static bool
build_id_map(void const* slots,
             size_t slot_stride,
             Uint32 cap,
             array(rl_id_entry) * out_entries,
             Uint32* out_count)
{
  if (cap == 0) {
    *out_count = 0;
    return true;
  }

  if (!array_alloc(out_entries, cap)) {
    return false;
  }

  Uint32 count = 0;
  for (Uint32 i = 0; i < cap; i++) {
    struct pool_slot_state const* const state =
      (struct pool_slot_state const*)((char const*)slots +
                                      (size_t)i * slot_stride);
    if (state->occupied) {
      struct rl_id_entry* const entry = array_at(out_entries, i);
      entry->id = ++count;
      entry->generation = state->generation;
    }
  }

  *out_count = count;
  return true;
}

bool
rl_alloc_writer(struct rl_world const* world, struct rl_writer* out)
{
  SDL_zerop(out);

  if (!build_id_map(world->actors.slots,
                    sizeof(*world->actors.slots),
                    pool_cap(&world->actors),
                    &out->actor_entries,
                    &out->actor_count)) {
    rl_free_writer(out);
    return false;
  }

  if (!build_id_map(world->items.slots,
                    sizeof(*world->items.slots),
                    pool_cap(&world->items),
                    &out->item_entries,
                    &out->item_count)) {
    rl_free_writer(out);
    return false;
  }

  return true;
}

void
rl_free_writer(struct rl_writer* w)
{
  array_free(&w->actor_entries);
  array_free(&w->item_entries);
}

Uint32
rl_to_actor_id(struct rl_writer const* w, handle(rl_actor) h)
{
  if (h.index >= array_cap(&w->actor_entries)) {
    return 0;
  }

  struct rl_id_entry const* const entry = array_at(&w->actor_entries, h.index);
  if (entry->id == 0 || entry->generation != h.generation) {
    return 0;
  }

  return entry->id;
}

Uint32
rl_to_item_id(struct rl_writer const* w, handle(rl_item) h)
{
  if (h.index >= array_cap(&w->item_entries)) {
    return 0;
  }

  struct rl_id_entry const* const entry = array_at(&w->item_entries, h.index);
  if (entry->id == 0 || entry->generation != h.generation) {
    return 0;
  }

  return entry->id;
}
