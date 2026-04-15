#pragma once

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Save configuration file from SD card to NVS.
 * @param config_file_path Path to config file on SD card (e.g. "/sdcard/config/config.ini")
 * @return true on success, false on failure
 */
bool NvsConfigSave(const char *config_file_path);

/**
 * Load configuration file from NVS and write it to the SD card.
 * @param config_file_path Path to write the restored config file on the SD card
 * @return true on success, false on failure
 */
bool NvsConfigLoad(const char *config_file_path);

/**
 * Check whether a configuration backup exists in NVS.
 * @return true if a valid backup exists, false otherwise
 */
bool NvsConfigHasBackup(void);

#ifdef __cplusplus
}
#endif
