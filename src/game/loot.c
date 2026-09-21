#include "loot.h"

#include <SDL3/SDL_assert.h>

#include "core/rand.h"

bool
rl_roll_loot(struct rl_loot_table const* table,
             struct rand_state* rng,
             enum rl_item_type* out)
{
  SDL_assert(table->drop_percent >= 0 && table->drop_percent <= 100);
  SDL_assert(table->count == 0 || table->items != NULL);

  if (table->count == 0 || table->drop_percent <= 0) {
    return false;
  }

  Uint64 total_weight = 0;
  for (size_t i = 0; i < table->count; i++) {
    Uint32 const weight = table->items[i].weight;
    total_weight += weight;
  }

  if (total_weight == 0) {
    return false;
  }

  // first roll
  Uint64 roll = rand_next_up_to(rng, 100);
  if (table->drop_percent < 100 && roll >= (Uint64)table->drop_percent) {
    // no item will be dropped
    return false;
  }

  // second roll
  roll = rand_next_up_to(rng, total_weight);
  for (size_t i = 0; i < table->count; i++) {
    struct rl_loot_entry const* entry = &table->items[i];

    if (roll < entry->weight) {
      // an item is dropped
      *out = entry->item;
      return true;
    }

    roll -= entry->weight;
  }

  return false;
}
