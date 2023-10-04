/*
 * This file is a part of: https://github.com/brilliantlabsAR/frame-codebase
 *
 * Authored by: Raj Nakarja / Brilliant Labs Ltd. (raj@brilliant.xyz)
 *              Rohit Rathnam / Silicon Witchery AB (rohit@siliconwitchery.com)
 *              Uma S. Gupta / Techno Exponent (umasankar@technoexponent.com)
 *
 * ISC Licence
 *
 * Copyright © 2023 Brilliant Labs Ltd.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>
#include "error_helpers.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "nrfx_log.h"

// extern uint32_t __heap_start;
// extern uint32_t __heap_end;

void run_lua(void)
{
    const char *LUA_FILE = "\
    function test_me()\n\
        k = 1\n\
        s = 0\n\
        for i = 0, 100000 do\n\
            if math.fmod(i, 2) == 0 then\n\
                s = s + (4 / k)\n\
            else\n\
                s = s - (4 / k)\n\
            end\n\
            k = k + 2\n\
        end\n\
        return s\n\
    end\n\
    ";

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    if (L == NULL)
    {
        error_with_message("Cannot create lua state: not enough memory");
    }

    luaL_dostring(L, LUA_FILE);
    lua_getglobal(L, "test_me");
    if (lua_isfunction(L, -1))
    {

        lua_pcall(L, 0, 1, 0);
        if (lua_isnumber(L, -1))
        {
            lua_Number ret = lua_tonumber(L, -1);
            LUA_LOG("ret = %d", (int)ret);
        }
        else
        {
            LUA_LOG("ret = NaN");
        }
    }
    lua_close(L);
}