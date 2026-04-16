#pragma once

#ifndef WEBUI_STORAGE_H
#define WEBUI_STORAGE_H

#include <string>

bool loadWebUiStorageFromConfig();
bool initWebUiStorage();
void setUseInternalWebUiStorage(bool enabled);
bool useInternalWebUiStorage();
std::string getWebUiFilePath(const std::string &uriPath);

#endif //WEBUI_STORAGE_H
