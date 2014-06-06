/*
 * Copyright (C) 2014 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/dir.h>
#if defined(ELF_PLATFORM_LINUX)
#  include <sys/stat.h>
#  include <sys/types.h>
#endif

namespace elf {
int dir_make(const char *dir)
{
#if defined(ELF_PLATFORM_WIN32)
    ::CreateDirectory(dir, NULL);
#elif defined(ELF_PLATFORM_LINUX)
    return mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}
} // namespace elf

