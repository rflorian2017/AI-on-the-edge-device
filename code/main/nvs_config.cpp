#include "nvs_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <nvs.h>
#include <nvs_flash.h>
#include "esp_log.h"

static const char *TAG        = "NVS_CFG";
static const char *NVS_NS     = "ai_edge";          /* NVS namespace (≤15 chars) */
static const char *NVS_KEY    = "cfg_ini";           /* NVS key       (≤15 chars) */

/* Maximum config file size we are willing to cache in NVS.
 * The NVS partition is 16 KB; leaving headroom for NVS overhead. */
#define MAX_CONFIG_SIZE (10 * 1024)

/* ------------------------------------------------------------------ */

bool NvsConfigSave(const char *config_file_path)
{
    FILE *pFile = fopen(config_file_path, "r");
    if (!pFile) {
        ESP_LOGW(TAG, "Config file not found: %s", config_file_path);
        return false;
    }

    /* Determine file size */
    fseek(pFile, 0, SEEK_END);
    long fileSize = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    if (fileSize <= 0 || fileSize > MAX_CONFIG_SIZE) {
        ESP_LOGE(TAG, "Config file size out of range: %ld bytes", fileSize);
        fclose(pFile);
        return false;
    }

    char *buf = (char *)malloc((size_t)fileSize);
    if (!buf) {
        ESP_LOGE(TAG, "Allocation failed");
        fclose(pFile);
        return false;
    }

    size_t bytesRead = fread(buf, 1, (size_t)fileSize, pFile);
    fclose(pFile);

    if ((long)bytesRead != fileSize) {
        ESP_LOGE(TAG, "Read error: expected %ld bytes, got %zu", fileSize, bytesRead);
        free(buf);
        return false;
    }

    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NS, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: 0x%x", (unsigned)err);
        free(buf);
        return false;
    }

    err = nvs_set_blob(handle, NVS_KEY, buf, (size_t)fileSize);
    free(buf);

    if (err == ESP_OK) {
        err = nvs_commit(handle);
    }
    nvs_close(handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_set_blob / nvs_commit failed: 0x%x", (unsigned)err);
        return false;
    }

    ESP_LOGI(TAG, "Config backed up to NVS (%ld bytes)", fileSize);
    return true;
}

/* ------------------------------------------------------------------ */

bool NvsConfigLoad(const char *config_file_path)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NS, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "nvs_open failed: 0x%x (no backup available)", (unsigned)err);
        return false;
    }

    /* Query required blob size */
    size_t required = 0;
    err = nvs_get_blob(handle, NVS_KEY, NULL, &required);
    if (err != ESP_OK || required == 0) {
        ESP_LOGW(TAG, "No config backup in NVS (err 0x%x, size %zu)", (unsigned)err, required);
        nvs_close(handle);
        return false;
    }

    char *buf = (char *)malloc(required);
    if (!buf) {
        ESP_LOGE(TAG, "Allocation failed");
        nvs_close(handle);
        return false;
    }

    err = nvs_get_blob(handle, NVS_KEY, buf, &required);
    nvs_close(handle);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_get_blob failed: 0x%x", (unsigned)err);
        free(buf);
        return false;
    }

    FILE *pFile = fopen(config_file_path, "w");
    if (!pFile) {
        ESP_LOGE(TAG, "Cannot open %s for writing", config_file_path);
        free(buf);
        return false;
    }

    size_t written = fwrite(buf, 1, required, pFile);
    fclose(pFile);
    free(buf);

    if (written != required) {
        ESP_LOGE(TAG, "Write error: expected %zu bytes, wrote %zu", required, written);
        return false;
    }

    ESP_LOGI(TAG, "Config restored from NVS (%zu bytes)", required);
    return true;
}

/* ------------------------------------------------------------------ */

bool NvsConfigHasBackup(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NS, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        return false;
    }

    size_t required = 0;
    err = nvs_get_blob(handle, NVS_KEY, NULL, &required);
    nvs_close(handle);

    return (err == ESP_OK && required > 0);
}
