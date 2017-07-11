/*
 * Copyright (C) 2012 Yule Fox. All rights reserved.
 * http://www.yulefox.com/
 */

#include <elf/script/script.h>
#include <elf/log.h>
#if defined (ELF_PLATFORM_WIN32)
#   include <elf/dumper.h>
#endif /* ELF_PLATFORM_WIN32 */

#define MAX_MODULE_LAYERS           8
#define MAX_MODULE_NAME_LENGTH      64
#define MAX_FUNCTION_NAME_LENGTH    64

namespace elf {
static lua_State *L;

// static const char *script_errstr(int err);
static int script_error(lua_State *L);
static int script_traceback(lua_State *L);
// static int script_stackdump(lua_State *L);

int script_init(void)
{
    MODULE_IMPORT_SWITCH;
    L = luaL_newstate();
    luaL_openlibs(L);
    return 0;
}

/**
 * Release script module
 */
int script_fini(void)
{
    MODULE_IMPORT_SWITCH;
    lua_close(L);
    return 0;
}

void script_foo(void)
{
}

lua_State *script_get_state(void)
{
    return L;
}

int script_str_exec(const char *cmd)
{
    int err = 0, handler = 0;

    if ((err = luaL_loadstring(L, cmd)) != 0) {
        return script_error(L);
    }
    handler = lua_gettop(L);
    lua_pushcfunction(L, script_traceback);
    lua_insert(L, handler);
    err = lua_pcall(L, 0, LUA_MULTRET, handler);
    lua_remove(L, handler);
    if (err) {
        return script_error(L);
    }
    if (strstr(cmd, "res_code = ") != NULL) {
        lua_getglobal(L, "res_code");
        if (lua_isnumber(L, -1)) {
            return lua_tointeger(L, -1);
        }
    }
    return SCRIPT_RC_OK;
}

int script_file_exec(const char *file)
{
    int err = 0, handler;

    if ((err = luaL_loadfile(L, file)) != 0) {
        return script_error(L);
    }
    handler = lua_gettop(L);
    lua_pushcfunction(L, script_traceback);
    lua_insert(L, handler);
    err = lua_pcall(L, 0, LUA_MULTRET, handler);
    lua_remove(L, handler);
    if (err) {
        return script_error(L);
    }
    return SCRIPT_RC_OK;
}

int script_func_exec(const char *func, const char *sig, ...)
{
    int lev = 1; /* `func` is global function */
    int err = 0;
    const char *cur_mod = func,  *mod;
    char module[MAX_MODULE_NAME_LENGTH];

    lua_getglobal(L, "_G");
    if (!lua_istable(L, -1)) {
        lua_pop(L, lev);
        LOG_ERROR("script", "%s", "Global table is NOT available.");
        return SCRIPT_RC_ERROR;
    }
    while ((mod = strchr(cur_mod, '.'))) {
        size_t tok_len = mod - cur_mod;

        strncpy(module, cur_mod, tok_len);
        module[tok_len] = '\0';
        cur_mod = mod + 1;
        lua_getfield(L, -1, module);
        ++lev;
        if (!lua_istable(L, -1)) {
            lua_pop(L, lev);
            LOG_ERROR("script", "`%s()` is NOT recognized as a Lua function.",
                func);
            return SCRIPT_RC_ERROR;
        }
    }
    lua_getfield(L, -1, cur_mod); /* function */
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, lev + 1);
        LOG_ERROR("script", "`%s()` is NOT recognized as a Lua function.",
            func);
        return SCRIPT_RC_ERROR;
    }

    va_list args;
    va_start(args, sig);
    int argc = 0;

    /* parse input arguments */
    for (; sig && *sig && *sig != '>'; ++sig, ++argc) {
        switch (*sig) {
        case 'i': /* integer */
            lua_pushnumber(L, va_arg(args, int));
            break;
        case 'l': /* long integer */
            tolua_pushusertype(L, va_arg(args, void *), "elf::oid_t");
            break;
        case 's': /* string */
            lua_pushstring(L, va_arg(args, char *));
            break;
        case 'f': /* double */
            lua_pushnumber(L, va_arg(args, double));
            break;
        default: /* double */
            LOG_ERROR("script", "invalid option (%c)", *sig);
            lua_pop(L, lev + argc);
            va_end(args);
            return SCRIPT_RC_ERROR;
        }
    }

    int handler = lua_gettop(L) - argc;

    lua_pushcfunction(L, script_traceback);
    lua_insert(L, handler);
    err = lua_pcall(L, argc, LUA_MULTRET, handler);
    lua_remove(L, handler);
    va_end(args);
    if (err) {
        return script_error(L);
    }
    lua_pop(L, lev);
    return SCRIPT_RC_OK;
}

/*
static const char *script_errstr(int err)
{
    switch (err) {
    case LUA_YIELD:
        return "[LUA_YIELD]";
    case LUA_ERRRUN:
        return "[LUA_ERRRUN]";
    case LUA_ERRSYNTAX:
        return "[LUA_ERRSYNTAX]";
    case LUA_ERRMEM:
        return "[LUA_ERRMEM]";
        // case LUA_ERRGCMM:  // Lua 5.2
        //    return "[LUA_ERRGCMM]";
    case LUA_ERRERR:
        return "[LUA_ERRERR]";
    case LUA_ERRFILE:
        return "[LUA_ERRFILE]";
    default:
        return "[UNKNOWN]";
    }
}
*/

static int script_error(lua_State *L)
{
    LOG_ERROR("script", "%s", lua_tostring(L, -1));
    lua_pop(L, 1);
    lua_gc(L, LUA_GCCOLLECT, 0);
    return SCRIPT_RC_ERROR;
}

static int script_traceback(lua_State *L)
{
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    lua_pushvalue(L, 1);  /* pass error message */
    lua_pushinteger(L, 2);  /* skip this function and traceback */
    lua_call(L, 2, 1);  /* call debug.traceback */
    return 1;
}

/*
static int script_stackdump(lua_State *L)
{
    int depth = lua_gettop(L);
    char line[ELF_MAX_LINE];
    char output[ELF_MAX_STACK * ELF_MAX_LINE] = "Lua stack:\n";

    snprintf(line, ELF_MAX_LINE, "depth: %d\n", depth);
    strcat(output, line);
    for (int i = 1; i <= depth; ++i) {
        int t = lua_type(L, i);
        switch (t) {
        case LUA_TBOOLEAN:
            snprintf(line, ELF_MAX_LINE, "\t%d %s\n", i,
                lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            snprintf(line, ELF_MAX_LINE,
                "\t%d %g\n", i, lua_tonumber(L, i));
            break;
        case LUA_TSTRING:
            snprintf(line, ELF_MAX_LINE,
                "\t%d '%s'\n", i, lua_tostring(L, i));
            break;
        default:
            snprintf(line, ELF_MAX_LINE,
                "\t%d %s\n", i, lua_typename(L, t));
            break;
        }
        strcat(output, line);
    }
    LOG_WARN("script", output);
    return depth;
}
*/
} // namespace elf

