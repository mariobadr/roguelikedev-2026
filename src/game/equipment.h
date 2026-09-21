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
};

/**
 * The items an actor has equipped.
 */
struct rl_equipment
{
  /** The item wielded. */
  handle(rl_item) weapon;
  /** The item worn. */
  handle(rl_item) armour;
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
 * @return whether rl_equip would accept item.
 */
bool
rl_can_equip(struct rl_actor const* actor, struct rl_item const* item);

/**
 * @return whether rl_unequip would accept item.
 */
bool
rl_can_unequip(struct rl_actor const* actor, struct rl_item const* item);

/**
 * @return the equipped item's handle, or an invalid handle if the slot is
 * empty or does not exist.
 */
handle(rl_item)
rl_get_equipped_item(struct rl_actor const* actor, enum rl_equipment_slot slot);

/**
 * Put item into the actor's compatible slot and mark it equipped.
 *
 * @return whether item now occupies a slot.
 */
bool
rl_equip(struct rl_actor* actor, struct rl_item* item);

/**
 * Empty the slot holding item, moving it to inventory.
 *
 * @return whether item is now held rather than equipped.
 */
bool
rl_unequip(struct rl_actor* actor, struct rl_item* item);

#endif // GINC_ROGUELIKE_EQUIPMENT_H
