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

#define mstr_new(SRC) _Generic((SRC), \
    char *: mstr_newc,                \
    int: mstr_newl,                   \
    mstr *: mstr_news)(SRC)

mstr *mstr_new_g(size_t size, size_t len, char *str);
mstr *mstr_newc(char *str);
mstr *mstr_news(mstr *mstr);
mstr *mstr_newl(size_t size);

// void mstr_clear(mstr *s);

#define mstr_append(DST, SRC) _Generic((SRC), \
    char *: mstr_append_c,                    \
    mstr *: mstr_append_s)(DST, SRC)
void mstr_append_g(mstr *dst, char *src, size_t len);
void mstr_append_c(mstr *dst, char *src);
void mstr_append_s(mstr *dst, mstr *src);

#define mstr_prepend(DST, SRC) _Generic((SRC), \
    char *: mstr_prepend_c,                    \
    mstr *: mstr_prepend_s)(DST, SRC)
void mstr_prepend_g(mstr *dst, char *src, size_t len);
void mstr_prepend_c(mstr *dst, char *src);
void mstr_prepend_s(mstr *dst, mstr *src);

// #define mstr_insert(DST, SRC, POS) _Generic((SRC),     \
//     char *: mstr_insert_g(DST, SRC, strlen(SRC), POS), \
//     mstr *: mstr_insert_g(DST, SRC->str, SRC->len, POS))

#define mstr_insert(DST, SRC, POS) _Generic((SRC), \
    char *: mstr_insert_c,                         \
    mstr *: mstr_insert_s)(DST, SRC, POS)
void mstr_insert_g(mstr *dst, char *src, size_t len, int pos);
void mstr_insert_c(mstr *dst, char *src, int pos);
void mstr_insert_s(mstr *dst, mstr *src, int pos);

// char *mstr_to_char(mstr *s);

void mstr_free(mstr *mstr);

// void mstr_replace(mstr *s, char *old, char *new);

void mstr_rstrip(mstr *s, char *st);
void mstr_lstrip(mstr *s, char *st);
void mstr_strip(mstr *s, char *st);

size_t mstr_len(mstr *s);

bool mstr_is_alpha(mstr *s);
bool mstr_is_numeric(mstr *s);
bool mstr_is_alnum(mstr *s);

void mstr_upper(mstr *s);
void mstr_lower(mstr *s);

void mstr_print(mstr *s);

#endif // _MSTR_H_

#ifdef __cplusplus
} // end brace for extern "C"
#endif
