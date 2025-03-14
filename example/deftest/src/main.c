/**
 *---------------------------------------------------------------------------
 * @brief   Makeplates main example file.
 *
 * @file    main.c
 * @author  Peter Malmberg <peter.malmberg@gmail.com>
 * @date    2016-10-24
 * @licence GPLv2
 *
 *---------------------------------------------------------------------------
 */

// Includes ---------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#include "unity.h"

#include "def.h"
#include "def_util.h"
#include "def_linux.h"
#include "i2i.h"
#include "i2s.h"
#include "s2s.h"
#include "mstr.h"

// Defines ----------------------------------------------------------------

#define PROGNAME "makeplate"

// Variables --------------------------------------------------------------

// Prototypes -------------------------------------------------------------
void sigInt(int sig);
void sigHup(int sig);

void colorTest(void);
void I2S_test(void);
void S2S_test(void);
void I2I_test(void);
void defTest(void);
int unitTest(void);

// Code -------------------------------------------------------------------

// void escapeFilter(char *str) {
//     int len;
//     len = strlen(str);
// }

S2S fgColors[] = {
    {E_BLACK, "Black"},
    {E_RED, "Red"},
    {E_GREEN, "Green"},
    {E_YELLOW, "Yellow"},
    {E_BLUE, "Blue"},
    {E_MAGENTA, "Magenta"},
    {E_CYAN, "Cyan"},
    {E_GRAY, "Gray"},
    {E_DARKGRAY, "Darkgray"},
    {E_BR_RED, "Br Red"},
    {E_BR_GREEN, "Br Green"},
    {E_BR_YELLOW, "Br Yellow"},
    {E_BR_BLUE, "Br Blue"},
    {E_BR_MAGENTA, "Br Magenta"},
    {E_BR_CYAN, "Br Cyan"},
    {E_WHITE, "White"},
    {S2S_END}};

S2S bgColors[] = {
    {E_BG_BLACK, "Black"},
    {E_BG_RED, "Red"},
    {E_BG_GREEN, "Green"},
    {E_BG_YELLOW, "Yellow"},
    {E_BG_BLUE, "Blue"},
    {E_BG_MAGENTA, "Magenta"},
    {E_BG_CYAN, "Cyan"},
    {E_BG_WHITE, "White"},
    {S2S_END}};

void colorTest(void)
{
    int i, j;

    for (j = 0; j < S2S_len(bgColors); j++)
    {
        for (i = 0; i < S2S_len(fgColors); i++)
        {
            printf("%s%s %s " E_RESET, bgColors[j].key, fgColors[i].key, fgColors[i].value);
        }

        printf("\n");
    }

    printf("\n");
    printf(E_DIM "Low intensity text\n" E_RESET);
    printf(E_UNDERLINE "Underline text\n" E_RESET);
    printf(E_REVERSE "Reverse text\n" E_RESET);

    printf("\n");
}

void sigInt(int sig)
{
    UNUSED(sig);

    printf("\nExiting program\n");
    exit(0);
}

void sigHup(int sig)
{
    UNUSED(sig);
    printf("Sighup\n");
}

I2S numbersDb[] = {
    {1, "First"},
    {2, "Second"},
    {3, "Third"},
    {4, "Last"},
    {I2S_END},
};

void setUp(void)
{
}

void tearDown(void)
{
}

void I2S_test(void)
{
    I2S *db;

    db = I2S_copy(numbersDb);
    TEST_ASSERT_EQUAL_INT(2, I2S_findValue(db, "Third"));

    TEST_ASSERT_EQUAL_INT(4, I2S_len(numbersDb));
    TEST_ASSERT_EQUAL_STRING("Last", I2S_getValue(numbersDb, 4));
    TEST_ASSERT_EQUAL_INT(2, I2S_findKey(numbersDb, 3));
    TEST_ASSERT_EQUAL_INT(2, I2S_findValue(numbersDb, "Third"));

    I2S_setValue(numbersDb, 4, "Nisse");
    TEST_ASSERT_EQUAL_STRING("Nisse", I2S_getValue(numbersDb, 4));

    I2S_setKeyValue(numbersDb, 0, 22, "kv22");
    TEST_ASSERT_EQUAL_INT(0, I2S_findKey(numbersDb, 22));
    TEST_ASSERT_EQUAL_STRING("kv22", I2S_getValue(numbersDb, 22));

    db = I2S_new(12);
    TEST_ASSERT_EQUAL_INT(12, I2S_len(db));

    I2S_printDb(numbersDb);
}

int ia[] = {12, 33, 54, 11, 412, -443};

int ix[] = {44, 33, 54, 11, 412, -443};

void S2S_test(void)
{
    S2S *db;
    db = fgColors;

    TEST_ASSERT_EQUAL_INT(16, S2S_len(db));
    TEST_ASSERT_EQUAL_INT(6, S2S_findKey(db, E_CYAN));
    TEST_ASSERT_EQUAL_STRING("Cyan", S2S_getValue(db, E_CYAN));
    TEST_ASSERT_EQUAL_INT(0, S2S_first(db));
    TEST_ASSERT_EQUAL_INT(15, S2S_last(db));
}

i2i ii[] = {
    {1, 111},
    {2, 222},
    {3, 333},
    {4, 444},
    {5, 555},
    {I2I_END}};

void I2I_test(void)
{
    i2i *db;

    db = ii;
    TEST_ASSERT_EQUAL_INT(5, i2i_len(db));
    TEST_ASSERT_EQUAL_INT(1, i2i_findKey(db, 2));
    TEST_ASSERT_EQUAL_INT(4, i2i_findKey(db, 5));
    TEST_ASSERT_EQUAL_INT(333, i2i_getValue(db, 3));
    TEST_ASSERT_EQUAL_INT(0, i2i_first(db));
    TEST_ASSERT_EQUAL_INT(4, i2i_last(db));

    db = i2i_new(10);
    TEST_ASSERT_EQUAL_INT(10, i2i_len(db));

    db = i2i_copy(ii);
    TEST_ASSERT_EQUAL_INT(5, i2i_len(db));
    TEST_ASSERT_EQUAL_INT(1, i2i_findKey(db, 2));
    TEST_ASSERT_EQUAL_INT(4, i2i_findKey(db, 5));
    TEST_ASSERT_EQUAL_INT(333, i2i_getValue(db, 3));
    TEST_ASSERT_EQUAL_INT(0, i2i_first(db));
    TEST_ASSERT_EQUAL_INT(4, i2i_last(db));

    i2i_printDb(ii);
}

void defTest(void)
{
    TEST_ASSERT_EQUAL_INT(10, Max(10, 5));
    TEST_ASSERT_EQUAL_INT(10, Max(5, 10));
    TEST_ASSERT_EQUAL_INT(5, Max(5, -10));
    TEST_ASSERT_EQUAL_INT(5, Max(-10, 5));

    TEST_ASSERT_EQUAL_INT(5, Min(10, 5));
    TEST_ASSERT_EQUAL_INT(5, Min(5, 10));
    TEST_ASSERT_EQUAL_INT(-10, Min(5, -10));
    TEST_ASSERT_EQUAL_INT(-10, Min(-10, 5));

    TEST_ASSERT_EQUAL_INT(485, Abs(485));
    TEST_ASSERT_EQUAL_INT(485, Abs(-485));

    TEST_ASSERT_EQUAL_INT(42, Clamp(42, -100, 100));
    TEST_ASSERT_EQUAL_INT(100, Clamp(142, -100, 100));
    TEST_ASSERT_EQUAL_INT(-42, Clamp(-42, -100, 100));
    TEST_ASSERT_EQUAL_INT(-100, Clamp(-142, -100, 100));

    TEST_ASSERT_EQUAL_INT(0xF100, Swap16(0x00F1));
    TEST_ASSERT_EQUAL_INT(0xCDAB0000, Swap32(0x0000ABCD));
    TEST_ASSERT_EQUAL_INT(0xDDCCBBAA00000000, Swap64(0x00000000AABBCCDD));

    TEST_ASSERT_TRUE(isWithin(5, -10, 10));
    TEST_ASSERT_TRUE(isWithin(-5, -10, 10));
    TEST_ASSERT_FALSE(isWithin(11, -10, 10));
    TEST_ASSERT_FALSE(isWithin(-11, -10, 10));
    TEST_ASSERT_TRUE(isOutside(15, -10, 10));
    TEST_ASSERT_TRUE(isOutside(-15, -10, 10));
    TEST_ASSERT_FALSE(isOutside(-5, -10, 10));
    TEST_ASSERT_FALSE(isOutside(5, -10, 10));
}

int unitTest(void)
{

    printf("Swap %X\n", Swap16(0xFF00));

    UNITY_BEGIN();
    RUN_TEST(I2S_test);
    RUN_TEST(S2S_test);
    RUN_TEST(I2I_test);
    RUN_TEST(defTest);

    return UNITY_END();
}
bool is_null_terminated(mstr *str)
{
    if (str->str[str->len] == '\0')
        return true;

    return false;
}
// void mstr_printx(mstr *s, char *cmd)
void mstr_printx(mstr *s, char *cmd, const char *fmt, ...)
{
    char buf[256];
    va_list args;
    UNUSED(args);

    if (s == NULL)
    {
        printf(" Length  Capacity  Alpha  Numeric Alfanumeric Space Empty NT Command    String\n");
        return;
    }

    printf(" %4ld     %4ld       %d       %d         %d        %d     %d    %d %-10s   \"%s\"\n", s->len, s->capacity, mstr_is_alpha(s), mstr_is_numeric(s), mstr_is_alnum(s), mstr_is_space(s), mstr_is_empty(s), is_null_terminated(s), fmt, s->str);

    // Alternative mode
    // printf(buf, " %4ld %4ld %d %d  %d %d  %-10s \"%s\"", s->size, s->len, mstr_is_alpha(s), mstr_is_numeric(s), mstr_is_alnum(s), mstr_is_space(s), cmd, s->str);

    // va_start(args, fmt);
    // vfprintf(fmt, args);
    // for (int i = 0; i < args; i++)

    // printf(" %4ld %4ld %d %d  %d %d  %-10s \"%s\"\n", s->size, s->len, mstr_is_alpha(s), mstr_is_numeric(s), mstr_is_alnum(s), mstr_is_space(s), fmt, s->str);
    // printf(" %4ld %4ld %d %d  %d %d  %-10s \"%s\"\n", s->size, s->len, mstr_is_alpha(s), mstr_is_numeric(s), mstr_is_alnum(s), mstr_is_space(s), fmt, s->str);
    // #printf("%s", buf);
    // va_end(args);
}

int mstr_test()
{
    mstr *again = mstr_new(" Again! ");
    mstr *num = mstr_new("1234567890");
    mstr *ml = mstr_new(512);

    printTextLine("msrt test");
    printf("\nMSTR_BLOCK: %2d\nMSTR_EXTRA: %2d\n\n", MSTR_BLOCK, MSTR_EXTRA);

    mstr_printx(NULL, "", "");

    mstr *ms = mstr_new("");
    mstr_printx(ms, "new", "");

    mstr_append(ms, "world");
    mstr_printx(ms, "append", "");

    mstr_prepend(ms, "Hello ");
    mstr_printx(ms, "prepend", "");

    mstr_append(ms, "! Some more text to trigger reallocation.");
    mstr_printx(ms, "append", "");

    mstr *mc = mstr_new(ms);
    mstr_printx(mc, "new copy", "");

    // mstr_prepend(ms, "Prepended: ");
    // mstr_print(ms);

    // mstr_prepend(ms, again);
    // mstr_print(ms);

    mstr *strip = mstr_new("    Pleace strip   some for   me!!        ");
    mstr_printx(strip, "new", "");
    mstr_strip(strip, " ");
    mstr_printx(strip, "strip", "");

    mstr *strip2 = mstr_new("   Some other striping    ");
    mstr_printx(strip2, "new", "");
    mstr_lstrip(strip2, " ");
    mstr_printx(strip2, "lstrip", "");
    mstr_rstrip(strip2, " ");
    mstr_printx(strip2, "rstrip", "");

    mstr *astrip = mstr_new("  ,!! ..   A more...advanced,,, stripping..  ..,,,     ");
    mstr_printx(astrip, "new", "");
    mstr_strip(astrip, " ,.!");
    mstr_printx(astrip, "strip", "");

    mstr *insert = mstr_new("This incomplete!");
    mstr_printx(insert, "new", "");
    mstr_insert(insert, "text ", 5);
    mstr_printx(insert, "insert", "");
    mstr *insert2 = mstr_new("was ");
    mstr_insert(insert, insert2, 10);
    mstr_printx(insert, "insert", "");

    // mstr *insertbehind = mstr_new("Insert behind!");
    // mstr_print(insertbehind);
    // mstr_insert(insertbehind, "from ", -8);
    // mstr_print(insertbehind);
    mstr *insert3 = mstr_new("235679");
    mstr_printx(insert3, "new", "");

    mstr_insert(insert3, "1", 0);
    mstr_printx(insert3, "insert", "");
    mstr_insert(insert3, "4", 3);
    mstr_printx(insert3, "insert", "");
    mstr_insert(insert3, "8", -1);
    mstr_printx(insert3, "insert", "");

    mstr *upplow = mstr_new("HeLlo WoRlD");
    mstr_printx(upplow, "new", "");
    mstr_upper(upplow);
    mstr_printx(upplow, "upper", "");
    mstr_lower(upplow);
    mstr_printx(upplow, "lower", "");

    // mstr *space = mstr_new("    ");
    // mstr_printx(space);

    mstr *replace = mstr_new("A field with a flower, a tree and a bush");
    mstr_printx(replace, "new", "");
    mstr_replace(replace, "a tree", "oak");
    mstr_printx(replace, "replace", "");

    // mstr *number = mstr_new("091234");
    // mstr_printx(replace, "Tro")

    mstr_clear(ms);
    mstr_printx(ms, "clear", "");

    mstr *no_termination = mstr_new("No termination");
    no_termination->str[no_termination->len] = '!';
    no_termination->str[no_termination->len + 1] = '\0';
    mstr_printx(no_termination, "", "");

    TEST_ASSERT_TRUE_MESSAGE(mstr_startwidth(upplow, "hello"), "Startwidth failed");
    TEST_ASSERT_FALSE_MESSAGE(mstr_startwidth(upplow, "XXXhello"), "Startwidth failed");
}

int main(int argc, char *argv[])
{
    int x;
    char buf[64];

    UNUSED(argc);
    UNUSED(argv);

    unitTest();

    mstr_test();

    printTextLine("Info");
    printSysInfo();
    printf("Path to program: %s\n", getPathToSelf());

    printTextLine("Bits");
    printf("Binary %s\n", int2bin(buf, 0xAA, 8));
    printf("Binary %s\n", int2bin(buf, 0xFF, 8));
    printf("Binary %s\n", int2bin(buf, 0xAAAA, 16));
    printf("Binary %s\n", int2bin(buf, 0xAAFFAAFF, 32));
    printf("Binary %s\n", int2bin(buf, 0xAAFFAAFF, 48));
    printf("Builtin Binary %b\n", 0xAAFFAAFF);
    printf("Builtin Binary %b\n", 0b10101010);
    printf("Builtin Binary %016b\n", 0b10101010);

    /*
        printf("|||||||+--\n");
        printf("||||||+---\n");
        printf("|||||+----\n");
        printf("||||+-----\n");
        printf("|||+------\n");
        printf("||+-------\n");
        printf("|+--------\n");
        printf("+---------\n");
    */

    x = 0;
    printf("x = %s\n", int2bin(buf, x, 8));
    Set_bits(x, 0x02);
    printf("x = %s\n", int2bin(buf, x, 8));

    Set_bits(x, 0xF0);
    printf("x = %s\n", int2bin(buf, x, 8));

    Clr_bits(x, 0xA0);
    printf("x = %s\n", int2bin(buf, x, 8));

    Tgl_bits(x, 0x08);
    printf("x = %s\n", int2bin(buf, x, 8));

    //  x = bit_reverse8(x);
    //  printf("x = %s\n", int2bin(x, 8));

    // printLine();
    // printTextLine("Kalle");
    // printTextLine("A test");

    return 0;
}
