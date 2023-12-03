/**
 *---------------------------------------------------------------------------
 * @brief    A advanced string library
 *
 * @file     mstr.h
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2023-11-29
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

#ifndef _MSTR_H_
#define _MSTR_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C"
#endif

#define MSTR_SIZE 1024

    typedef struct mstr_t
{
  char *str;
  size_t size;
  size_t len;
} mstr;

#define mstr_new(S) _Generic((S), \
    char *: mstr_newc,            \
    int: mstr_newl,               \
    mstr *: mstr_news)(S)

mstr *mstr_new_g(size_t size, size_t len, char *str);
mstr *mstr_newc(char *str);
mstr *mstr_news(mstr *mstr);
mstr *mstr_newl(size_t size);

// void mstr_clear(mstr *s);

#define mstr_append(D, S) _Generic((S), \
    char *: mstr_appendc,               \
    mstr *: mstr_appends)(D, S)
void mstr_append_g(mstr *dst, char *src, size_t len);
void mstr_appendc(mstr *dst, char *src);
void mstr_appends(mstr *dst, mstr *src);

#define mstr_prepend(D, S) _Generic((S), \
    char *: mstr_prepend_c,              \
    mstr *: mstr_prepend_s)(D, S)
void mstr_prepend_g(mstr *dst, char *src, size_t len);
void mstr_prepend_c(mstr *dst, char *src);
void mstr_prepend_s(mstr *dst, mstr *src);

// char *mstr_to_char(mstr *s);

void mstr_free(mstr *mstr);

// void mstr_replace(mstr *s, char *old, char *new);

void mstr_rstrip(mstr *s, char *st);
void mstr_lstrip(mstr *s, char *st);
void mstr_strip(mstr *s, char *st);

size_t mstr_len(mstr *s);

bool mstr_is_numeric(mstr *s);
// bool mstr_is_alnum(mstr *s);
// bool mstr_is_alpha(mstr *s);

void mstr_print(mstr *s);

#endif // _MSTR_H_

#ifdef __cplusplus
} // end brace for extern "C"
#endif
