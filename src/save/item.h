/**
 * @file item.h
 */
#ifndef GINC_ROGUELIKE_SAVE_ITEM_H
#define GINC_ROGUELIKE_SAVE_ITEM_H

#include <SDL3/SDL_stdinc.h>

#include "game/item_def.h"

#include "save/result.h"

// external forward declarations
typedef struct SDL_IOStream SDL_IOStream;

// forward declarations
struct rl_item;
struct rl_reader;
struct rl_writer;

/**
 * @return whether type is a known item type.
 */
bool
rl_is_valid_item_type(enum rl_item_type type);

/**
 * Write item's state to dst.
 *
 * w resolves the item's actor handle to a stable id, if it is held or
 * equipped by an actor.
 *
 * @return whether the write succeeded.
 */
bool
rl_write_item(SDL_IOStream* dst,
              struct rl_writer const* w,
              struct rl_item const* item);

/**
 * Read an item's state from src into out.
 *
 * r resolves a stable actor id back to a handle, if the item is held or
 * equipped by an actor.
 */
enum rl_read_result
rl_read_item(SDL_IOStream* src, struct rl_reader const* r, struct rl_item* out);

#endif // GINC_ROGUELIKE_SAVE_ITEM_H
