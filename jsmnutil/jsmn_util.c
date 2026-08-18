/**
 *---------------------------------------------------------------------------
 * @brief    Some helper functions for jsmn JSON parser
 *
 * @file     jsmn_util.c
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2026-08-11
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

#include "jsmn_util.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsmn.h"

static bool strcasecmp_simple(const char* a, const char* b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b))
            return false;
        a++;
        b++;
    }
    return *a == *b;
}

static bool jsoneq(const char* json, jsmntok_t* tok, const char* s) {
    if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return true;
    }
    return false;
}

static void jsmn_get_token(char* dst, int idx, const jsmntok_t* tokens,
                           const char* json) {
    size_t len = tokens[idx].end - tokens[idx].start;
    strncpy(dst, json + tokens[idx].start, len);
    dst[len] = '\0';
}


static int jsmn_find_token(const char* json, jsmntok_t* tokens, int num_tokens,
                           const char* key) {
    for (int i = 0; i < num_tokens; i++) {
        if (tokens[i].type == JSMN_STRING && jsoneq(json, &tokens[i], key)) {
            return i;
        }
    }
    return -1;  // Key not found
}

static int jsmn_get_int(int* value, const char* key, const char* json,
                        jsmntok_t* tokens, int num_tokens, int default_value) {
    *value = default_value;
    int idx = jsmn_find_token(json, tokens, num_tokens, key);
    if (idx < 0) {
        return -1;  // Key not found
    }

    if (tokens[idx + 1].type == JSMN_PRIMITIVE) {
        char value_str[32];
        jsmn_get_token(value_str, idx + 1, tokens, json);
        *value = atoi(value_str);
        return 0;
    }

    return -2;  // Value is not an integer
}

static int jsmn_get_string(char* value, const char* key, const char* json,
                           jsmntok_t* tokens, int num_tokens,
                           const char* default_value) {
    strcpy(value, default_value);
    // value[value_size - 1] = '\0';  // Ensure null-termination
    int idx = jsmn_find_token(json, tokens, num_tokens, key);
    if (idx < 0) {
        return -1;  // Key not found
    }
    if (tokens[idx + 1].type == JSMN_STRING) {
        jsmn_get_token(value, idx + 1, tokens, json);
        return 0;
    }
    return -2;  // Value is not a string
}

static int jsmn_get_boolean(bool* value, const char* key, const char* json,
                            jsmntok_t* tokens, int num_tokens,
                            bool default_value) {
    char token[64];
    *value = default_value;

    int idx = jsmn_find_token(json, tokens, num_tokens, key);
    if (idx < 0) {
        return -1;  // Key not found
    }

    if (tokens[idx + 1].type == JSMN_PRIMITIVE) {
        jsmn_get_token(token, idx + 1, tokens, json);
        if (strcasecmp_simple(token, "true")) {
            *value = true;
            return 0;
        } else if (strcasecmp_simple(token, "false")) {
            *value = false;
            return 0;
        }
    }
    return -2;  // Value is not boolean
}

static void jsmn_print_tokens(const char* json, const jsmntok_t* tokens,
                              int num_tokens) {
    for (int i = 0; i < num_tokens; i++) {
        const jsmntok_t* tok = &tokens[i];
        printf("Token %2d: type=%d, start=%3d, end=%3d, size=%3d  ", i,
               tok->type, tok->start, tok->end, tok->size);

        switch (tok->type) {
            case JSMN_OBJECT:
                printf("Object:    \n%.*s\n", tok->end - tok->start,
                       json + tok->start);
                break;

            case JSMN_ARRAY:
                printf("Array:     %.*s\n", tok->end - tok->start,
                       json + tok->start);
                break;

            case JSMN_STRING:
                printf("String:    %.*s\n", tok->end - tok->start,
                       json + tok->start);
                break;

            case JSMN_PRIMITIVE:
                printf("Primitive: %.*s\n", tok->end - tok->start,
                       json + tok->start);
                break;

            default:
                printf("Unknown:   %.*s\n", tok->end - tok->start,
                       json + tok->start);
                break;
        }
    }
}


int jsmn_test() {
    jsmn_parser parser;
    jsmntok_t tokens[64];
    int ret;
    static const char* JSON_TEST =
        "{\n"
        "  \"string1\": \"Hello, World!\",\n"
        "  \"number1\": 42,\n"
        "  \"number2\": 123456789,\n"
        "  \"number3\": -765634,\n"
        "  \"bool1\": true\n"
        "  \"bool2\": True\n"
        "  \"bool3\": TRUE\n"
        "  \"bool4\": false\n"
        "  \"bool5\": False\n"
        "  \"bool6\": FALSE\n"
        "  \"array\": [1, 2, 3],\n"
        "  \"section1\": {\n"
        "    \"key1\": \"value1\",\n"
        "    \"key2\": \"value2\"\n"
        "  },\n"
        "  \"blaha\": \"xxx\",\n"
        "  \"section2\": {\n"
        "    \"key3\": \"value3\",\n"
        "    \"key4\": \"value4\"\n"
        "  },\n"
        "}\n";

    jsmn_init(&parser);

    ret = jsmn_parse(&parser, JSON_TEST, strlen(JSON_TEST), tokens,
                     sizeof(tokens) / sizeof(tokens[0]));
    if (ret < 0) {
        printf("Failed to parse JSON: %d\n", ret);
        return -1;
    }

    jsmn_print_tokens(JSON_TEST, tokens, ret);

    char string_value[64];
    jsmn_get_string(string_value, "string1", JSON_TEST, tokens, ret,
                    "default_string");
    printf("string1: %s\n", string_value);

    int number_value;
    jsmn_get_int(&number_value, "number1", JSON_TEST, tokens, ret, 0);
    printf("number1: %d\n", number_value);
    jsmn_get_int(&number_value, "number2", JSON_TEST, tokens, ret, 0);
    printf("number2: %d\n", number_value);
    jsmn_get_int(&number_value, "number3", JSON_TEST, tokens, ret, 0);
    printf("number3: %d\n", number_value);
    jsmn_get_int(&number_value, "number_not_existing", JSON_TEST, tokens, ret,
                 -555);
    printf("number_not_existing: %d\n", number_value);

    bool boolx;
    jsmn_get_boolean(&boolx, "bool1", JSON_TEST, tokens, ret, false);
    printf("bool1: %s\n", boolx ? "true" : "false");
    jsmn_get_boolean(&boolx, "bool2", JSON_TEST, tokens, ret, false);
    printf("bool2: %s\n", boolx ? "true" : "false");
    jsmn_get_boolean(&boolx, "bool3", JSON_TEST, tokens, ret, false);
    printf("bool3: %s\n", boolx ? "true" : "false");
    jsmn_get_boolean(&boolx, "bool4", JSON_TEST, tokens, ret, true);
    printf("bool4: %s\n", boolx ? "true" : "false");
    jsmn_get_boolean(&boolx, "bool5", JSON_TEST, tokens, ret, true);
    printf("bool5: %s\n", boolx ? "true" : "false");
    jsmn_get_boolean(&boolx, "bool6", JSON_TEST, tokens, ret, true);
    printf("bool6: %s\n", boolx ? "true" : "false");

    return 0;
}

int jsmn_parse_params(json_param* params, const char* json) {
    jsmn_parser parser;
    jsmntok_t tokens[64];
    int num_tokens = sizeof(tokens) / sizeof(tokens[0]);
    int ret;
    int i = 0;

    jsmn_init(&parser);
    ret = jsmn_parse(&parser, json, strlen(json), tokens, 64);
    if (ret < 0) {
        printf("Failed to parse JSON: %d\n", ret);
        return -1;
    }
    jsmn_print_tokens(json, tokens, ret);
    // for (int i = 0; i < num_params; i++) {
    while (params[i].type != JSON_TYPE_END) {
        json_param* param = &params[i];
        switch (param->type) {
            case JSON_TYPE_STRING:
                jsmn_get_string(param->value.str, param->name, json, tokens, num_tokens, "");
                break;
            case JSON_TYPE_NUMBER:
                jsmn_get_int(&param->value.num, param->name, json, tokens, num_tokens, 0);
                break;
            case JSON_TYPE_BOOLEAN:
                jsmn_get_boolean(&param->value.boolean, param->name, json, tokens, num_tokens, false);
                break;
            case JSON_TYPE_OBJECT:
                // For objects, we can store the number of nested items (size) in value.num
            default:
                // Handle other types if needed
                break;
        }
        i++;
    }
    return 0;
}

void jsmn_print_params(json_param* params) {
    int i = 0;
    while(params[i].type != JSON_TYPE_END) {
        json_param* param = &params[i];
        printf("Parameter: %-8s Type: %1d, Value: ", param->name, param->type);
        switch (param->type) {
            case JSON_TYPE_STRING:
                printf("%s\n", param->value.str);
                break;
            case JSON_TYPE_NUMBER:
                printf("%d\n", param->value.num);
                break;
            case JSON_TYPE_BOOLEAN:
                printf("%s\n", param->value.boolean ? "true" : "false");
                break;
            case JSON_TYPE_OBJECT:
                printf("Object with %d nested items\n", param->value.num);
                break;
            default:
                printf("Unknown type\n");
                break;
        }
        i++;
    }
}


// void jsmn_print_params(json_param* params, int num_params) {
//     for (int i = 0; i < num_params; i++) {
//         json_param* param = &params[i];
//         printf("Parameter: %-8s Type: %1d, Value: ", param->name, param->type);
//         switch (param->type) {
//             case JSON_TYPE_STRING:
//                 printf("%s\n", param->value.str);
//                 break;
//             case JSON_TYPE_NUMBER:
//                 printf("%d\n", param->value.num);
//                 break;
//             case JSON_TYPE_BOOLEAN:
//                 printf("%s\n", param->value.boolean ? "true" : "false");
//                 break;
//             case JSON_TYPE_OBJECT:
//                 printf("Object with %d nested items\n", param->value.num);
//                 break;
//             default:
//                 printf("Unknown type\n");
//                 break;
//         }
//     }
// }


void jsmn_params_to_json(const json_param* params, char* buffer, size_t buffer_size) {
// void jsmn_params_to_json(const json_param* params, int num_params, char* buffer, size_t buffer_size) {
    size_t offset = 0;
    int object_count = 0;
    int i = 0;
    offset += snprintf(buffer + offset, buffer_size - offset, "{\n");
    // for (int i = 0; i < num_params; i++) {
    while (params[i].type != JSON_TYPE_END) {
        const json_param* param = &params[i];
        if ((i > 0) && (params[i - 1].type != JSON_TYPE_OBJECT || object_count == 0)) {    
            offset += snprintf(buffer + offset, buffer_size - offset, ",\n");
        }
        if (object_count > 0) {
            offset += snprintf(buffer + offset, buffer_size - offset, "  ");
        }
      
        switch (param->type) {
            case JSON_TYPE_OBJECT:
                offset += snprintf(buffer + offset, buffer_size - offset, "  \"%s\": {\n", param->name);
                object_count = param->value.num+1;  // Store the number of nested objects
                // printf("Object count: %d  value.num: %d\n", object_count, param->value.num);
                break;
            case JSON_TYPE_STRING:
                offset += snprintf(buffer + offset, buffer_size - offset, "  \"%s\": \"%s\"", param->name, param->value.str);
                break;
            case JSON_TYPE_NUMBER:
                offset += snprintf(buffer + offset, buffer_size - offset, "  \"%s\": %d", param->name, param->value.num);
                break;
            case JSON_TYPE_BOOLEAN:
                offset += snprintf(buffer + offset, buffer_size - offset, "  \"%s\": %s", param->name,
                                   param->value.boolean ? "true" : "false");
                break;
            default:
                // Handle other types if needed
                break;
        }

        if (object_count == 1) {
            offset += snprintf(buffer + offset, buffer_size - offset, "\n  }");
        }
        if (object_count > 0) {
            object_count--;
            // printf("-- %d\n", object_count);
        }
        i++;
    }
    snprintf(buffer + offset, buffer_size - offset, "\n}\n");
}

int jsmn_set_param(json_param* params, const char* param_name, const void* value) {

    // for (int i = 0; i < num_params; i++) {
    int i = 0;
    while (params[i].type != JSON_TYPE_END) {
        if (strcmp(params[i].name, param_name) != 0) {
            i++;
            continue;
        }
        switch (params[i].type) {
            case JSON_TYPE_STRING:
                strncpy(params[i].value.str, (const char*)value, sizeof(params[i].value.str) - 1);
                params[i].value.str[sizeof(params[i].value.str) - 1] = '\0';  // Ensure null-termination
                break;
            case JSON_TYPE_NUMBER:
                params[i].value.num = *(const int*)value;
                break;
            case JSON_TYPE_BOOLEAN:
                params[i].value.boolean = *(const bool*)value;
                break;
            default:
                // Handle other types if needed
                break;
        }
        return 0;  // Exit after setting the parameter
    }
        // json_param* param = &params[i];}
    return -1;  // Parameter not found        
}