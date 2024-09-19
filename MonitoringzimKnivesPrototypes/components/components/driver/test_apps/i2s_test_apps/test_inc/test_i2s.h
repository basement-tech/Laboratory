/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SAMPLE_RATE     (48000)
#define SAMPLE_BITS     (16)

#if CONFIG_IDF_TARGET_ESP32
#define MASTER_MCK_IO 0
#define MASTER_BCK_IO 4
#define MASTER_WS_IO 5
#define SLAVE_BCK_IO 21
#define SLAVE_WS_IO 22
#define DATA_IN_IO 19
#define DATA_OUT_IO 18
#elif CONFIG_IDF_TARGET_ESP32S2
#define MASTER_MCK_IO 0
#define MASTER_BCK_IO 4
#define MASTER_WS_IO 5
#define SLAVE_BCK_IO 14
#define SLAVE_WS_IO 15
#define DATA_IN_IO 19
#define DATA_OUT_IO 18
#elif CONFIG_IDF_TARGET_ESP32C3
#define MASTER_MCK_IO 0
#define MASTER_BCK_IO 4
#define MASTER_WS_IO 5
#define SLAVE_BCK_IO 14
#define SLAVE_WS_IO 15
#define DATA_IN_IO 19
#define DATA_OUT_IO 18
#elif CONFIG_IDF_TARGET_ESP32S3
#define MASTER_MCK_IO 0
#define MASTER_BCK_IO 4
#define MASTER_WS_IO 5
#define SLAVE_BCK_IO 14
#define SLAVE_WS_IO 15
#define DATA_IN_IO 19
#define DATA_OUT_IO 18
#elif CONFIG_IDF_TARGET_ESP32P4
#define MASTER_MCK_IO 34
#define MASTER_BCK_IO 35
#define MASTER_WS_IO 48
#define SLAVE_BCK_IO 10
#define SLAVE_WS_IO 11
#define DATA_IN_IO 12
#define DATA_OUT_IO 49
#else
#define MASTER_MCK_IO 0
#define MASTER_BCK_IO 4
#define MASTER_WS_IO 5
#define SLAVE_BCK_IO 2
#define SLAVE_WS_IO 3
#define DATA_IN_IO 6
#define DATA_OUT_IO 7
#endif

#ifdef __cplusplus
}
#endif
