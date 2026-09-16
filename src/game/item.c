#include "item.h"

struct rl_item
rl_make_item(enum rl_item_type type)
{
  struct rl_item item = { 0 };
  item.handle = handle_invalid(rl_item);
  item.itype = type;
  item.ltype = RL_ITEM_LOCATION_NONE;

  return item;
}
