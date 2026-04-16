FILE(GLOB_RECURSE app_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.*)

file(GLOB WEBUI_EMBED_FILES "${CMAKE_CURRENT_SOURCE_DIR}/../../../sd-card/html/*")
list(FILTER WEBUI_EMBED_FILES EXCLUDE REGEX ".*/\\.gitignore$")

set(WEBUI_INDEX_CPP "${CMAKE_CURRENT_BINARY_DIR}/webui_embedded_index.cpp")
file(WRITE "${WEBUI_INDEX_CPP}" "#include \"webui_embedded.h\"\n\n")

set(WEBUI_INDEX 0)
foreach(WEBUI_FILE ${WEBUI_EMBED_FILES})
    get_filename_component(WEBUI_FILENAME "${WEBUI_FILE}" NAME)
    string(REGEX REPLACE "[^A-Za-z0-9]" "_" WEBUI_SYMBOL_PATH "${WEBUI_FILE}")

    file(APPEND "${WEBUI_INDEX_CPP}" "extern const unsigned char webui_data_${WEBUI_INDEX}_start[] asm(\"_binary_${WEBUI_SYMBOL_PATH}_start\");\n")
    file(APPEND "${WEBUI_INDEX_CPP}" "extern const unsigned char webui_data_${WEBUI_INDEX}_end[] asm(\"_binary_${WEBUI_SYMBOL_PATH}_end\");\n")

    math(EXPR WEBUI_INDEX "${WEBUI_INDEX} + 1")
endforeach()

file(APPEND "${WEBUI_INDEX_CPP}" "\nstatic const EmbeddedWebUiFile WEBUI_FILES[] = {\n")

set(WEBUI_INDEX 0)
foreach(WEBUI_FILE ${WEBUI_EMBED_FILES})
    get_filename_component(WEBUI_FILENAME "${WEBUI_FILE}" NAME)
    file(APPEND "${WEBUI_INDEX_CPP}" "    {\"/html/${WEBUI_FILENAME}\", webui_data_${WEBUI_INDEX}_start, webui_data_${WEBUI_INDEX}_end},\n")

    math(EXPR WEBUI_INDEX "${WEBUI_INDEX} + 1")
endforeach()

file(APPEND "${WEBUI_INDEX_CPP}" "};\n\n")
file(APPEND "${WEBUI_INDEX_CPP}" "const EmbeddedWebUiFile* findEmbeddedWebUiFile(const std::string &uriPath)\n")
file(APPEND "${WEBUI_INDEX_CPP}" "{\n")
file(APPEND "${WEBUI_INDEX_CPP}" "    for (const auto &file : WEBUI_FILES) {\n")
file(APPEND "${WEBUI_INDEX_CPP}" "        if (uriPath == file.uriPath) {\n")
file(APPEND "${WEBUI_INDEX_CPP}" "            return &file;\n")
file(APPEND "${WEBUI_INDEX_CPP}" "        }\n")
file(APPEND "${WEBUI_INDEX_CPP}" "    }\n")
file(APPEND "${WEBUI_INDEX_CPP}" "    return nullptr;\n")
file(APPEND "${WEBUI_INDEX_CPP}" "}\n")

list(APPEND app_sources "${WEBUI_INDEX_CPP}")

idf_component_register(SRCS ${app_sources}
                    INCLUDE_DIRS "." "../../include" "miniz"
                    REQUIRES vfs esp_http_server app_update esp_http_client nvs_flash jomjol_tfliteclass jomjol_flowcontroll spiffs jomjol_helper jomjol_controlGPIO
                    EMBED_FILES ${WEBUI_EMBED_FILES})
