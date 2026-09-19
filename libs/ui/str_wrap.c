#include "ui/str_wrap.h"

bool
ui_wrap_next(char const** cursor, int columns, struct str_view* line)
{
  char const* text = *cursor;

  if (columns <= 0 || text[0] == '\0') {
    return false;
  }

  int length = 0;

  // Take as many characters as will fit.
  while (text[length] != '\0' && length < columns) {
    ++length;
  }

  // If more text remains, prefer breaking at a space.
  if (text[length] != '\0') {
    int split = length;

    while (split > 0 && text[split] != ' ') {
      --split;
    }

    if (split > 0) {
      length = split;
    }
  }

  char const* next = text + length;

  // Skip spaces before the next line.
  while (*next == ' ') {
    ++next;
  }

  line->data = text;
  line->length = length;
  *cursor = next;

  return true;
}
