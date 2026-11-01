/* SPDX-License-Identifier: MPL-2.0
 * THEORY-1: music theory for the M-VAVE FM-1. */
/* The app's half of the contract with the platform (base/device/unity.c, base/host/plat_host.c). */
#pragma once
#include <stdint.h>
#include "plat.h"

#define APP_NAME "THEORY-1"
#define APP_SPLASH_RGB RGB(70, 170, 255)
#define APP_PALETTE 2u                   /* gfx.c PALETTES: cyan */
#define APP_BLOCK 256u                   /* frames per audio block: 5.8 ms at 44.1 kHz */
#define APP_TOUCH                        /* velocity from the keys (lib/touch.h) */
#define APP_BTDEV                        /* Bluetooth MIDI devices (lib/btdev.h): a piano or keyboard over the air */
#define APP_BLE_AT_BOOT (!theory_ble_off())   /* Bluetooth at power-on: as the player left it (SETTINGS) */
#define APP_CANVAS_PX (240u * 120u)      /* the screen is drawn in two halves of 120 rows */
int theory_ble_off(void);

/* the store: the settings and records, the panel calibration (the platform's), Bluetooth's address and pairing */
enum { OBJ_SETTINGS, OBJ_PANEL, OBJ_BLE, OBJ_NOBJ };
#define APP_STORE_MAP {0xFC000u, 0xDC000u, 0xDE000u}

/* the contract: the platform calls these */
void app_init(void);
void app_render(int32_t *lr, uint32_t n, uint32_t master_q12);
void app_panic(void);
void app_ui_init(void);
void app_frame(void);
void app_input_only(void);
void app_say(const char *a, const char *b);

/* the host simulator's checks and commands (app/tests: the scripts) */
#define APP_HOST_EXPECT
int app_expect(const char *what, int *got);
#define APP_HOST_CMD
#define APP_HOST_FAILS                   /* texts that did not fit the screen */
int app_host_fails(void);
int app_host_cmd(const char *cmd, const char *a, const char *b, const char *c, const char *d);
