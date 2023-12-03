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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mstr.h"
#include <stdbool.h>
#include <stdarg.h>

mstr *mstr_new_g(size_t size, size_t len, char *str)
{
  mstr *mst = (mstr *)malloc(sizeof(mstr));
  mst->str = (char *)malloc(size);
  mst->size = size;
  mst->len = len;
  strncpy(mst->str, str, len);
  return mst;
}

mstr *mstr_newc(char *str)
{
  return mstr_new_g(MSTR_SIZE, strlen(str), str);
}

mstr *mstr_news(mstr *src)
{
  return mstr_new_g(MSTR_SIZE, src->len, src->str);
}

mstr *mstr_newl(size_t size)
{
  return mstr_new_g(size, 0, "");
}

void mstr_append_g(mstr *dst, char *src, size_t len)
{
  assert(dst->size > (dst->len + len));
  strncpy(dst->str + dst->len, src, len);
  dst->len += len;
  dst->str[dst->len] = '\0';
}

void mstr_appendc(mstr *mst, char *astr)
{
  mstr_append_g(mst, astr, strlen(astr));
}

void mstr_appends(mstr *mst, mstr *astr)
{
  mstr_append_g(mst, astr->str, astr->len);
}

void mstr_prepend_g(mstr *dst, char *src, size_t len)
{
  assert(dst->size > (dst->len + len));
  for (int i = dst->len - 1; i >= 0; i--)
  {
    dst->str[i + len] = dst->str[i];
  }

  for (size_t i = 0; i < len; i++)
  {
    dst->str[i] = src[i];
  }
  dst->len += len;
  dst->str[dst->len] = '\0';
}

void mstr_prepend_c(mstr *dst, char *src)
{
  mstr_prepend_g(dst, src, strlen(src));
}

void mstr_prepend_s(mstr *dst, mstr *src)
{
  mstr_prepend_g(dst, src->str, src->len);
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

bool mstr_is_numeric(mstr *s)
{
  for (size_t i = 0; i < s->len; i++)
  {
    if (s->str[i] < '0' || s->str[i] > '9')
      return false;
  }
  return true;
}

void mstr_print(mstr *s)
{
  if (s == NULL)
  {
    printf("size  length  N  string\n");
    return;
  }

  printf("%4ld  %6ld  %d  \"%s\"\n", s->size, s->len, mstr_is_numeric(s), s->str);
  // printf("size: %4ld  length: %4ld  %d  str: \"%s\"\n", s->size, s->len, mstr_is_numeric(s), s->str);
}