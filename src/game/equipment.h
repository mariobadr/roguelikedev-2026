/**
 * @file equipment.h
 */
#ifndef GINC_ROGUELIKE_EQUIPMENT_H
#define GINC_ROGUELIKE_EQUIPMENT_H

#include <SDL3/SDL_stdinc.h>

#include "game/handles.h"
#include "game/item_def.h"

// forward declarations
struct rl_actor;
struct rl_item;

/**
 * The slots an actor can equip an item into.
 */
enum rl_equipment_slot
{
  RL_EQUIPMENT_SLOT_NONE,   //< the item cannot be equipped
  RL_EQUIPMENT_SLOT_WEAPON, //< the item is wielded
  RL_EQUIPMENT_SLOT_ARMOUR, //< the item is worn
  RL_EQUIPMENT_SLOT_COUNT,  //< number of slots; not a slot
};

/**
 * The items an actor has equipped.
 *
 * @invariant slots[RL_EQUIPMENT_SLOT_NONE] is invalid.
 * @invariant every other valid handle in slots is a live item that is
 * equipped and belongs in that slot.
 */
struct rl_equipment
{
  /** The item in each slot. */
  handle(rl_item) slots[RL_EQUIPMENT_SLOT_COUNT];
};

/**
 * Initialise equipment to empty slots.
 */
void
rl_init_equipment(struct rl_equipment* equipment);

/**
 * @return the slot an item of this type occupies, or RL_EQUIPMENT_SLOT_NONE if
 * the type cannot be equipped.
 */
enum rl_equipment_slot
rl_get_equipment_slot(enum rl_item_type type);

/**
 * @return whether item is equipped by actor.
 */
bool
rl_is_equipped_by(struct rl_actor const* actor, struct rl_item const* item);

/**
 * @return the equipped item's handle, or an invalid handle if the slot is
 * empty.
 */
handle(rl_item)
rl_get_equipped_item(struct rl_actor const* actor, enum rl_equipment_slot slot);

/**
 * Put item into the actor's compatible slot and mark it equipped.
 *
 * @param item must be equippable and held by actor, and its slot must be
 * empty.
 */
void
rl_equip(struct rl_actor* actor, struct rl_item* item);

/**
 * Empty the slot holding item, moving it to inventory.
 *
 * @param item must be equipped by actor.
 */
void
rl_unequip(struct rl_actor* actor, struct rl_item* item);

#endif // GINC_ROGUELIKE_EQUIPMENT_H
