/**
 * @file str_wrap.h
 *
 * Line-breaking (word-wrap) for fixed-width byte-glyph text, independent of
 * rendering, allocation, and geometry.
 */
#ifndef GINC_UI_STR_WRAP_H
#define GINC_UI_STR_WRAP_H

#include <SDL3/SDL_stdinc.h>

/**
 * A span referencing a contiguous, not null-terminated run of bytes.
 */
struct ui_string_span
{
  char const* data; /**< Pointer to the first byte of the span. */
  int length;       /**< Number of bytes in the span. */
};

/**
 * Extract the next wrapped line from *cursor.
 *
 * Takes as many characters as fit within columns; if more text remains
 * after that point, the break is moved back to the last space within the
 * taken range (if any), so words are not split mid-word. Spaces
 * immediately following the break are skipped, so the next call starts at
 * the beginning of the next word.
 *
 * @param cursor  In/out pointer to the remaining text. Must point to a
 *                null-terminated string. Advanced in place past the
 *                returned line (and any separating spaces) on success.
 * @param columns Maximum number of characters per line.
 * @param line    Out parameter receiving the span of the next line. Left
 *                unmodified if this function returns false.
 *
 * @return true if a line was produced; false if the input is exhausted
 *         (**cursor == '\0') or columns is zero.
 */
bool
ui_wrap_next(char const** cursor, int columns, struct ui_string_span* line);

#endif // GINC_UI_STR_WRAP_H
