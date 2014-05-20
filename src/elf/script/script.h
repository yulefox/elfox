/**
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 *
 * @file script/script.h
 * @date 2012-08-14
 * @author Fox (yulefox at gmail.com)
 * Script.
 */

#if defined(ELF_HAVE_PRAGMA_ONCE)
#   pragma once
#endif

#ifndef ELF_SCRIPT_SCRIPT_H
#define ELF_SCRIPT_SCRIPT_H

#include <elf/config.h>
#include <lua.hpp>
#include <tolua++.h>

#if defined(ELF_PLATFORM_WIN32)
#    pragma comment(lib, "lua51.lib")
#endif

#define MAX_SCRIPT_TOKEN_LENGTH     256

namespace elf {

enum script_rc {
    SCRIPT_RC_OK,
    SCRIPT_RC_ERROR,
};

int script_init(void);

/**
 * Release script module
 */
int script_fini(void);

void script_foo(void);

lua_State *script_get_state(void);

int script_str_exec(const char *cmd);

int script_file_exec(const char *file);

int script_func_exec(const char *func, const char *sig, ...);
} // namespace elf

#endif /* !ELF_SCRIPT_SCRIPT_H */
