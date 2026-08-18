/**
 *---------------------------------------------------------------------------
 * @brief    Some simple support functions for jsmn
 *
 * @file     main.c
 * @author   Peter Malmberg <peter.malmberg@gmail.com>
 * @version  0.01
 * @date     2026-08-08
 * @license  MIT
 *
 *---------------------------------------------------------------------------
 *
 *
 */

// Include ------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "main.h"
#include "jsmn_util.h"
#include "def.h"

// Macros -------------------------------------------------------------------


// Prototypes ---------------------------------------------------------------


// Datatypes ----------------------------------------------------------------


// Variables ----------------------------------------------------------------


// Code ---------------------------------------------------------------------

json_param parameters[] = {
    {"user", JSON_TYPE_STRING, .value.str = "johndoe"},
    {"admin", JSON_TYPE_BOOLEAN, .value.boolean = false},
    {"network", JSON_TYPE_OBJECT, .value.num = 4},  // Placeholder for nested object
    {"dhcp", JSON_TYPE_BOOLEAN, .value.boolean = true},
    {"ip", JSON_TYPE_STRING, .value.str = "192.168.1.100"},
    {"netmask", JSON_TYPE_STRING, .value.str = "255.255.255.0"},
    {"gateway", JSON_TYPE_STRING, .value.str = "192.168.1.1"},
    {"uid", JSON_TYPE_NUMBER, .value.num = 1200},
    {"gid", JSON_TYPE_NUMBER, .value.num = 2400},
    {"lcd", JSON_TYPE_OBJECT, .value.num = 4},  // Placeholder for nested object
    {"lcd_width", JSON_TYPE_NUMBER, .value.num = 16},
    {"lcd_height", JSON_TYPE_NUMBER, .value.num = 2},
    {"lcd_backlight", JSON_TYPE_BOOLEAN, .value.boolean = true},
    {"lcd_active", JSON_TYPE_BOOLEAN, .value.boolean = true},
    {"end", JSON_TYPE_END, .value.num = 0}  // End marker
};

static const char* JSON_TEST =
    "{\n"
    "  \"user\": \"johndoe\",\n"
    "  \"admin\": false,\n"
    "  \"network\": {\n"
    "    \"dhcp\": true,\n"
    "    \"ip\": \"192.168.1.100\",\n"
    "    \"netmask\": \"255.255.255.0\",\n"
    "    \"gateway\": \"192.168.1.1\",\n"
    "  },\n"
    "  \"lcd\": {\n"
    "    \"width\": 16,\n"
    "    \"height\": 2,\n"
    "    \"backlight\": true,\n"
    "    \"active\": true\n"
    "  },\n"
    "  \"uid\": 1000,\n"
    "  \"gid\": 2200,\n" 
    "}\n";

int main(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    char buffer[512];

    jsmn_parse_params(parameters, JSON_TEST);
    jsmn_print_params(parameters);

    jsmn_params_to_json(parameters, buffer, sizeof(buffer));
    printf(buffer);

    jsmn_set_param(parameters, "user", "janedoe");
    jsmn_set_param(parameters, "admin", &(bool){true});
    jsmn_set_param(parameters, "uid", &(int){1234});
    jsmn_set_param(parameters, "dhcp", &(bool){false});
    jsmn_set_param(parameters, "ip", "172.0.0.23");
    jsmn_set_param(parameters, "netmask", "255.0.0.0");
    jsmn_set_param(parameters, "gateway", "172.0.0.1");

    jsmn_params_to_json(parameters, buffer, sizeof(buffer));
    printf(buffer);
    return 0;
}
