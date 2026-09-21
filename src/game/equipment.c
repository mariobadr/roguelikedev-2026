#include "equipment.h"

#include "actor.h"
#include "item.h"
#include "item_def.h"

static handle(rl_item) const*
get_slot(struct rl_actor const* actor, enum rl_equipment_slot slot)
{
  switch (slot) {
    case RL_EQUIPMENT_SLOT_WEAPON:
      return &actor->equipment.weapon;
    case RL_EQUIPMENT_SLOT_ARMOUR:
      return &actor->equipment.armour;
    case RL_EQUIPMENT_SLOT_NONE:
      break;
  }

  return NULL;
}

static handle(rl_item)*
edit_slot(struct rl_actor* actor, enum rl_equipment_slot slot)
{
  return (handle(rl_item)*)get_slot(actor, slot);
}

void
rl_init_equipment(struct rl_equipment* equipment)
{
  equipment->weapon = handle_invalid(rl_item);
  equipment->armour = handle_invalid(rl_item);
}

enum rl_equipment_slot
rl_get_equipment_slot(enum rl_item_type type)
{
  switch (rl_get_item_def(type)->class) {
    case RL_ITEM_CLASS_WEAPON:
      return RL_EQUIPMENT_SLOT_WEAPON;
    case RL_ITEM_CLASS_ARMOUR:
      return RL_EQUIPMENT_SLOT_ARMOUR;
    case RL_ITEM_CLASS_POTION:
    case RL_ITEM_CLASS_SCROLL:
      break;
  }

  return RL_EQUIPMENT_SLOT_NONE;
}

handle(rl_item)
rl_get_equipped_item(struct rl_actor const* actor, enum rl_equipment_slot slot)
{
  handle(rl_item) const* const item = get_slot(actor, slot);
  return item != NULL ? *item : handle_invalid(rl_item);
}

bool
rl_can_equip(struct rl_actor const* actor, struct rl_item const* item)
{
  if (item->ltype != RL_ITEM_LOCATION_HELD &&
      item->ltype != RL_ITEM_LOCATION_EQUIPPED) {
    return false;
  }

  if (!handle_equal(item->on.actor, actor->handle)) {
    return false;
  }

  return rl_get_equipment_slot(item->itype) != RL_EQUIPMENT_SLOT_NONE;
}

bool
rl_equip(struct rl_actor* actor, struct rl_item* item)
{
  if (!rl_can_equip(actor, item)) {
    return false;
  }

  handle(rl_item)* const slot =
    edit_slot(actor, rl_get_equipment_slot(item->itype));
  if (handle_is_nonnull(*slot) && !handle_equal(*slot, item->handle)) {
    return false;
  }

  *slot = item->handle;
  item->ltype = RL_ITEM_LOCATION_EQUIPPED;

  return true;
}

bool
rl_is_equipped_by(struct rl_actor const* actor, struct rl_item const* item)
{
  if (item->ltype != RL_ITEM_LOCATION_EQUIPPED ||
      !handle_equal(item->on.actor, actor->handle)) {
    return false;
  }

  handle(rl_item) const* const slot =
    get_slot(actor, rl_get_equipment_slot(item->itype));

  return slot != NULL && handle_equal(*slot, item->handle);
}

bool
rl_can_unequip(struct rl_actor const* actor, struct rl_item const* item)
{
  return rl_is_equipped_by(actor, item);
}

bool
rl_unequip(struct rl_actor* actor, struct rl_item* item)
{
  if (!rl_can_unequip(actor, item)) {
    return false;
  }

  *edit_slot(actor, rl_get_equipment_slot(item->itype)) =
    handle_invalid(rl_item);
  item->ltype = RL_ITEM_LOCATION_HELD;

  return true;
}
