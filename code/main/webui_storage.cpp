#include "../../include/webui_storage.h"

#include <vector>

#include "ClassLogFile.h"
#include "configFile.h"
#include "Helper.h"
#include "../../include/defines.h"

static const char *TAG = "WEBUI STORAGE";
static bool internalWebUiStorage = false;

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

            if ((value == "INTERNAL") || (value == "INTERNALFLASH") || (value == "FLASH") || (value == "TRUE")) {
                return true;
            }

            if ((value == "SDCARD") || (value == "SD") || (value == "FALSE") || (value == "")) {
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

bool useInternalWebUiStorage()
{
    return internalWebUiStorage;
}

std::string getWebUiFilePath(const std::string &uriPath)
{
    if (useInternalWebUiStorage()) {
        return "/html" + uriPath;
    }

    return "/sdcard/html" + uriPath;
}
