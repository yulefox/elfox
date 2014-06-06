/*
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/log.h>
#include <elf/dir.h>

static const char *CONFIG_FILE_NAME = "CONFIG/log.conf";

#define LOG(fmt, args...) printf(fmt, ##args)
#define LOGE(fmt, args...) LOG(fmt, ##args)

namespace elf {
int log_init(void)
{
    MODULE_IMPORT_SWITCH;
    dir_make("log");
#if defined(ELF_USE_LOG4CPLUS)
    try {
        log4cplus::ConfigureAndWatchThread th(CONFIG_FILE_NAME);
    } catch(...) {
        LOGE("Exception occured...");
    }
#endif /* ELF_USE_LOG4CPLUS */
    return 0;
}

int log_fini(void)
{
    MODULE_IMPORT_SWITCH;
#if defined(ELF_USE_LOG4CPLUS)
    log4cplus::Logger::shutdown();
#endif /* ELF_USE_LOG4CPLUS */
    return 0;
}
} // namespace elf

