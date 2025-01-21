/**
 *---------------------------------------------------------------------------
 * @brief    A advanced string library
 *
 * @file     mstr.c
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2023-11-29
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include "mstr.h"

void mstr_move(mstr *dst, int pos, int moves);
void mstr_move(mstr *dst, int pos, int moves)
{
  int pos_x = ((pos >= 0) ? pos : ((int)dst->len + pos));
  // move existing text forward
  for (int i = dst->len - 1; i >= pos_x; i--)
  {
    dst->str[i + moves] = dst->str[i];
  }
  dst->len += moves;
  dst->str[dst->len] = '\0';
}

void mstr_copy(mstr *dst, char *src, int pos, size_t len);
void mstr_copy(mstr *dst, char *src, int pos, size_t len)
{
  int pos_x = ((pos >= 0) ? pos : ((int)dst->len + pos - 1));
  // copy new text from src
  for (size_t i = 0; i < len; i++)
  {
    dst->str[i + pos_x] = src[i];
  }
}

void mstr_clear(mstr *s)
{
  s->len = 0;
  s->str[0] = '\0';
}

void mstr_allocate_capacity(mstr *dst, size_t required_capacity);
void mstr_allocate_capacity(mstr *dst, size_t required_capacity)
{
  char *new_str;
  size_t new_capacity;

  new_capacity = ((required_capacity / MSTR_BLOCK) + 1) * MSTR_BLOCK;
  if ((new_capacity - required_capacity) <= MSTR_EXTRA)
    new_capacity += MSTR_BLOCK;

  if (new_capacity <= dst->capacity)
    return;

  // printf("Current capacity: %3lu  Required capacity: %3lu  New capacity: %lu\n", dst->capacity, required_capacity, new_capacity);
  dst->capacity = new_capacity;

  new_str = (char *)malloc(new_capacity);
  if (dst->str == NULL)
  {
    dst->str = new_str;
    return;
  }

  strncpy(new_str, dst->str, dst->len);
  free(dst->str);
  dst->str = new_str;
}

void mstr_assert_capacity(mstr *mst, size_t required_capacity)
{
  if (((int)mst->capacity - (int)required_capacity) < MSTR_EXTRA)
  {
    mstr_allocate_capacity(mst, required_capacity);
  }
  // printf("Assert: %s\n", mst->str);
  // printf("Capacity: %3lu   Required capacity: %lu\n", mst->capacity, required_capacity);
  assert(mst->capacity >= (required_capacity + MSTR_EXTRA));
}

mstr *mstr_new_g(size_t required_capacity, char *src)
{
  mstr *mst = (mstr *)malloc(sizeof(mstr));

  mst->str = NULL;
  mst->capacity = 0;
  mst->len = 0;
  mstr_allocate_capacity(mst, required_capacity);

  if (src == NULL)
    return mst;

  mstr_append(mst, src);
  return mst;
}

mstr *mstr_newc(char *src)
{
  return mstr_new_g(strlen(src), src);
}

mstr *mstr_news(mstr *src)
{
  return mstr_new_g(src->len, src->str);
}

mstr *mstr_newl(size_t size)
{
  return mstr_new_g(size, NULL);
}

void mstr_append_g(mstr *dst, char *src, size_t src_len)
{
  mstr_assert_capacity(dst, dst->len + src_len);

  mstr_insert_g(dst, src, src_len, dst->len);
}

void mstr_append_c(mstr *mst, char *astr)
{
  mstr_append_g(mst, astr, strlen(astr));
}

void mstr_append_s(mstr *mst, mstr *astr)
{
  mstr_append_g(mst, astr->str, astr->len);
}

void mstr_prepend_g(mstr *dst, char *src, size_t len)
{
  mstr_assert_capacity(dst, dst->len + len);

  mstr_insert(dst, src, 0);
}

void mstr_prepend_c(mstr *dst, char *src)
{
  mstr_prepend_g(dst, src, strlen(src));
}

void mstr_prepend_s(mstr *dst, mstr *src)
{
  mstr_prepend_g(dst, src->str, src->len);
}

void mstr_insert_g(mstr *dst, char *src, size_t len, int pos)
{

  if (dst->capacity <= (dst->len + len))
  {
    mstr_allocate_capacity(dst, dst->len + len);
  }
  assert(dst->capacity > (dst->len + len - pos));

  mstr_move(dst, pos, len);
  mstr_copy(dst, src, pos, len);
}

void mstr_insert_c(mstr *dst, char *src, int pos)
{
  mstr_insert_g(dst, src, strlen(src), pos);
}

void mstr_insert_s(mstr *dst, mstr *src, int pos)
{
  mstr_insert_g(dst, src->str, src->len, pos);
}

// int mstr_find(mstr *s, char *x)
// {
// }

void mstr_replace(mstr *s, char *old, char *new)
{
  char *x;
  int pos;
  x = strstr(s->str, old);
  pos = (int)x - (int)s;
  // printf("x = %d\n", pos);
}

void mstr_free(mstr *s)
{
  free(s->str);
  free(s);
}

bool mstr_instr(char ch, char *str);
bool mstr_instr(char ch, char *str)
{
  for (size_t i = 0; i < strlen(str); i++)
  {
    if (ch == str[i])
      return true;
  }
  return false;
}

void mstr_rstrip(mstr *s, char *st)
{
  size_t anchor;
  anchor = 0;

  for (size_t i = 0; i < s->len; i++)
  {
    if (mstr_instr(s->str[i], st))
      anchor++;
    else
      break;
  }

  for (size_t i = 0; i < s->len; i++)
    s->str[i] = s->str[i + anchor];

  s->len -= anchor;
}

void mstr_lstrip(mstr *s, char *st)
{

  for (int i = s->len - 1; i > 0; i--)
  {
    if (mstr_instr(s->str[i], st))
      s->len--;
    else
      break;
  }
  s->str[s->len] = '\0';
}

void mstr_strip(mstr *s, char *st)
{
  mstr_rstrip(s, st);
  mstr_lstrip(s, st);
}

size_t mstr_len(mstr *s)
{
  return s->len;
}

bool mstr_is_alpha(mstr *s)
{
  if (s->len == 0)
    return false;

  for (size_t i = 0; i < s->len; i++)
  {
    if (!isalpha(s->str[i]))
      return false;
  }
  return true;
}

bool mstr_is_numeric(mstr *s)
{
  if (s->len == 0)
    return false;

  for (size_t i = 0; i < s->len; i++)
  {
    if (!isdigit(s->str[i]))
      return false;
  }
  return true;
}

bool mstr_is_alnum(mstr *s)
{
  if (s->len == 0)
    return false;

  for (size_t i = 0; i < s->len; i++)
  {
    if (!isalnum(s->str[i]))
      return false;
  }
  return true;
}

bool mstr_is_space(mstr *s)
{
  if (s->len == 0)
    return false;

  for (size_t i = 0; i < s->len; i++)
  {
    if (!isspace(s->str[i]))
      return false;
  }
  return true;
}

bool mstr_is_empty(mstr *s)
{
  if (s->len == 0)
    return true;

  return false;
}

void mstr_upper(mstr *s)
{
  for (size_t i = 0; i < s->len; i++)
  {
    s->str[i] = toupper(s->str[i]);
  }
}

void mstr_lower(mstr *s)
{
  for (size_t i = 0; i < s->len; i++)
  {
    s->str[i] = tolower(s->str[i]);
  }
}

void mstr_print(mstr *s)
{
  if (s == NULL)
  {
    printf(" Capacity  Len A N AN S  string\n");
    return;
  }

  printf("      %4ld %4ld %d %d  %d %d  \"%s\"\n", s->capacity, s->len, mstr_is_alpha(s), mstr_is_numeric(s), mstr_is_alnum(s), mstr_is_space(s), s->str);
}