# Si7005 Sensor Driver for Mongoose OS

## Overview

[Si7005](https://eu.mouser.com/new/Silicon-Laboratories/silabs-si7005/) is a temperature and relative humidity sensor by Silicon Labs.

This library provides a driver for this device.

## API documentation

See [si7005.h](include/si7005.h).

## Example


```c
#include "mgos.h"

#include "si7005.h"

static void temp_timer_cb(void *arg) {
  float temp = si7005_read_temp();
  float rh = si7005_read_rh();
  LOG(LL_INFO, ("T %.2f RH %.2f", temp, rh));
  (void) arg;
}

enum mgos_app_init_result mgos_app_init(void) {
  if (si7005_probe()) {
    LOG(LL_INFO, ("Si7005 sensor found"));
    mgos_set_timer(1000, MGOS_TIMER_REPEAT, temp_timer_cb, NULL);
  } else {
    LOG(LL_WARN, ("Failed to init temp sensor"));
  }
  return MGOS_APP_INIT_SUCCESS;
}
```

_Note:_ You need to make sure that I2C is enabled. This can be achieved by adding
```yaml
config_schema:
  - ["i2c.enable", true]
```
to mos.yml. You may need to adjust SDA and SCL pins as well.
See [I2C](https://github.com/mongoose-os-libs/i2c) library for details.
