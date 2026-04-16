#include "../../include/webui_storage.h"

#include <vector>

#include "esp_err.h"
#include "esp_littlefs.h"
#include "ClassLogFile.h"
#include "configFile.h"
#include "Helper.h"
#include "../../include/defines.h"

static const char *TAG = "WEBUI STORAGE";
static bool internalWebUiStorage = false;
static const char *WEBUI_LITTLEFS_PARTITION_LABEL = "webui";
static const char *WEBUI_LITTLEFS_BASE_PATH = "/html";

bool loadWebUiStorageFromConfig()
{
    ConfigFile configFile = ConfigFile(CONFIG_FILE);

    if (!configFile.ConfigFileExists()) {
        return false;
    }

    std::vector<std::string> splitted;
    std::string line = "";
    bool disabledLine = false;
    bool eof = false;

    while ((!configFile.GetNextParagraph(line, disabledLine, eof) ||
            (line.compare("[System]") != 0)) && !eof) {}

    if (eof || disabledLine) {
        return false;
    }

    while (configFile.getNextLine(&line, disabledLine, eof) &&
            !configFile.isNewParagraph(line)) {
        splitted = ZerlegeZeile(line, "=");

        if ((splitted.size() >= 1) && (toUpper(splitted[0]) == "HTMLSTORAGE")) {
            std::string value = "";
            if (splitted.size() > 1) {
                value = toUpper(trim(splitted[1]));
            }

            if (value == "INTERNAL") {
                return true;
            }

            if (value == "SDCARD") {
                return false;
            }

            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Unknown value for [System]/HtmlStorage: " + value + ". Using default: SDCard");
            return false;
        }
    }

    return false;
}

void setUseInternalWebUiStorage(bool enabled)
{
    internalWebUiStorage = enabled;
}

static bool mountInternalWebUiStorage()
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = WEBUI_LITTLEFS_BASE_PATH,
        .partition_label = WEBUI_LITTLEFS_PARTITION_LABEL,
        .partition = NULL,
        .format_if_mount_failed = false,
        .read_only = true,
        .dont_mount = false
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_ERROR, TAG, std::string("Failed to mount LittleFS partition 'webui' (") + esp_err_to_name(ret) + ")");
        return false;
    }

    size_t total = 0, used = 0;
    ret = esp_littlefs_info(WEBUI_LITTLEFS_PARTITION_LABEL, &total, &used);
    if (ret == ESP_OK) {
        LogFile.WriteToFile(ESP_LOG_INFO, TAG, "Mounted LittleFS partition 'webui': " + std::to_string(used) + "/" + std::to_string(total) + " bytes used");
    }
    else {
        LogFile.WriteToFile(ESP_LOG_WARN, TAG, std::string("Mounted LittleFS partition 'webui', but could not read partition info (") + esp_err_to_name(ret) + ")");
    }

    return true;
}

bool initWebUiStorage()
{
    bool useInternalStorage = loadWebUiStorageFromConfig();

    if (useInternalStorage) {
        if (!mountInternalWebUiStorage()) {
            LogFile.WriteToFile(ESP_LOG_WARN, TAG, "Falling back to SDCard Web UI storage");
            setUseInternalWebUiStorage(false);
            return false;
        }

        setUseInternalWebUiStorage(true);
        return true;
    }

    setUseInternalWebUiStorage(false);
    return true;
}

bool useInternalWebUiStorage()
{
    return internalWebUiStorage;
}

std::string getWebUiFilePath(const std::string &uriPath)
{
    if (useInternalWebUiStorage()) {
        return std::string(WEBUI_LITTLEFS_BASE_PATH) + uriPath;
    }

    return "/sdcard/html" + uriPath;
}
