/**
 * @file context.h
 */
#ifndef GINC_ROGUELIKE_SAVE_CONTEXT_H
#define GINC_ROGUELIKE_SAVE_CONTEXT_H

#include <SDL3/SDL_stdinc.h>

#include "container/array.h"

#include "game/handles.h"

// forward declarations
struct rl_world;

/**
 * A growable array of actor handles.
 */
array_define_as(handle(rl_actor), rl_actor_handle);

/**
 * A growable array of item handles.
 */
array_define_as(handle(rl_item), rl_item_handle);

// index into the pool corresponds to an entity's handle index; id is 0 for
// unoccupied slots.
struct rl_id_entry
{
  Uint32 id;
  Uint32 generation;
};

/**
 * A growable array of id-map entries.
 */
array_define_as(struct rl_id_entry, rl_id_entry);

// entity IDs are 1-based; 0 means "no entity"
#define RL_SNAPSHOT_MAX_LEVELS 128u
#define RL_SNAPSHOT_MAX_ACTORS 512u
#define RL_SNAPSHOT_MAX_ITEMS 512u
#define RL_SNAPSHOT_MAX_DIM 256

/**
 * Maps the ids assigned to actors and items in a snapshot back to their
 * handles, while that snapshot is being read.
 */
struct rl_reader
{
  Uint32 actor_count;
  Uint32 item_count;
  array(rl_actor_handle) actor_handles; //< index 1..actor_count
  array(rl_item_handle) item_handles;   //< index 1..item_count
};

/**
 * Free the resources held by r.
 */
void
rl_free_reader(struct rl_reader* r);

/**
 * Assigns actors and items in a world stable ids, so they can be referenced
 * by id while a snapshot of it is being written.
 */
struct rl_writer
{
  Uint32 actor_count;
  array(rl_id_entry) actor_entries;

  Uint32 item_count;
  array(rl_id_entry) item_entries;
};

/**
 * Prepare out to write a snapshot of world, assigning each of its actors and
 * items a stable id.
 *
 * @return whether out was prepared successfully.
 */
bool
rl_alloc_writer(struct rl_world const* world, struct rl_writer* out);

/**
 * Free the resources held by w.
 */
void
rl_free_writer(struct rl_writer* w);

/**
 * @return h's id in w, or 0 if h is not a live actor handle in the world w
 * was prepared for.
 */
Uint32
rl_to_actor_id(struct rl_writer const* w, handle(rl_actor) h);

/**
 * @return h's id in w, or 0 if h is not a live item handle in the world w
 * was prepared for.
 */
Uint32
rl_to_item_id(struct rl_writer const* w, handle(rl_item) h);

#endif // GINC_ROGUELIKE_SAVE_CONTEXT_H
