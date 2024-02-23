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

#include <stdbool.h>
#include <stdint.h>
#include "lua.h"
#include "lauxlib.h"
#include "spi.h"
#include "error_logging.h"
#include "i2c.h"
#include "nrf_gpio.h"
#include "pinout.h"

static int lua_camera_capture(lua_State *L)
{
    if (nrf_gpio_pin_out_read(CAMERA_SLEEP_PIN) == false)
    {
        luaL_error(L, "camera is asleep");
    }

    uint8_t address = 0x20;
    spi_write(FPGA, &address, 1, false);
    return 0;
}

static uint16_t get_bytes_available(void)
{
    uint8_t address = 0x21;
    uint8_t data[2] = {0, 0};

    spi_write(FPGA, &address, 1, true);
    spi_read(FPGA, (uint8_t *)data, sizeof(data), false);

    uint16_t bytes_available = (uint16_t)data[0] << 8 |
                               (uint16_t)data[1];

    return bytes_available;
}

static int lua_camera_read(lua_State *L)
{
    luaL_checkinteger(L, 1);

    lua_Integer bytes_requested = lua_tointeger(L, 1);
    uint16_t bytes_available = get_bytes_available();

    if (bytes_requested <= 0)
    {
        luaL_error(L, "bytes must be greater than 0");
    }

    if (bytes_available <= 0)
    {
        lua_pushnil(L);
        return 1;
    }

    uint8_t address = 0x22;

    uint16_t length = bytes_available < bytes_requested
                          ? bytes_available
                          : bytes_requested;

    uint8_t *data = malloc(length);
    if (data == NULL)
    {
        luaL_error(L, "not enough memory");
    }

    spi_write(FPGA, &address, 1, true);
    spi_read(FPGA, data, length, false);

    lua_pushlstring(L, (char *)data, length);
    free(data);

    return 1;
}

static int lua_camera_sleep(lua_State *L)
{
    nrf_gpio_pin_write(CAMERA_SLEEP_PIN, false);
    return 0;
}

static int lua_camera_wake(lua_State *L)
{
    nrf_gpio_pin_write(CAMERA_SLEEP_PIN, true);
    return 0;
}

static int lua_camera_get_brightness(lua_State *L)
{
    if (nrf_gpio_pin_out_read(CAMERA_SLEEP_PIN) == false)
    {
        luaL_error(L, "camera is asleep");
    }

    uint8_t address = 0x25;
    spi_write(FPGA, &address, 1, true);

    uint8_t data[3];
    spi_read(FPGA, data, sizeof(data), false);

    lua_newtable(L);

    lua_pushnumber(L, data[0]);
    lua_setfield(L, -2, "r");

    lua_pushnumber(L, data[1]);
    lua_setfield(L, -2, "g");

    lua_pushnumber(L, data[2]);
    lua_setfield(L, -2, "b");

    return 1;
}

static int lua_camera_set_exposure(lua_State *L)
{
    if (nrf_gpio_pin_out_read(CAMERA_SLEEP_PIN) == false)
    {
        luaL_error(L, "camera is asleep");
    }

    luaL_checkinteger(L, 1);

    lua_Integer exposure_time = lua_tointeger(L, 1);

    if (exposure_time < 20 || exposure_time > 25000)
    {
        return luaL_error(L, "exposure must be between 20us and 25000us");
    }

    check_error(i2c_write(CAMERA, 0x3500, 0x03, exposure_time >> 12).fail);
    check_error(i2c_write(CAMERA, 0x3501, 0xFF, exposure_time >> 4).fail);
    check_error(i2c_write(CAMERA, 0x3502, 0xF0, exposure_time << 4).fail);

    return 0;
}

static int lua_camera_set_gain(lua_State *L)
{
    if (nrf_gpio_pin_out_read(CAMERA_SLEEP_PIN) == false)
    {
        luaL_error(L, "camera is asleep");
    }

    luaL_checkinteger(L, 1);

    lua_Integer sensor_gain = lua_tointeger(L, 1);

    if (sensor_gain > 0x3FF)
    {
        return luaL_error(L, "gain must be less than 0x3FF");
    }

    check_error(i2c_write(CAMERA, 0x3504, 0x03, sensor_gain >> 8).fail);
    check_error(i2c_write(CAMERA, 0x3505, 0xFF, sensor_gain).fail);

    return 0;
}

static int lua_camera_set_white_balance(lua_State *L)
{
    if (nrf_gpio_pin_out_read(CAMERA_SLEEP_PIN) == false)
    {
        luaL_error(L, "camera is asleep");
    }

    luaL_checkinteger(L, 1);
    luaL_checkinteger(L, 2);
    luaL_checkinteger(L, 3);

    lua_Integer red_gain = lua_tointeger(L, 1);
    lua_Integer green_gain = lua_tointeger(L, 1);
    lua_Integer blue_gain = lua_tointeger(L, 1);

    if (red_gain > 0x3FF || green_gain > 0x3FF || blue_gain > 0x3FF)
    {
        return luaL_error(L, "gain values must be less than 0x3FF");
    }

    check_error(i2c_write(CAMERA, 0x5180, 0x0F, red_gain >> 8).fail);
    check_error(i2c_write(CAMERA, 0x5181, 0xFF, red_gain).fail);
    check_error(i2c_write(CAMERA, 0x5182, 0x0F, green_gain >> 8).fail);
    check_error(i2c_write(CAMERA, 0x5183, 0xFF, green_gain).fail);
    check_error(i2c_write(CAMERA, 0x5184, 0x0F, blue_gain >> 8).fail);
    check_error(i2c_write(CAMERA, 0x5185, 0xFF, blue_gain).fail);

    return 0;
}

static int lua_camera_set_register(lua_State *L)
{
    if (nrf_gpio_pin_out_read(CAMERA_SLEEP_PIN) == false)
    {
        luaL_error(L, "camera is asleep");
    }

    luaL_checkinteger(L, 1);
    luaL_checkinteger(L, 2);

    lua_Integer address = lua_tointeger(L, 1);
    lua_Integer value = lua_tointeger(L, 2);

    if (address < 0 || address > 0xFFFF)
    {
        luaL_error(L, "address must be a 16 bit unsigned number");
    }

    if (value < 0 || value > 0xFF)
    {
        luaL_error(L, "value must be an 8 bit unsigned number");
    }

    i2c_response_t response = i2c_write(CAMERA,
                                        (uint16_t)address,
                                        0xFF,
                                        (uint8_t)value);

    if (response.fail)
    {
        error();
    }

    return 0;
}

void lua_open_camera_library(lua_State *L)
{
    lua_getglobal(L, "frame");

    lua_newtable(L);

    lua_pushcfunction(L, lua_camera_capture);
    lua_setfield(L, -2, "capture");

    lua_pushcfunction(L, lua_camera_read);
    lua_setfield(L, -2, "read");

    lua_pushcfunction(L, lua_camera_sleep);
    lua_setfield(L, -2, "sleep");

    lua_pushcfunction(L, lua_camera_wake);
    lua_setfield(L, -2, "wake");

    lua_pushcfunction(L, lua_camera_get_brightness);
    lua_setfield(L, -2, "get_brightness");

    lua_pushcfunction(L, lua_camera_set_exposure);
    lua_setfield(L, -2, "set_exposure");

    lua_pushcfunction(L, lua_camera_set_gain);
    lua_setfield(L, -2, "set_gain");

    lua_pushcfunction(L, lua_camera_set_white_balance);
    lua_setfield(L, -2, "set_white_balance");

    lua_pushcfunction(L, lua_camera_set_register);
    lua_setfield(L, -2, "set_register");

    lua_setfield(L, -2, "camera");

    lua_pop(L, 1);
}