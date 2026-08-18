/**
 *---------------------------------------------------------------------------
 * @brief    Some helper functions for jsmn JSON parser
 *
 * @file     jsmn_util.h
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2026-08-11
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    JSON_TYPE_STRING = 0,
    JSON_TYPE_NUMBER,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL,
    JSON_TYPE_OBJECT,
    JSON_TYPE_ARRAY,
    JSON_TYPE_END
} JSON_Type;

typedef struct {
    char name[32];
    JSON_Type type;
    union {
        char str[64];
        int num;
        bool boolean;
    } value;
} json_param;

#ifdef __cplusplus
extern "C" {
#endif

    int jsmn_test();

    int jsmn_parse_params(json_param* params, const char* json);

    void jsmn_print_params(json_param* params);

    void jsmn_params_to_json(const json_param* params, char* buffer, size_t buffer_size);

    int jsmn_set_param(json_param* params, const char* param_name, const void* value);

#ifdef __cplusplus
    }  // end brace for extern "C"
#endif
