/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_STERMINAL_H
#define ANDROID_STERMINAL_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <math.h>
#include <errno.h>

#include <hardware/hardware.h>
#include <cutils/native_handle.h>

__BEGIN_DECLS

/*****************************************************************************/

#define STERMINAL_HEADER_VERSION          1
#define STERMINAL_MODULE_API_VERSION_1_0  HARDWARE_MODULE_API_VERSION(1, 0)
#define STERMINAL_DEVICE_API_VERSION_1_0  HARDWARE_DEVICE_API_VERSION_2(1, 0, STERMINAL_HEADER_VERSION)

/*
 * The id of this module
 *  Name of the device to open
 */
#define STERMINAL_HARDWARE_MODULE_ID  "dummysterminal"

#define ST_PAGE_SIZE	16

typedef struct sterminal_str {
    uint8_t data[ST_PAGE_SIZE];
    int32_t len;
} sterminal_str_t;


typedef struct sterminal_device sterminal_device_t;
typedef struct __sterminal_dev_ops sterminal_dev_ops;

/**
 * Callback function which is called back when the data is available in the serial device
 * Can use to pass incomming data to the upper layers
 */
typedef void (*st_data_callback_t) (sterminal_str_t* c_data);

struct __sterminal_dev_ops {
	int (*st_init)(sterminal_device_t *hdev, st_data_callback_t data_cback); 	/** Returns no.of bytes writeen to the device */
	int (*st_write)(sterminal_device_t *hdev, sterminal_str_t *p_data); /** returns the no.of bytes read from the device */
};

struct sterminal_device {
	struct hw_device_t common;
	int st_fd;
	sterminal_dev_ops *ops;
};

__END_DECLS

#endif