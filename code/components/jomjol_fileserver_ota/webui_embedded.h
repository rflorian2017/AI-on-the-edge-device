#pragma once

#ifndef WEBUI_EMBEDDED_H
#define WEBUI_EMBEDDED_H

#include <string>

struct EmbeddedWebUiFile {
    const char *uriPath;
    const unsigned char *start;
    const unsigned char *end;
};

const EmbeddedWebUiFile* findEmbeddedWebUiFile(const std::string &uriPath);

#endif //WEBUI_EMBEDDED_H
