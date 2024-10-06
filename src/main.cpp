#include <wups.h>

#include "fs/CFile.hpp"
#include "utils/logger.h"
#include "globals.h"
#include "plugin.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <vector>

#include <coreinit/title.h>
#include <coreinit/debug.h>
#include <coreinit/internal.h>

#include <sysapp/launch.h>

WUPS_PLUGIN_NAME(PLUGIN_NAME);
WUPS_PLUGIN_DESCRIPTION(PLUGIN_DESCRIPTION);
WUPS_PLUGIN_VERSION(PLUGIN_VERSION);
WUPS_PLUGIN_AUTHOR(PLUGIN_AUTHOR);
WUPS_PLUGIN_LICENSE(PLUGIN_LICENSE);

WUPS_USE_WUT_DEVOPTAB();

void trimWhitespace(uint8_t *buffer, uint64_t size) {
    uint64_t _size = size;
    while (_size > 0 && (buffer[_size - 1] == '\n' || buffer[_size - 1] == '\r' || buffer[_size - 1] == ' ')) {
        buffer[_size - 1] = 0;
        _size--;
    }
}

INITIALIZE_PLUGIN() {
    initLogging();
}

DEINITIALIZE_PLUGIN() {
    deinitLogging();
}

ON_APPLICATION_START() {
    initLogging();

    uint64_t titleId = OSGetTitleID();
    DEBUG_FUNCTION_LINE("Current Title ID is %016llX", titleId);

    if (!gWiiUMenuStarted && (titleId == 0x0005001010040000 || titleId == 0x0005001010040100 || titleId == 0x0005001010040200)) {
        gWiiUMenuStarted = true;

        DEBUG_FUNCTION_LINE("Wii U Menu Started");

        CFile file("fs:/vol/external01/wiiu/AutoStart/AutoStart.txt", CFile::eOpenTypes::ReadOnly);
        if (!file.isOpen()) {
            OSFatal(PLUGIN_NAME ": Failed to open AutoStart.txt");
        }

        uint64_t size = file.size();
        uint8_t *buffer = new uint8_t[size + 1];
        memset(buffer, 0, size + 1);
        if (file.read(buffer, size) < 0) {
            OSFatal(PLUGIN_NAME ": Failed to read AutoStart.txt");
        }

        file.close();

        trimWhitespace(buffer, size);

        if (sscanf((char*)buffer, "%llx", &gAutoStartTitleId) != 1) {
            char _buffer[0xFF];
            __os_snprintf(_buffer, 0xFF, PLUGIN_NAME ": Failed to parse AutoStart.txt as a uint64\n\nContent:\n\n%s", buffer);
            OSFatal(_buffer);
        }

        delete[] buffer;

        DEBUG_FUNCTION_LINE("AutoStart Title ID is %016llX", gAutoStartTitleId);
        DEBUG_FUNCTION_LINE("Launching AutoStart Title");

        SYSLaunchTitle(gAutoStartTitleId);
    }
}

ON_APPLICATION_ENDS() {
    deinitLogging();
}