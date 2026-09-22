#include "equipment.h"

#include "actor.h"
#include "item.h"
#include "item_def.h"

void
rl_init_equipment(struct rl_equipment* equipment)
{
  for (int slot = 0; slot < RL_EQUIPMENT_SLOT_COUNT; ++slot) {
    equipment->slots[slot] = handle_invalid(rl_item);
  }
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
  return actor->equipment.slots[slot];
}

void
rl_equip(struct rl_actor* actor, struct rl_item* item)
{
  actor->equipment.slots[rl_get_equipment_slot(item->itype)] = item->handle;
  item->ltype = RL_ITEM_LOCATION_EQUIPPED;
}

bool
rl_is_equipped_by(struct rl_actor const* actor, struct rl_item const* item)
{
  if (item->ltype != RL_ITEM_LOCATION_EQUIPPED ||
      !handle_equal(item->on.actor, actor->handle)) {
    return false;
  }

  return handle_equal(
    actor->equipment.slots[rl_get_equipment_slot(item->itype)], item->handle);
}

void
rl_unequip(struct rl_actor* actor, struct rl_item* item)
{
  actor->equipment.slots[rl_get_equipment_slot(item->itype)] =
    handle_invalid(rl_item);
  item->ltype = RL_ITEM_LOCATION_HELD;
}
