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

/* https://www.silabs.com/documents/public/data-sheets/Si7005.pdf */

#include "si7005.h"

#define SI7005_ADDR 0x40

#define SI7005_REG_STATUS 0x0
#define SI7005_REG_STATUS_nRDY (1 << 0)

#define SI7005_REG_DATA 0x1

#define SI7005_REG_CONFIG 0x3
#define SI7005_REG_CONFIG_START (1 << 0)
#define SI7005_REG_CONFIG_HEAT (1 << 1)
#define SI7005_REG_CONFIG_TEMP (1 << 4)
#define SI7005_REG_CONFIG_FAST (1 << 5)

#define SI7005_REG_ID 0x11
#define SI7005_REG_ID_VALUE 0x50

bool si7005_probe(void) {
  return si7005_probe_bus(mgos_i2c_get_global());
}

float si7005_read_rh(void) {
  return si7005_read_rh_bus(mgos_i2c_get_global());
}

float si7005_read_temp(void) {
  return si7005_read_temp_bus(mgos_i2c_get_global());
}

bool si7005_set_heater(bool on) {
  return si7005_set_heater_bus(mgos_i2c_get_global(), on);
}

bool si7005_probe_bus(struct mgos_i2c *i2c) {
  if (i2c == NULL) return false;
  return mgos_i2c_read_reg_b(i2c, SI7005_ADDR, SI7005_REG_ID) ==
         SI7005_REG_ID_VALUE;
}

float si7005_read_temp_bus(struct mgos_i2c *i2c) {
  if (!si7005_probe_bus(i2c)) return SI7005_INVALID_VALUE;
  if (!si7005_start_conversion(i2c, true /* temp */, false /* fast */)) {
    return SI7005_INVALID_VALUE;
  }
  while (!si7005_is_data_ready(i2c)) {
  }
  return si7005_read_data(i2c, true /* temp */);
}

float si7005_read_rh_bus(struct mgos_i2c *i2c) {
  if (!si7005_probe_bus(i2c)) return SI7005_INVALID_VALUE;
  float temp = si7005_read_temp_bus(i2c);
  if (temp == SI7005_INVALID_VALUE) return SI7005_INVALID_VALUE;
  if (!si7005_start_conversion(i2c, false /* temp */, false /* fast */)) {
    return SI7005_INVALID_VALUE;
  }
  while (!si7005_is_data_ready(i2c)) {
  }
  float rh_val = si7005_read_data(i2c, false /* temp */);
  if (rh_val == SI7005_INVALID_VALUE) return SI7005_INVALID_VALUE;
  return si7005_rh_tcomp(rh_val, temp);
}

static bool si7005_modify_config(struct mgos_i2c *i2c, uint8_t mask,
                                 uint8_t set) {
  int ret = mgos_i2c_read_reg_b(i2c, SI7005_ADDR, SI7005_REG_CONFIG);
  if (ret < 0) return false;
  uint8_t val = (uint8_t) ret;
  val = (val & (~mask)) | set;
  return mgos_i2c_write_reg_b(i2c, SI7005_ADDR, SI7005_REG_CONFIG, val);
}

bool si7005_set_heater_bus(struct mgos_i2c *i2c, bool on) {
  if (!si7005_probe_bus(i2c)) return false;
  uint8_t set = (on ? SI7005_REG_CONFIG_HEAT : 0);
  return si7005_modify_config(i2c, SI7005_REG_CONFIG_HEAT, set);
}

bool si7005_start_conversion(struct mgos_i2c *i2c, bool temp, bool fast) {
  uint8_t mask = (SI7005_REG_CONFIG_TEMP | SI7005_REG_CONFIG_FAST |
                  SI7005_REG_CONFIG_START);
  uint8_t set = ((temp ? SI7005_REG_CONFIG_TEMP : 0) |
                 (fast ? SI7005_REG_CONFIG_FAST : 0) | SI7005_REG_CONFIG_START);
  return si7005_modify_config(i2c, mask, set);
}

bool si7005_is_data_ready(struct mgos_i2c *i2c) {
  int ret = mgos_i2c_read_reg_b(i2c, SI7005_ADDR, SI7005_REG_STATUS);
  return (ret & SI7005_REG_STATUS_nRDY) == 0;
}

float si7005_read_data(struct mgos_i2c *i2c, bool temp) {
  int ret = mgos_i2c_read_reg_w(i2c, SI7005_ADDR, SI7005_REG_DATA);
  if (ret < 0) return SI7005_INVALID_VALUE;
  float val = 0;
  if (temp) {
    val = ((ret >> 2) / 32.0f) - 50;
  } else {
    val = ((ret >> 4) / 16.0f) - 24;
    /* Linearizatiom, as described in 4.3 */
    val -= -0.00393f * (val * val) + 0.4008f * val - 4.7844f;
  }
  return val;
}

float si7005_rh_tcomp(float rh_val, float temp) {
  float rh_tcomp = rh_val + (temp - 30.0f) * (0.00237f * rh_val + 0.1973);
  return rh_tcomp;
}

bool mgos_si7005_init(void) {
  return true;
}
