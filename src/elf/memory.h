/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

/**
 * @file memory.h
 * @author Fox (yulefox at gmail.com)
 * @date 2012-02-29
 * Based on Gamebryo EFD.
 */

#ifndef ELF_MEMORY_H
#define ELF_MEMORY_H

#include <elf/config.h>

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#define S_FREE(x)  do { if (x) { free(x); (x)=NULL; } } while (0)
#define S_DELETE(x)  do { if (x) { delete(x); (x)=NULL; } } while (0)

#define E_ALLOC malloc
#define E_REALLOC realloc
#define E_NEW new
#define E_FREE S_FREE
#define E_DELETE delete

#endif /* !ELF_MEMORY_H */
