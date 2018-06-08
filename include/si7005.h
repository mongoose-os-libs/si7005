/*
 * Copyright (c) 2014-2018 Cesanta Software Limited
 * All rights reserved
 *
 * Licensed under the Apache License, Version 2.0 (the ""License"");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an ""AS IS"" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <stdbool.h>

#include "mgos_i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Simple interface: blocking methods that use global I2C instance
 * and wait for conversion (conversion takes approx 35 ms, 2x for RH).
 */

#define SI7005_INVALID_VALUE -1000.0

/* Returns true if a sensor is present on the bus. */
bool si7005_probe(void);

/* Performs conversion and returns temperature, in C, or INVALID_VALUE */
float si7005_read_temp(void);

/*
 * Performs conversion and returns relative humidity, in %, or INVALID_VALUE.
 * Note: Performs temperature measurement prior to the RH to perform temperature
 * compensation.
 */
float si7005_read_rh(void);

/* Turns the built-in heater element on or off. */
bool si7005_set_heater(bool on);

/* Variants of the simple blocking methods above with custom bus. */
bool si7005_probe_bus(struct mgos_i2c *i2c);
float si7005_read_temp_bus(struct mgos_i2c *i2c);
float si7005_read_rh_bus(struct mgos_i2c *i2c);
bool si7005_set_heater_bus(struct mgos_i2c *i2c, bool on);

/* Advanced interface: allows performing async conversions. */

/* Start temperature or humidity conversion. */
bool si7005_start_conversion(struct mgos_i2c *i2c, bool temp, bool fast);

/* Check if the data is ready. */
bool si7005_is_data_ready(struct mgos_i2c *i2c);

/*
 * Read out the data.
 * Note that RH value is linearized but not temp-compensated.
 */
float si7005_read_data(struct mgos_i2c *i2c, bool temp);

/* Perform temperature compensation of the RH reading. */
float si7005_rh_tcomp(float rh_val, float temp);

#ifdef __cplusplus
}
#endif
