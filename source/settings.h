#pragma once
#include <hal.h>
#include "hal_mfs.h"
#include <inttypes.h>

// Add in your Makefile:
// include $(CHIBIOS)/os/hal/lib/complex/mfs/hal_mfs.mk
// and enable EFL in halconf.h

typedef struct __attribute__((aligned(8)))
{
    uint8_t id;                     // ID of this node
    uint8_t baudrate;               // baudrate  est qu'il suffit ?? pas mettre uint32 ? 
    uint8_t return_delay;           // return delay for response packet
    uint8_t pump_duty;              // duty cycle for the pump ON state
    uint8_t valve_duty;             // duty cycle for the valve OPEN state
    uint16_t current_threshold;     // current threshold for grab detection
    uint16_t valve_release_time;    // OPEN state duration for the valve release
} rom_settings_t ;



void settingsInit();

mfs_error_t store_settings(mfs_id_t id, rom_settings_t* settings);
mfs_error_t read_settings(mfs_id_t id, rom_settings_t* settings);

mfs_error_t storage_erase();
mfs_error_t storage_restart();
