/*
 * Copyright (C) 2011-2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/log.h>
#include <elf/os.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

static const char *CONFIG_FILE_NAME = "CONFIG/log.conf";
static const char *LOG_PATH = "log/events";
static int LOG_FILE_SIZE = 1024 * 1024 * 100; // 100M

#define LOG(fmt, args...) printf(fmt, ##args)
#define LOGE(fmt, args...) LOG(fmt, ##args)

namespace elf {
int log_init(void)
{
    MODULE_IMPORT_SWITCH;
    os_mkdir("log");
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

//////////////////////////////////////////////////////////////////////////////

typedef struct log_entry_s {
    size_t offset;
    size_t size;
    int fd;
    char *ident;
} log_entry_t;

typedef std::map<const char*, log_entry_t*> LOG_ENTRY_MAP;

static LOG_ENTRY_MAP s_log_entry;

static log_entry_t *log_entry_create(const char *ident)
{
    log_entry_t *entry;
    char fullname[256];
    int fd;

    os_mkdir(LOG_PATH);

    struct tm *tm = NULL;
    time_t now;

    now = time(NULL);
    tm = localtime(&now);
    if (tm == NULL) {
        return NULL;
    }

    int year = tm->tm_year + 1900;
    int mon = tm->tm_mon + 1;
    int day = tm->tm_mday;

    sprintf(fullname, "%s/%s-%d%02d%02d.log", LOG_PATH, ident, year, mon, day);

    fd = open(fullname, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH);

    entry = (log_entry_t*)malloc(sizeof(log_entry_t));
    if (entry == NULL) {
        close(fd);
        return NULL;
    }
    
    entry->fd = fd;
    entry->offset = lseek(fd, 0, SEEK_END);
    entry->size = LOG_FILE_SIZE;
    entry->ident = (char*)malloc(strlen(ident) + 1);
    strcpy(entry->ident, ident);

    s_log_entry.insert(std::make_pair(ident, entry));
    return entry;
}

static log_entry_t *log_entry_find(const char *ident)
{
    LOG_ENTRY_MAP::iterator itr = s_log_entry.find(ident);
    if (itr == s_log_entry.end()) {
        return NULL;
    }
    return itr->second;
}

static int log_entry_close(log_entry_t *entry)
{
    char oldpath[256];
    char newpath[256];
    struct tm *tm = NULL;
    time_t now;

    now = time(NULL);
    tm = localtime(&now);
    if (tm == NULL) {
        return -1;
    }

    close(entry->fd);

    int year = tm->tm_year + 1900;
    int mon = tm->tm_mon + 1;
    int day = tm->tm_mday;

    int idx = now % 86400;
    sprintf(newpath, "%s/%s-%d%02d%02d-%d.log",
            LOG_PATH, entry->ident, year, mon, day, idx);
    sprintf(oldpath, "%s/%s-%d%02d%02d.log", LOG_PATH, entry->ident,
            year, mon, day);

    rename(oldpath, newpath);
    s_log_entry.erase(entry->ident);
    free(entry->ident);
    free(entry);
    return 0;
}

static void log_entry_append(log_entry_t *entry, const char *buf)
{
    if (entry->offset + strlen(buf) > entry->size) {
        char ident[128];
        strcpy(ident, entry->ident);
        log_entry_close(entry);
        log_append(ident, buf);
    } else {
        write(entry->fd, (const void*)buf, strlen(buf));
        fsync(entry->fd);
        entry->offset += strlen(buf);
    }
}

void log_append(const char *ident, const char *buf)
{
    log_entry_t *entry = log_entry_find(ident);
    if (entry == NULL) {
        entry = log_entry_create(ident);
    }
    log_entry_append(entry, buf);
}

} // namespace elf

