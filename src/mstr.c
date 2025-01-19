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

void mstr_move(mstr *dst, int pos, int moves)
{
  int pos_x = ((pos >= 0) ? pos : (dst->len + pos));
  // move existing text forward
  for (int i = dst->len - 1; i >= pos_x; i--)
  {
    dst->str[i + moves] = dst->str[i];
  }
  dst->len += moves;
  dst->str[dst->len] = '\0';
}

void mstr_copy(mstr *dst, char *src, int pos, size_t len)
{
  int pos_x = ((pos >= 0) ? pos : (dst->len + pos - 1));
  // copy new text from src
  for (size_t i = 0; i < len; i++)
  {
    dst->str[i + pos_x] = src[i];
  }
}

mstr *mstr_new_g(size_t capacity, size_t len, char *str)
{
  mstr *mst = (mstr *)malloc(sizeof(mstr));
  mst->capacity = capacity;
  mst->str = (char *)malloc(mst->capacity);
  mstr_append(mst, str);
  return mst;
}

void mstr_clear(mstr *s)
{
  s->len = 0;
  s->str[0] = '\0';
}

mstr *mstr_newc(char *str)
{
  size_t len;
  len = strlen(str);
  return mstr_new_g(len, len, str);
}

mstr *mstr_news(mstr *src)
{
  return mstr_new_g(src->len, src->len, src->str);
}

mstr *mstr_newl(size_t size)
{
  return mstr_new_g(size, 0, "");
}

void mstr_reallocate(mstr *dst, size_t new_len)
{
  char *new_str;
  size_t new_capacity;

  new_capacity = ((new_len / MSTR_BLOCK) + 1) * MSTR_BLOCK;

  new_str = (char *)malloc(new_capacity);
  strncpy(new_str, dst->str, dst->len);
  free(dst->str);
  dst->capacity = new_capacity;
  dst->str = new_str;
}

void mstr_append_g(mstr *dst, char *src, size_t src_len)
{
  if (dst->capacity <= (dst->len + src_len))
  {
    mstr_reallocate(dst, dst->len + src_len);
  }

  assert(dst->capacity > (dst->len + src_len));

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
  mstr_reallocate(dst, (dst->len + len));
  assert(dst->capacity > (dst->len + len));

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
    mstr_reallocate(dst, dst->len + len);
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