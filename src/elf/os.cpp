/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/os.h>
#if defined(ELF_PLATFORM_LINUX)
#  include <sys/stat.h>
#  include <sys/types.h>
#  include <stdlib.h>
#  include <unistd.h>
#endif

namespace elf {
int os_mkdir(const char *dir)
{
#if defined(ELF_PLATFORM_WIN32)
    ::CreateDirectory(dir, NULL);
#elif defined(ELF_PLATFORM_LINUX)
    return mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

int os_putenv(char *val)
{
    return putenv(val);
}

const char *os_getenv(const char *name)
{
    return getenv(name);
}

int os_getenv_int(const char *name)
{
    const char *val = os_getenv(name);

    if (val) {
        return atoi(val);
    } else {
        return 0;
    }
}

int get_cpus()
{    
#if defined(ELF_PLATFORM_LINUX)
    return sysconf(_SC_NPROCESSORS_ONLN);
#else
    return 4;
#endif
}

} // namespace elf

