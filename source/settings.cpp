#include "settings.h"
#include "hal.h"

#include "hal_mfs.h"
#include "stdutil.h"


#define SECTOR_SIZE_BYTES 2048U
#define MFS_BANK_NB_SECTORS 2U
#define BANK_0_START_SECTOR 124U

const MFSConfig mfscfg1 = {
  .flashp           = (BaseFlash *)&EFLD1,
  .erased           = 0xFFFFFFFFU,
  .bank_size        = SECTOR_SIZE_BYTES,
  .bank0_start      = BANK_0_START_SECTOR,
  .bank0_sectors    = MFS_BANK_NB_SECTORS,
  .bank1_start      = BANK_0_START_SECTOR + MFS_BANK_NB_SECTORS,
  .bank1_sectors    = MFS_BANK_NB_SECTORS
};


mfs_nocache_buffer_t __nocache_mfsbuf1;
MFSDriver mfs1;

void settingsInit() {
  eflStart(&EFLD1, NULL);
  mfsObjectInit(&mfs1, &__nocache_mfsbuf1);
  mfsStart(&mfs1, &mfscfg1);
}

mfs_error_t store_settings(mfs_id_t id, rom_settings_t* settings) {
    size_t len = sizeof(rom_settings_t);
    return mfsWriteRecord(&mfs1, id, len, (uint8_t*)settings);
}

mfs_error_t read_settings(mfs_id_t id, rom_settings_t* settings) {
    size_t len = sizeof(rom_settings_t);
    return mfsReadRecord(&mfs1, id, &len, (uint8_t*)settings);
}

mfs_error_t storage_erase() {
    return mfsErase(&mfs1);
}

mfs_error_t storage_restart() {
    mfsStop(&mfs1);
    chThdSleepMilliseconds(10);
    return mfsStart(&mfs1, &mfscfg1);

}