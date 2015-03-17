/*
// 这个函数把一个 C 对象指针置入对应的 userdata ，如果是第一次 push 则创建出新的 userdata ，否则复用曾经创建过的。
int script_signin(lua_State *L, void *object)
{
    void **ud = NULL;

    if (luaL_newmetatable(L, "script")) {
        // 在注册表中创建一个表存放所有的 object 指针到 userdata 的关系。
        // 这个表应该是一个 weak table ，当 Lua 中不再存在对 C 对象的引用会删除对应的记录。
        lua_newtable(L);
        lua_pushliteral(L, "kv");
        lua_setfield(L, -2, "__mode");
        lua_setmetatable(L, -2);
    }
    lua_rawgetp(L, -1, object);
    if (lua_type(L, -1) == LUA_TUSERDATA) {
        ud = (void **)lua_touserdata(L, -1);
        if (*ud == object) {
            lua_replace(L, -2);
            return 0;
        }
        // C 对象指针被释放后，有可能地址被重用。
        // 这个时候，可能取到曾经保存起来的 userdata ，里面的指针必然为空。
        assert(*ud == NULL);
    }
    ud = (void **)lua_newuserdata(L, sizeof(void*));
    *ud = object;
    lua_pushvalue(L, -1);
    lua_rawsetp(L, -4, object);
    lua_replace(L, -3);
    lua_pop(L, 1);
    return 1;
}

// 这个函数会解除 C 对象在 Lua 中的引用，后续在 Lua 中对这个对象的访问，都将得到 NULL 指针。
void script_signout(lua_State *L, void *object)
{
    luaL_getmetatable(L, "script");
    if (lua_istable(L, -1)) {
        lua_rawgetp(L, -1, object);
        if (lua_type(L, -1) == LUA_TUSERDATA) {
            void **ud = (void **)lua_touserdata(L, -1);
            // 这个 assert 防止 deleteobject 被重复调用。
            assert(*ud == object);
            // 销毁一个被 Lua 引用住的对象，只需要把 *ud 置为 NULL 。
            *ud = NULL;
        }
        lua_pop(L, 2);
    } else {
        // 有可能从未调用过 pushobject ，此时注册表中 script 项尚未建立。
        lua_pop(L, 1);
    }
}

// 这个函数把 index 处的 userdata 转换为一个 C 对象。如果对象已经销毁，则返回 NULL 指针。 在给这个对象绑定 C 方法时，应注意在 toobject 调用后，全部对指针做检查，空指针应该被正确处理。
void *script_check(lua_State *L, int index)
{
    void **ud = (void **)lua_touserdata(L, index);
    if (ud == NULL)
        return NULL;
    // 如果 object 已在 C 代码中销毁，*ud 为 NULL 。
    return *ud;
}
*/

