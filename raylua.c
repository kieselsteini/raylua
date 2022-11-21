/*
================================================================================

    rayLua - a simple Lua binding for raylib

    LICENSE: zlib/libpng
    Copyright (c) 2022 Sebastian Steinhauer <s.steinhauer@yahoo.de>

    This software is provided "as-is", without any express or implied warranty. In no event
    will the authors be held liable for any damages arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose, including commercial
    applications, and to alter it and redistribute it freely, subject to the following restrictions:

        1. The origin of this software must not be misrepresented; you must not claim that you
        wrote the original software. If you use this software in a product, an acknowledgment
        in the product documentation would be appreciated but is not required.

        2. Altered source versions must be plainly marked as such, and must not be misrepresented
        as being the original software.

        3. This notice may not be removed or altered from any source distribution.

*/
//==[[ Includes ]]==============================================================
#include <string.h>
#include <ctype.h>

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#include "raylib.h"
#include "raymath.h"


//==[[ Helper Functions ]]======================================================

enum { INDEX_TABLE, NEWINDEX_TABLE, METHOD_TABLE };

static void *push_object(lua_State *L, const char *name, const size_t size, const int uvalues) {
    void *object = lua_newuserdatauv(L, size, uvalues);
    luaL_setmetatable(L, name);
    return object;
}

static void push_meta(lua_State *L, const char *name, const luaL_Reg funcs[]) {
    luaL_newmetatable(L, name);
    lua_newtable(L); lua_pushvalue(L, -1); lua_rawseti(L, -3, INDEX_TABLE);
    lua_newtable(L); lua_pushvalue(L, -1); lua_rawseti(L, -4, NEWINDEX_TABLE);
    lua_newtable(L); lua_pushvalue(L, -1); lua_rawseti(L, -5, METHOD_TABLE);
    for (int i = 0; funcs[i].name != NULL; ++i) {
        lua_pushcfunction(L, funcs[i].func);
        switch (funcs[i].name[0]) {
            case '_': lua_setfield(L, -5, funcs[i].name); break; // set directly to meta-table
            case '?': lua_setfield(L, -4, &funcs[i].name[1]); break; // set to index table
            case '=': lua_setfield(L, -3, &funcs[i].name[1]); break; // set to new-index table
            default: lua_setfield(L, -2, funcs[i].name); break; // set to the method table
        }
    }
    lua_pop(L, 4);
}

static int push_index(lua_State *L, const char *name) {
    if (luaL_getmetatable(L, name) != LUA_TTABLE)
        return 0;
    const char *key = lua_tostring(L, 2);
    int index = lua_absindex(L, -1);
    // try to locate method
    if (lua_rawgeti(L, index, METHOD_TABLE) == LUA_TTABLE) {
        if (lua_getfield(L, -1, key) == LUA_TFUNCTION)
            return 1;
    }
    // try to locate component
    if (lua_rawgeti(L, index, INDEX_TABLE) == LUA_TTABLE) {
        if (lua_getfield(L, -1, key) == LUA_TFUNCTION) {
            lua_pushvalue(L, 1);
            lua_call(L, 1, 1);
            return 1;
        }
    }
    return 0;
}

static int push_newindex(lua_State *L, const char *name) {
    if (luaL_getmetatable(L, name) != LUA_TTABLE)
        return 0;
    const char *key = lua_tostring(L, 2);
    int index = lua_absindex(L, -1);
    // try to locate component
    if (lua_rawgeti(L, index, NEWINDEX_TABLE) == LUA_TTABLE) {
        if (lua_getfield(L, -1, key) == LUA_TFUNCTION) {
            lua_pushvalue(L, 1);
            lua_pushvalue(L, 3);
            lua_call(L, 2, 0);
        }
    }
    return 0;
}


//==[[ Vector2 object ]]========================================================

static int push_Vector2(lua_State *L, const Vector2 vector) {
    *((Vector2*)push_object(L, "Vector2", sizeof(Vector2), 0)) = vector;
    return 1;
}

static Vector2 *check_Vector2(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Vector2");
}

static Vector2 *test_Vector2(lua_State *L, const int idx) {
    return luaL_testudata(L, idx, "Vector2");
}

static Vector2 *check_Vector2_List(lua_State *L, const int idx, int *count) {
    static Vector2 points[1024];
    int i;
    luaL_argcheck(L, lua_type(L, idx) == LUA_TTABLE, idx, "table with Vector2 expected");
    for (i = 0; i < 1024; ++i) {
        lua_rawgeti(L, idx, i + 1);
        const Vector2 *vector = test_Vector2(L, -1);
        if (vector == NULL) break;
        points[i] = *vector;
        lua_pop(L, 1);
    }
    *count = i;
    return points;
}

static int f_Vector2(lua_State *L) {
    switch (lua_gettop(L)) {
        case 0: return push_Vector2(L, Vector2Zero());
        case 1: return push_Vector2(L, *check_Vector2(L, 1));
        case 2: return push_Vector2(L, (Vector2){ .x = (float)luaL_checknumber(L, 1), .y = (float)luaL_checknumber(L, 2) });
        default: return luaL_error(L, "wrong number of arguments");
    }
}

static int f_Vector2__index(lua_State *L) {
    return push_index(L, "Vector2");
}

static int f_Vector2__newindex(lua_State *L) {
    return push_newindex(L, "Vector2");
}

static int f_Vector2__tostring(lua_State *L) {
    const Vector2 *vector = check_Vector2(L, 1);
    lua_pushfstring(L, "Vector2(x = %f, y = %f)", vector->x, vector->y);
    return 1;
}

static int f_Vector2__add(lua_State *L) {
    const Vector2 *v1 = check_Vector2(L, 1);
    const Vector2 *v2 = test_Vector2(L, 2);
    if (v2 != NULL) return push_Vector2(L, Vector2Add(*v1, *v2));
    return push_Vector2(L, Vector2AddValue(*v1, (float)luaL_checknumber(L, 2)));
}

static int f_Vector2__sub(lua_State *L) {
    const Vector2 *v1 = check_Vector2(L, 1);
    const Vector2 *v2 = test_Vector2(L, 2);
    if (v2 != NULL) return push_Vector2(L, Vector2Subtract(*v1, *v2));
    return push_Vector2(L, Vector2SubtractValue(*v1, (float)luaL_checknumber(L, 2)));
}

static int f_Vector2__mul(lua_State *L) {
    const Vector2 *v1 = check_Vector2(L, 1);
    const Vector2 *v2 = test_Vector2(L, 2);
    if (v2 != NULL) return push_Vector2(L, Vector2Multiply(*v1, *v2));
    return push_Vector2(L, Vector2Scale(*v1, (float)luaL_checknumber(L, 2)));
}

static int f_Vector2__div(lua_State *L) {
    const Vector2 *v1 = check_Vector2(L, 1);
    const Vector2 *v2 = test_Vector2(L, 2);
    if (v2 != NULL) return push_Vector2(L, Vector2Divide(*v1, *v2));
    return push_Vector2(L, Vector2Scale(*v1, 1.0f / (float)luaL_checknumber(L, 2)));
}

static int f_Vector2__unm(lua_State *L) {
    return push_Vector2(L, Vector2Negate(*check_Vector2(L, 1)));
}

static int f_Vector2_Length(lua_State *L) {
    lua_pushnumber(L, Vector2Length(*check_Vector2(L, 1)));
    return 1;
}

static int f_Vector2_Distance(lua_State *L) {
    lua_pushnumber(L, Vector2Distance(*check_Vector2(L, 1), *check_Vector2(L, 2)));
    return 1;
}

static int f_Vector2_Normal(lua_State *L) {
    return push_Vector2(L, Vector2Normalize(*check_Vector2(L, 1)));
}

static int f_Vector2_Angle(lua_State *L) {
    lua_pushnumber(L, Vector2Angle(*check_Vector2(L, 1), *check_Vector2(L, 2)));
    return 1;
}

static int f_Vector2_get_x(lua_State *L) {
    lua_pushnumber(L, check_Vector2(L, 1)->x);
    return 1;
}

static int f_Vector2_set_x(lua_State *L) {
    check_Vector2(L, 1)->x = (float)luaL_checknumber(L, 2);
    return 0;
}

static int f_Vector2_get_y(lua_State *L) {
    lua_pushnumber(L, check_Vector2(L, 1)->y);
    return 1;
}

static int f_Vector2_set_y(lua_State *L) {
    check_Vector2(L, 1)->y = (float)luaL_checknumber(L, 2);
    return 0;
}

//==[[ Vector3 object ]]========================================================

static int push_Vector3(lua_State *L, const Vector3 vector) {
    *((Vector3*)push_object(L, "Vector3", sizeof(Vector3), 0)) = vector;
    return 1;
}

static Vector3 *check_Vector3(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Vector3");
}

static Vector3 *test_Vector3(lua_State *L, const int idx) {
    return luaL_testudata(L, idx, "Vector3");
}

static int f_Vector3(lua_State *L) {
    switch (lua_gettop(L)) {
        case 0: return push_Vector3(L, Vector3Zero());
        case 1: return push_Vector3(L, *check_Vector3(L, 1));
        case 3: return push_Vector3(L, (Vector3){ .x = (float)luaL_checknumber(L, 1), .y = (float)luaL_checknumber(L, 2), .z = (float)luaL_checknumber(L, 3) });
        default: return luaL_error(L, "wrong number of arguments");
    }
}

static int f_Vector3__tostring(lua_State *L) {
    const Vector3 *vector = check_Vector3(L, 1);
    lua_pushfstring(L, "Vector3(x = %f, y = %f, z = %f)", vector->x, vector->y, vector->z);
    return 1;
}

static int f_Vector3__index(lua_State *L) {
    return push_index(L, "Vector3");
}

static int f_Vector3__newindex(lua_State *L) {
    return push_newindex(L, "Vector3");
}

static int f_Vector3__add(lua_State *L) {
    const Vector3 *v1 = check_Vector3(L, 1);
    const Vector3 *v2 = test_Vector3(L, 2);
    if (v2 != NULL) return push_Vector3(L, Vector3Add(*v1, *v2));
    return push_Vector3(L, Vector3AddValue(*v1, (float)luaL_checknumber(L, 2)));
}

static int f_Vector3__sub(lua_State *L) {
    const Vector3 *v1 = check_Vector3(L, 1);
    const Vector3 *v2 = test_Vector3(L, 2);
    if (v2 != NULL) return push_Vector3(L, Vector3Subtract(*v1, *v2));
    return push_Vector3(L, Vector3SubtractValue(*v1, (float)luaL_checknumber(L, 2)));
}

static int f_Vector3__mul(lua_State *L) {
    const Vector3 *v1 = check_Vector3(L, 1);
    const Vector3 *v2 = test_Vector3(L, 2);
    if (v2 != NULL) return push_Vector3(L, Vector3Multiply(*v1, *v2));
    return push_Vector3(L, Vector3Scale(*v1, (float)luaL_checknumber(L, 2)));
}

static int f_Vector3__div(lua_State *L) {
    const Vector3 *v1 = check_Vector3(L, 1);
    const Vector3 *v2 = test_Vector3(L, 2);
    if (v2 != NULL) return push_Vector3(L, Vector3Divide(*v1, *v2));
    return push_Vector3(L, Vector3Scale(*v1, 1.0f / (float)luaL_checknumber(L, 2)));
}

static int f_Vector3__unm(lua_State *L) {
    return push_Vector3(L, Vector3Negate(*check_Vector3(L, 1)));
}

static int f_Vector3__eq(lua_State *L) {
    lua_pushboolean(L, Vector3Equals(*check_Vector3(L, 1), *check_Vector3(L, 2)));
    return 1;
}

static int f_Vector3_Length(lua_State *L) {
    lua_pushnumber(L, Vector3Length(*check_Vector3(L, 1)));
    return 1;
}

static int f_Vector3_Distance(lua_State *L) {
    lua_pushnumber(L, Vector3Distance(*check_Vector3(L, 1), *check_Vector3(L, 2)));
    return 1;
}

static int f_Vector3_Normal(lua_State *L) {
    return push_Vector3(L, Vector3Normalize(*check_Vector3(L, 1)));
}

static int f_Vector3_get_x(lua_State *L) {
    lua_pushnumber(L, check_Vector3(L, 1)->x);
    return 1;
}

static int f_Vector3_set_x(lua_State *L) {
    check_Vector3(L, 1)->x = (float)luaL_checknumber(L, 2);
    return 0;
}


static int f_Vector3_get_y(lua_State *L) {
    lua_pushnumber(L, check_Vector3(L, 1)->y);
    return 1;
}

static int f_Vector3_set_y(lua_State *L) {
    check_Vector3(L, 1)->y = (float)luaL_checknumber(L, 2);
    return 0;
}


static int f_Vector3_get_z(lua_State *L) {
    lua_pushnumber(L, check_Vector3(L, 1)->z);
    return 1;
}

static int f_Vector3_set_z(lua_State *L) {
    check_Vector3(L, 1)->z = (float)luaL_checknumber(L, 2);
    return 0;
}


//==[[ Color object ]]==========================================================

static int push_Color(lua_State *L, const Color color) {
    *((Color*)push_object(L, "Color", sizeof(Color), 0)) = color;
    return 1;
}

static Color *check_Color(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Color");
}

static unsigned char check_color_component(lua_State *L, const int idx) {
    return (unsigned char)Clamp((float)luaL_checknumber(L, idx), 0.0f, 255.0f);
}

static int f_Color(lua_State *L) {
    switch (lua_gettop(L)) {
        case 1: return push_Color(L, *check_Color(L, 1));
        case 3:
        case 4:
            return push_Color(L, (Color){
                .r = check_color_component(L, 1),
                .g = check_color_component(L, 2),
                .b = check_color_component(L, 3),
                .a = (unsigned char)Clamp((float)luaL_optnumber(L, 4, 255.0), 0.0f, 255.0f),
            });
        default: return luaL_error(L, "wrong number of arguments");
    }
}

static int f_Color__tostring(lua_State *L) {
    const Color *color = check_Color(L, 1);
    lua_pushfstring(L, "Color(r = %d, g = %d, b = %d, a = %d)", color->r, color->g, color->b, color->a);
    return 1;
}

static int f_Color__index(lua_State *L) {
    return push_index(L, "Color");
}

static int f_Color__newindex(lua_State *L) {
    return push_newindex(L, "Color");
}

static int f_Color_Fade(lua_State *L) {
    return push_Color(L, Fade(*check_Color(L, 1), (float)luaL_checknumber(L, 2)));
}

static int f_Color_get_r(lua_State *L) {
    lua_pushinteger(L, check_Color(L, 1)->r);
    return 1;
}

static int f_Color_set_r(lua_State *L) {
    check_Color(L, 1)->r = check_color_component(L, 2);
    return 0;
}


static int f_Color_get_g(lua_State *L) {
    lua_pushinteger(L, check_Color(L, 1)->g);
    return 1;
}

static int f_Color_set_g(lua_State *L) {
    check_Color(L, 1)->g = check_color_component(L, 2);
    return 0;
}


static int f_Color_get_b(lua_State *L) {
    lua_pushinteger(L, check_Color(L, 1)->b);
    return 1;
}

static int f_Color_set_b(lua_State *L) {
    check_Color(L, 1)->b = check_color_component(L, 2);
    return 0;
}


static int f_Color_get_a(lua_State *L) {
    lua_pushinteger(L, check_Color(L, 1)->a);
    return 1;
}

static int f_Color_set_a(lua_State *L) {
    check_Color(L, 1)->a = check_color_component(L, 2);
    return 0;
}


//==[[ Rectangle object ]]======================================================

static int push_Rectangle(lua_State *L, const Rectangle rect) {
    *((Rectangle*)push_object(L, "Rectangle", sizeof(Rectangle), 0)) = rect;
    return 1;
}

static Rectangle *check_Rectangle(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Rectangle");
}

static int f_Rectangle(lua_State *L) {
    switch (lua_gettop(L)) {
        case 1: return push_Rectangle(L, *check_Rectangle(L, 1));
        case 4: return push_Rectangle(L, (Rectangle){
                    .x = (float)luaL_checknumber(L, 1), .y = (float)luaL_checknumber(L, 2),
                    .width = (float)luaL_checknumber(L, 3), .height = (float)luaL_checknumber(L, 4)
                });
        default: return luaL_error(L, "wrong number of arguments");
    }
}

static int f_Rectangle__tostring(lua_State *L) {
    const Rectangle *rect = check_Rectangle(L, 1);
    lua_pushfstring(L, "Rectangle(x = %f, y = %f, width = %f, height = %f)", rect->x, rect->y, rect->width, rect->height);
    return 1;
}

static int f_Rectangle__index(lua_State *L) {
    return push_index(L, "Rectangle");
}

static int f_Rectangle__newindex(lua_State *L) {
    return push_newindex(L, "Rectangle");
}

static int f_Rectangle_get_x(lua_State *L) {
    lua_pushnumber(L, check_Rectangle(L, 1)->x);
    return 1;
}

static int f_Rectangle_set_x(lua_State *L) {
    check_Rectangle(L, 1)->x = (float)luaL_checknumber(L, 2);
    return 0;
}


static int f_Rectangle_get_y(lua_State *L) {
    lua_pushnumber(L, check_Rectangle(L, 1)->y);
    return 1;
}

static int f_Rectangle_set_y(lua_State *L) {
    check_Rectangle(L, 1)->y = (float)luaL_checknumber(L, 2);
    return 0;
}


static int f_Rectangle_get_width(lua_State *L) {
    lua_pushnumber(L, check_Rectangle(L, 1)->width);
    return 1;
}

static int f_Rectangle_set_width(lua_State *L) {
    check_Rectangle(L, 1)->width = (float)luaL_checknumber(L, 2);
    return 0;
}


static int f_Rectangle_get_height(lua_State *L) {
    lua_pushnumber(L, check_Rectangle(L, 1)->height);
    return 1;
}

static int f_Rectangle_set_height(lua_State *L) {
    check_Rectangle(L, 1)->height = (float)luaL_checknumber(L, 2);
    return 0;
}


//==[[ Image object ]]==========================================================

static int push_Image(lua_State *L, const Image image) {
    *((Image*)push_object(L, "Image", sizeof(Image), 0)) = image;
    return 1;
}

static Image *check_Image(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Image");
}

static int f_Image__tostring(lua_State *L) {
    const Image *image = check_Image(L, 1);
    lua_pushfstring(L, "Image(id = %p, width = %d, height = %d)", image, image->width, image->height);
    return 1;
}

static int f_Image__gc(lua_State *L) {
    UnloadImage(*check_Image(L, 1));
    return 0;
}

static int f_Image__index(lua_State *L) {
    return push_index(L, "Image");
}


//==[[ Texture object ]]========================================================

static int push_Texture(lua_State *L, const Texture texture) {
    *((Texture*)push_object(L, "Texture", sizeof(Texture), 0)) = texture;
    return 1;
}

static Texture *check_Texture(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Texture");
}

static int f_Texture__tostring(lua_State *L) {
    const Texture *texture = check_Texture(L, 1);
    lua_pushfstring(L, "Texture(id = %d, width = %d, height = %d)", texture->id, texture->width, texture->height);
    return 1;
}

static int f_Texture__index(lua_State *L) {
    return push_index(L, "Texture");
}

static int f_Texture__gc(lua_State *L) {
    UnloadTexture(*check_Texture(L, 1));
    return 0;
}

static int f_Texture_get_width(lua_State *L) {
    lua_pushinteger(L, check_Texture(L, 1)->width);
    return 1;
}

static int f_Texture_get_height(lua_State *L) {
    lua_pushinteger(L, check_Texture(L, 1)->height);
    return 1;
}


//==[[ Font object ]]===========================================================

static int push_Font(lua_State *L, const Font font) {
    *((Font*)push_object(L, "Font", sizeof(Font), 0)) = font;
    return 1;
}

static Font *check_Font(lua_State *L, const int idx) {
    return luaL_checkudata(L, idx, "Font");
}

static int f_Font__tostring(lua_State *L) {
    Font *font = check_Font(L, 1);
    lua_pushfstring(L, "Font(%p)", font);
    return 1;
}

static int f_Font__gc(lua_State *L) {
    UnloadFont(*check_Font(L, 1));
    return 0;
}


//==[[ Color object ]]==========================================================

static int push_FilePathList(lua_State *L, FilePathList list, void (*unload)(FilePathList)) {
    lua_createtable(L, list.count, 0);
    for (unsigned int i = 0; i < list.count; ++i) {
        lua_pushstring(L, list.paths[i]);
        lua_rawseti(L, -2, i + 1);
    }
    (*unload)(list);
    return 1;
}


//==[[ module: core ]]==========================================================

// Window-related functions ----------------------------------------------------

static int f_InitWindow(lua_State *L) {
    InitWindow(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkstring(L, 3));
    return 0;
}

static int f_WindowShouldClose(lua_State *L) {
    lua_pushboolean(L, WindowShouldClose());
    return 1;
}

static int f_CloseWindow(lua_State *L) {
    (void)L;
    CloseWindow();
    return 0;
}

static int f_IsWindowReady(lua_State *L) {
    lua_pushboolean(L, IsWindowReady());
    return 1;
}

static int f_IsWindowFullscreen(lua_State *L) {
    lua_pushboolean(L, IsWindowFullscreen());
    return 1;
}

static int f_IsWindowHidden(lua_State *L) {
    lua_pushboolean(L, IsWindowHidden());
    return 1;
}

static int f_IsWindowMinimized(lua_State *L) {
    lua_pushboolean(L, IsWindowMinimized());
    return 1;
}

static int f_IsWindowMaximized(lua_State *L) {
    lua_pushboolean(L, IsWindowMaximized());
    return 1;
}

static int f_IsWindowFocused(lua_State *L) {
    lua_pushboolean(L, IsWindowFocused());
    return 1;
}

static int f_IsWindowResized(lua_State *L) {
    lua_pushboolean(L, IsWindowResized());
    return 1;
}

static int f_IsWindowState(lua_State *L) {
    lua_pushboolean(L, IsWindowState(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_SetWindowState(lua_State *L) {
    SetWindowState(luaL_checkinteger(L, 1));
    return 0;
}

static int f_ClearWindowState(lua_State *L) {
    ClearWindowState(luaL_checkinteger(L, 1));
    return 0;
}

static int f_ToggleFullscreen(lua_State *L) {
    (void)L; ToggleFullscreen();
    return 0;
}

static int f_MaximizeWindow(lua_State *L) {
    (void)L; MaximizeWindow();
    return 0;
}

static int f_MinimizeWindow(lua_State *L) {
    (void)L; MinimizeWindow();
    return 0;
}

static int f_RestoreWindow(lua_State *L) {
    (void)L; RestoreWindow();
    return 0;
}

static int f_SetWindowIcon(lua_State *L) {
    SetWindowIcon(*check_Image(L, 1));
    return 0;
}

static int f_SetWindowTitle(lua_State *L) {
    SetWindowTitle(luaL_checkstring(L, 1));
    return 0;
}

static int f_SetWindowPosition(lua_State *L) {
    SetWindowPosition(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
    return 0;
}

static int f_SetWindowMinSize(lua_State *L) {
    SetWindowMinSize(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
    return 0;
}

static int f_SetWindowSize(lua_State *L) {
    SetWindowSize(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
    return 0;
}

static int f_SetWindowOpacity(lua_State *L) {
    SetWindowOpacity((float)luaL_checknumber(L, 1));
    return 0;
}

static int f_GetScreenSize(lua_State *L) {
    lua_pushinteger(L, GetScreenWidth());
    lua_pushinteger(L, GetScreenHeight());
    return 2;
}

static int f_GetRenderSize(lua_State *L) {
    lua_pushinteger(L, GetRenderWidth());
    lua_pushinteger(L, GetRenderHeight());
    return 2;
}

static int f_SetClipboardText(lua_State *L) {
    SetClipboardText(luaL_checkstring(L, 1));
    return 0;
}

static int f_GetClipboardText(lua_State *L) {
    lua_pushstring(L, GetClipboardText());
    return 1;
}


// Custom frame control functions ----------------------------------------------

static int f_SwapScreenBuffer(lua_State *L) {
    (void)L; SwapScreenBuffer();
    return 0;
}

static int f_PollInputEvents(lua_State *L) {
    (void)L; PollInputEvents();
    return 0;
}

static int f_WaitTime(lua_State *L) {
    WaitTime(luaL_checknumber(L, 1));
    return 0;
}


// Cursor-related functions ----------------------------------------------------

static int f_ShowCursor(lua_State *L) {
    (void)L; ShowCursor(); return 0;
}

static int f_HideCursor(lua_State *L) {
    (void)L; HideCursor(); return 0;
}

static int f_IsCursorHidden(lua_State *L) {
    lua_pushboolean(L, IsCursorHidden());
    return 1;
}

static int f_EnableCursor(lua_State *L) {
    (void)L; EnableCursor(); return 0;
}

static int f_DisableCursor(lua_State *L) {
    (void)L; DisableCursor(); return 0;
}

static int f_IsCursorOnScreen(lua_State *L) {
    lua_pushboolean(L, IsCursorOnScreen());
    return 1;
}


// Drawing-related functions ---------------------------------------------------

static int f_ClearBackground(lua_State *L) {
    ClearBackground(*check_Color(L, 1));
    return 1;
}

static int f_BeginDrawing(lua_State *L) {
    (void)L; BeginDrawing();
    return 0;
}

static int f_EndDrawing(lua_State *L) {
    (void)L; EndDrawing();
    return 0;
}

static int f_BeginBlendMode(lua_State *L) {
    BeginBlendMode(luaL_checkinteger(L, 1));
    return 0;
}

static int f_EndBlendMode(lua_State *L) {
    (void)L; EndBlendMode();
    return 0;
}

static int f_BeginScissorMode(lua_State *L) {
    BeginScissorMode((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4));
    return 0;
}

static int f_EndScissorMode(lua_State *L) {
    (void)L; EndScissorMode();
    return 0;
}


// Timing-related functions ----------------------------------------------------

static int f_SetTargetFPS(lua_State *L) {
    SetTargetFPS(luaL_checkinteger(L, 1));
    return 0;
}

static int f_GetFPS(lua_State *L) {
    lua_pushinteger(L, GetFPS());
    return 1;
}

static int f_GetFrameTime(lua_State *L) {
    lua_pushnumber(L, GetFrameTime());
    return 1;
}

static int f_GetTime(lua_State *L) {
    lua_pushnumber(L, GetTime());
    return 1;
}


// Files management functions --------------------------------------------------

static int f_LoadFileData(lua_State *L) {
    unsigned int length;
    unsigned char *data = LoadFileData(luaL_checkstring(L, 1), &length);
    lua_pushlstring(L, (const char*)data, length);
    UnloadFileData(data);
    return 1;
}

static int f_SaveFileData(lua_State *L) {
    size_t length;
    const char *filename = luaL_checkstring(L, 1);
    const char *data = luaL_checklstring(L, 2, &length);
    lua_pushboolean(L, SaveFileData(filename, (void*)data, (unsigned int)length));
    return 1;
}

static int f_FileExists(lua_State *L) {
    lua_pushboolean(L, FileExists(luaL_checkstring(L, 1)));
    return 1;
}

static int f_DirectoryExists(lua_State *L) {
    lua_pushboolean(L, DirectoryExists(luaL_checkstring(L, 1)));
    return 1;
}

static int f_IsFileExtension(lua_State *L) {
    lua_pushboolean(L, IsFileExtension(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));
    return 1;
}

static int f_GetFileLength(lua_State *L) {
    lua_pushinteger(L, GetFileLength(luaL_checkstring(L, 1)));
    return 1;
}

static int f_GetFileExtension(lua_State *L) {
    lua_pushstring(L, GetFileExtension(luaL_checkstring(L, 1)));
    return 1;
}

static int f_GetFileName(lua_State *L) {
    lua_pushstring(L, GetFileName(luaL_checkstring(L, 1)));
    return 1;
}

static int f_GetFileNameWithoutExt(lua_State *L) {
    lua_pushstring(L, GetFileNameWithoutExt(luaL_checkstring(L, 1)));
    return 1;
}

static int f_GetDirectoryPath(lua_State *L) {
    lua_pushstring(L, GetDirectoryPath(luaL_checkstring(L, 1)));
    return 1;
}

static int f_GetPrevDirectoryPath(lua_State *L) {
    lua_pushstring(L, GetPrevDirectoryPath(luaL_checkstring(L, 1)));
    return 1;
}

static int f_GetWorkingDirectory(lua_State *L) {
    lua_pushstring(L, GetWorkingDirectory());
    return 1;
}

static int f_GetApplicationDirectory(lua_State *L) {
    lua_pushstring(L, GetApplicationDirectory());
    return 1;
}

static int f_ChangeDirectory(lua_State *L) {
    lua_pushboolean(L, ChangeDirectory(luaL_checkstring(L, 1)));
    return 1;
}

static int f_IsPathFile(lua_State *L) {
    lua_pushboolean(L, IsPathFile(luaL_checkstring(L, 1)));
    return 1;
}

static int f_LoadDirectoryFiles(lua_State *L) {
    return push_FilePathList(L, LoadDirectoryFiles(luaL_checkstring(L, 1)), UnloadDirectoryFiles);
}

static int f_LoadDirectoryFilesEx(lua_State *L) {
    return push_FilePathList(L, LoadDirectoryFilesEx(luaL_checkstring(L, 1), luaL_checkstring(L, 2), lua_toboolean(L, 3)), UnloadDirectoryFiles);
}

static int f_IsFileDropped(lua_State *L) {
    lua_pushboolean(L, IsFileDropped());
    return 1;
}

static int f_LoadDroppedFiles(lua_State *L) {
    return push_FilePathList(L, LoadDroppedFiles(), UnloadDroppedFiles);
}

static int f_GetFileModTime(lua_State *L) {
    lua_pushinteger(L, GetFileModTime(luaL_checkstring(L, 1)));
    return 1;
}


// Compression/Encoding functionality ------------------------------------------

static int f_CompressData(lua_State *L) {
    size_t input_length;
    int output_length;
    const char *input_data = luaL_checklstring(L, 1, &input_length);
    unsigned char *output_data = CompressData((unsigned char*)input_data, (int)input_length, &output_length);
    lua_pushlstring(L, (char*)output_data, output_length);
    MemFree(output_data);
    return 1;
}

static int f_DecompressData(lua_State *L) {
    size_t input_length;
    int output_length;
    const char *input_data = luaL_checklstring(L, 1, &input_length);
    unsigned char *output_data = DecompressData((unsigned char*)input_data, (int)input_length, &output_length);
    lua_pushlstring(L, (char*)output_data, output_length);
    MemFree(output_data);
    return 1;
}

static int f_EncodeDataBase64(lua_State *L) {
    size_t input_length;
    int output_length;
    const char *input_data = luaL_checklstring(L, 1, &input_length);
    char *output_data = EncodeDataBase64((unsigned char*)input_data, (int)input_length, &output_length);
    lua_pushlstring(L, output_data, output_length);
    MemFree(output_data);
    return 1;
}

static int f_DecodeDataBase64(lua_State *L) {
    int output_length;
    const char *input_data = luaL_checkstring(L, 1);
    unsigned char *output_data = DecodeDataBase64((unsigned char*)input_data, &output_length);
    lua_pushlstring(L, (char*)output_data, output_length);
    MemFree(output_data);
    return 1;
}


// Input-related functions: keyboard -------------------------------------------

static int f_IsKeyPressed(lua_State *L) {
    lua_pushboolean(L, IsKeyPressed(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsKeyDown(lua_State *L) {
    lua_pushboolean(L, IsKeyDown(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsKeyReleased(lua_State *L) {
    lua_pushboolean(L, IsKeyReleased(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsKeyUp(lua_State *L) {
    lua_pushboolean(L, IsKeyUp(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_SetExitKey(lua_State *L) {
    SetExitKey(luaL_optinteger(L, 1, KEY_NULL));
    return 0;
}

static int f_GetKeyPressed(lua_State *L) {
    lua_pushinteger(L, GetKeyPressed());
    return 1;
}

static int f_GetCharPressed(lua_State *L) {
    lua_pushinteger(L, GetCharPressed());
    return 1;
}

// Input-related functions: gamepads -------------------------------------------

static int f_IsGamepadAvailable(lua_State *L) {
    lua_pushboolean(L, IsGamepadAvailable(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_GetGamepadName(lua_State *L) {
    lua_pushstring(L, GetGamepadName(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsGamepadButtonPressed(lua_State *L) {
    lua_pushboolean(L, IsGamepadButtonPressed(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

static int f_IsGamepadButtonDown(lua_State *L) {
    lua_pushboolean(L, IsGamepadButtonDown(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

static int f_IsGamepadButtonReleased(lua_State *L) {
    lua_pushboolean(L, IsGamepadButtonReleased(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

static int f_IsGamepadButtonUp(lua_State *L) {
    lua_pushboolean(L, IsGamepadButtonUp(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

static int f_GetGamepadButtonPressed(lua_State *L) {
    lua_pushinteger(L, GetGamepadButtonPressed());
    return 1;
}

static int f_GetGamepadAxisCount(lua_State *L) {
    lua_pushinteger(L, GetGamepadAxisCount(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_GetGamepadAxisMovement(lua_State *L) {
    lua_pushnumber(L, GetGamepadAxisMovement(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2)));
    return 1;
}

// Input-related functions: mouse ----------------------------------------------

static int f_IsMouseButtonPressed(lua_State *L) {
    lua_pushboolean(L, IsMouseButtonPressed(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsMouseButtonDown(lua_State *L) {
    lua_pushboolean(L, IsMouseButtonDown(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsMouseButtonReleased(lua_State *L) {
    lua_pushboolean(L, IsMouseButtonReleased(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_IsMouseButtonUp(lua_State *L) {
    lua_pushboolean(L, IsMouseButtonUp(luaL_checkinteger(L, 1)));
    return 1;
}

static int f_GetMousePosition(lua_State *L) {
    return push_Vector2(L, GetMousePosition());
}

static int f_GetMouseDelta(lua_State *L) {
    return push_Vector2(L, GetMouseDelta());
}

static int f_SetMousePosition(lua_State *L) {
    const Vector2 *vector = check_Vector2(L, 1);
    SetMousePosition((int)vector->x, (int)vector->y);
    return 0;
}

static int f_SetMouseOffset(lua_State *L) {
    const Vector2 *vector = check_Vector2(L, 1);
    SetMouseOffset((int)vector->x, (int)vector->y);
    return 0;
}

static int f_SetMouseScale(lua_State *L) {
    const Vector2 *vector = check_Vector2(L, 1);
    SetMouseScale(vector->x, vector->y);
    return 0;
}

static int f_GetMouseWheelMove(lua_State *L) {
    return push_Vector2(L, GetMouseWheelMoveV());
}

static int f_SetMouseCursor(lua_State *L) {
    SetMouseCursor(luaL_checkinteger(L, 1));
    return 0;
}


//==[[ module: rshapes ]]=======================================================

static int f_SetShapesTexture(lua_State *L) {
    SetShapesTexture(*check_Texture(L, 1), *check_Rectangle(L, 2));
    return 0;
}

// Basic Shapes Drawing Functions ----------------------------------------------

static int f_DrawPixel(lua_State *L) {
    DrawPixel((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_DrawPixelV(lua_State *L) {
    DrawPixelV(*check_Vector2(L, 1), *check_Color(L, 2));
    return 0;
}

static int f_DrawLine(lua_State *L) {
    DrawLine((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawLineV(lua_State *L) {
    DrawLineV(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_DrawLineEx(lua_State *L) {
    DrawLineEx(*check_Vector2(L, 1), *check_Vector2(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawLineBezier(lua_State *L) {
    DrawLineBezier(*check_Vector2(L, 1), *check_Vector2(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawLineBezierQuad(lua_State *L) {
    DrawLineBezierQuad(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawLineBezierCubic(lua_State *L) {
    DrawLineBezierCubic(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Vector2(L, 4), (float)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_DrawLineStrip(lua_State *L) {
    int count;
    Vector2 *points = check_Vector2_List(L, 1, &count);
    DrawLineStrip(points, count, *check_Color(L, 2));
    return 0;
}

static int f_DrawCircle(lua_State *L) {
    DrawCircle((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawCircleSector(lua_State *L) {
    DrawCircleSector(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), luaL_checkinteger(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_DrawCircleSectorLines(lua_State *L) {
    DrawCircleSectorLines(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), luaL_checkinteger(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_DrawCircleGradient(lua_State *L) {
    DrawCircleGradient((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawCircleV(lua_State *L) {
    DrawCircleV(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_DrawCircleLines(lua_State *L) {
    DrawCircleLines((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawEllipse(lua_State *L) {
    DrawEllipse((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawEllipseLines(lua_State *L) {
    DrawEllipseLines((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawRing(lua_State *L) {
    DrawRing(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), luaL_checkinteger(L, 6), *check_Color(L, 7));
    return 0;
}

static int f_DrawRingLines(lua_State *L) {
    DrawRingLines(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), luaL_checkinteger(L, 6), *check_Color(L, 7));
    return 0;
}

static int f_DrawRectangle(lua_State *L) {
    DrawRectangle((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawRectangleV(lua_State *L) {
    DrawRectangleV(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_DrawRectangleRec(lua_State *L) {
    DrawRectangleRec(*check_Rectangle(L, 1), *check_Color(L, 2));
    return 0;
}

static int f_DrawRectanglePro(lua_State *L) {
    DrawRectanglePro(*check_Rectangle(L, 1), *check_Vector2(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawRectangleGradientV(lua_State *L) {
    DrawRectangleGradientV((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), *check_Color(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_DrawRectangleGradientH(lua_State *L) {
    DrawRectangleGradientV((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), *check_Color(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_DrawRectangleGradientEx(lua_State *L) {
    DrawRectangleGradientEx(*check_Rectangle(L, 1), *check_Color(L, 2), *check_Color(L, 3), *check_Color(L, 4), *check_Color(L, 6));
    return 0;
}

static int f_DrawRectangleLines(lua_State *L) {
    DrawRectangleLines((int)luaL_checknumber(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawRectangleLinesEx(lua_State *L) {
    DrawRectangleLinesEx(*check_Rectangle(L, 1), (float)luaL_checknumber(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_DrawRectangleRounded(lua_State *L) {
    DrawRectangleRounded(*check_Rectangle(L, 1), (float)luaL_checknumber(L, 2), luaL_checkinteger(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawRectangleRoundedLines(lua_State *L) {
    DrawRectangleRoundedLines(*check_Rectangle(L, 1), (float)luaL_checknumber(L, 2), luaL_checkinteger(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawTriangle(lua_State *L) {
    DrawTriangle(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawTriangleLines(lua_State *L) {
    DrawTriangleLines(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_DrawTriangleFan(lua_State *L) {
    int count;
    Vector2 *points = check_Vector2_List(L, 1, &count);
    DrawTriangleFan(points, count, *check_Color(L, 2));
    return 0;
}

static int f_DrawTriangleStrip(lua_State *L) {
    int count;
    Vector2 *points = check_Vector2_List(L, 1, &count);
    DrawTriangleStrip(points, count, *check_Color(L, 2));
    return 0;
}

static int f_DrawPoly(lua_State *L) {
    DrawPoly(*check_Vector2(L, 1), luaL_checkinteger(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawPolyLines(lua_State *L) {
    DrawPolyLines(*check_Vector2(L, 1), luaL_checkinteger(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawPolyLinesEx(lua_State *L) {
    DrawPolyLinesEx(*check_Vector2(L, 1), luaL_checkinteger(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}


// Basic shapes collision detection functions ----------------------------------

static int f_CheckCollisionRecs(lua_State *L) {
    lua_pushboolean(L, CheckCollisionRecs(*check_Rectangle(L, 1), *check_Rectangle(L, 2)));
    return 1;
}

static int f_CheckCollisionCircles(lua_State *L) {
    lua_pushboolean(L, CheckCollisionCircles(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), *check_Vector2(L, 3), (float)luaL_checknumber(L, 4)));
    return 1;
}

static int f_CheckCollisionCircleRec(lua_State *L) {
    lua_pushboolean(L, CheckCollisionCircleRec(*check_Vector2(L, 1), (float)luaL_checknumber(L, 2), *check_Rectangle(L, 3)));
    return 1;
}

static int f_CheckCollisionPointRec(lua_State *L) {
    lua_pushboolean(L, CheckCollisionPointRec(*check_Vector2(L, 1), *check_Rectangle(L, 2)));
    return 1;
}

static int f_CheckCollisionPointCircle(lua_State *L) {
    lua_pushboolean(L, CheckCollisionPointCircle(*check_Vector2(L, 1), *check_Vector2(L, 2), (float)luaL_checknumber(L, 3)));
    return 1;
}

static int f_CheckCollisionPointTriangle(lua_State *L) {
    lua_pushboolean(L, CheckCollisionPointTriangle(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Vector2(L, 4)));
    return 1;
}

static int f_CheckCollisionLines(lua_State *L) {
    Vector2 collision;
    lua_pushboolean(L, CheckCollisionLines(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Vector2(L, 4), &collision));
    push_Vector2(L, collision);
    return 2;
}

static int f_CheckCollisionPointLine(lua_State *L) {
    lua_pushboolean(L, CheckCollisionPointLine(*check_Vector2(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), luaL_checkinteger(L, 4)));
    return 1;
}

static int f_GetCollisionRec(lua_State *L) {
    return push_Rectangle(L, GetCollisionRec(*check_Rectangle(L, 1), *check_Rectangle(L, 2)));
}


//==[[ module: rtextures ]]=====================================================

// Image loading functions -----------------------------------------------------

static int f_LoadImage(lua_State *L) {
    return push_Image(L, LoadImage(luaL_checkstring(L, 1)));
}

static int f_LoadImageRaw(lua_State *L) {
    return push_Image(L, LoadImageRaw(luaL_checkstring(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), (int)luaL_checknumber(L, 5)));
}

static int f_LoadImageAnim(lua_State *L) {
    int frames;
    Image image = LoadImageAnim(luaL_checkstring(L, 1), &frames);
    push_Image(L, image);
    lua_pushinteger(L, frames);
    return 2;
}

static int f_LoadImageFromString(lua_State *L) {
    size_t length;
    const char *type = luaL_checkstring(L, 1);
    const char *data = luaL_checklstring(L, 2, &length);
    return push_Image(L, LoadImageFromMemory(type, (const unsigned char*)data, (int)length));
}

static int f_LoadImageFromTexture(lua_State *L) {
    return push_Image(L, LoadImageFromTexture(*check_Texture(L, 1)));
}

static int f_LoadImageFromScreen(lua_State *L) {
    return push_Image(L, LoadImageFromScreen());
}

static int f_ExportImage(lua_State *L) {
    lua_pushboolean(L, ExportImage(*check_Image(L, 1), luaL_checkstring(L, 2)));
    return 1;
}


// Image generation functions --------------------------------------------------

static int f_GenImageColor(lua_State *L) {
    return push_Image(L, GenImageColor(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), *check_Color(L, 3)));
}

static int f_GenImageGradientV(lua_State *L) {
    return push_Image(L, GenImageGradientV(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), *check_Color(L, 3),  *check_Color(L, 4)));
}

static int f_GenImageGradientH(lua_State *L) {
    return push_Image(L, GenImageGradientH(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), *check_Color(L, 3),  *check_Color(L, 4)));
}

static int f_GenImageGradientRadial(lua_State *L) {
    return push_Image(L, GenImageGradientRadial(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), (float)luaL_checknumber(L, 3), *check_Color(L, 4), *check_Color(L, 5)));
}

static int f_GenImageChecked(lua_State *L) {
    return push_Image(L, GenImageChecked(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), *check_Color(L, 5), *check_Color(L, 6)));
}

static int f_GenImageWhiteNoise(lua_State *L) {
    return push_Image(L, GenImageWhiteNoise(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), (float)luaL_checknumber(L, 3)));
}

static int f_GenImageCellular(lua_State *L) {
    return push_Image(L, GenImageWhiteNoise(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2), luaL_checkinteger(L, 3)));
}


// Image manipulation functions ------------------------------------------------

static int f_ImageCopy(lua_State *L) {
    return push_Image(L, ImageCopy(*check_Image(L, 1)));
}

static int f_ImageFromImage(lua_State *L) {
    return push_Image(L, ImageFromImage(*check_Image(L, 1), *check_Rectangle(L, 2)));
}

static int f_ImageText(lua_State *L) {
    return push_Image(L, ImageText(luaL_checkstring(L, 1), (int)luaL_checknumber(L, 2), *check_Color(L, 3)));
}

static int f_ImageTextEx(lua_State *L) {
    return push_Image(L, ImageTextEx(*check_Font(L, 1), luaL_checkstring(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5)));
}

static int f_ImageFormat(lua_State *L) {
    ImageFormat(check_Image(L, 1), (int)luaL_checknumber(L, 2));
    return 0;
}

static int f_ImageToPOT(lua_State *L) {
    ImageToPOT(check_Image(L, 1), *check_Color(L, 2));
    return 0;
}

static int f_ImageCrop(lua_State *L) {
    ImageCrop(check_Image(L, 1), *check_Rectangle(L, 2));
    return 0;
}

static int f_ImageAlphaCrop(lua_State *L) {
    ImageAlphaCrop(check_Image(L, 1), (float)luaL_checknumber(L, 2));
    return 0;
}

static int f_ImageAlphaClear(lua_State *L) {
    ImageAlphaClear(check_Image(L, 1), *check_Color(L, 2), (float)luaL_checknumber(L, 3));
    return 0;
}

static int f_ImageAlphaMask(lua_State *L) {
    ImageAlphaMask(check_Image(L, 1), *check_Image(L, 2));
    return 0;
}

static int f_ImageAlphaPremultiply(lua_State *L) {
    ImageAlphaPremultiply(check_Image(L, 1));
    return 0;
}

static int f_ImageResize(lua_State *L) {
    ImageResize(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3));
    return 0;
}

static int f_ImageResizeNN(lua_State *L) {
    ImageResizeNN(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3));
    return 0;
}

static int f_ImageResizeCanvas(lua_State *L) {
    ImageResizeCanvas(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), (int)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_ImageMipmaps(lua_State *L) {
    ImageMipmaps(check_Image(L, 1));
    return 0;
}

static int f_ImageDither(lua_State *L) {
    ImageDither(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), (int)luaL_checknumber(L, 5));
    return 0;
}

static int f_ImageFlipVertical(lua_State *L) {
    ImageFlipVertical(check_Image(L, 1));
    return 0;
}

static int f_ImageFlipHorizontal(lua_State *L) {
    ImageFlipHorizontal(check_Image(L, 1));
    return 0;
}

static int f_ImageRotateCW(lua_State *L) {
    ImageRotateCW(check_Image(L, 1));
    return 0;
}

static int f_ImageRotateCCW(lua_State *L) {
    ImageRotateCCW(check_Image(L, 1));
    return 0;
}

static int f_ImageColorTint(lua_State *L) {
    ImageColorTint(check_Image(L, 1), *check_Color(L, 2));
    return 0;
}

static int f_ImageColorInvert(lua_State *L) {
    ImageColorInvert(check_Image(L, 1));
    return 0;
}

static int f_ImageColorGrayscale(lua_State *L) {
    ImageColorGrayscale(check_Image(L, 1));
    return 0;
}

static int f_ImageColorContrast(lua_State *L) {
    ImageColorContrast(check_Image(L, 1), (float)luaL_checknumber(L, 2));
    return 0;
}

static int f_ImageColorBrightness(lua_State *L) {
    ImageColorBrightness(check_Image(L, 1), (int)luaL_checknumber(L, 2));
    return 0;
}

static int f_ImageColorReplace(lua_State *L) {
    ImageColorReplace(check_Image(L, 1), *check_Color(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_LoadImageColors(lua_State *L) {
    const Image *image = check_Image(L, 1);
    Color *colors = LoadImageColors(*image);
    lua_createtable(L, image->width * image->height, 0);
    for (int i = 0; i < image->width * image->height; ++i) {
        push_Color(L, colors[i]);
        lua_rawseti(L, -2, i + 1);
    }
    UnloadImageColors(colors);
    return 1;
}

static int f_LoadImagePalette(lua_State *L) {
    int count;
    Color *colors = LoadImagePalette(*check_Image(L, 1), 256, &count);
    lua_createtable(L, count, 0);
    for (int i = 0; i < count; ++i) {
        push_Color(L, colors[i]);
        lua_rawseti(L, -2, i + 1);
    }
    UnloadImagePalette(colors);
    return 1;
}

static int f_GetImageAlphaBorder(lua_State *L) {
    return push_Rectangle(L, GetImageAlphaBorder(*check_Image(L, 1), (float)luaL_checknumber(L, 2)));
}

static int f_GetImageColor(lua_State *L) {
    return push_Color(L, GetImageColor(*check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3)));
}


// Image drawing functions -----------------------------------------------------

static int f_ImageClearBackground(lua_State *L) {
    ImageClearBackground(check_Image(L, 1), *check_Color(L, 2));
    return 0;
}

static int f_ImageDrawPixel(lua_State *L) {
    ImageDrawPixel(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_ImageDrawPixelV(lua_State *L) {
    ImageDrawPixelV(check_Image(L, 1), *check_Vector2(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_ImageDrawLine(lua_State *L) {
    ImageDrawLine(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), (int)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_ImageDrawLineV(lua_State *L) {
    ImageDrawLineV(check_Image(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_ImageDrawCircle(lua_State *L) {
    ImageDrawCircle(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_ImageDrawCircleV(lua_State *L) {
    ImageDrawCircleV(check_Image(L, 1), *check_Vector2(L, 2), (int)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_ImageDrawRectangle(lua_State *L) {
    ImageDrawRectangle(check_Image(L, 1), (int)luaL_checknumber(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), (int)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_ImageDrawRectangleV(lua_State *L) {
    ImageDrawRectangleV(check_Image(L, 1), *check_Vector2(L, 2), *check_Vector2(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_ImageDrawRectangleRec(lua_State *L) {
    ImageDrawRectangleRec(check_Image(L, 1), *check_Rectangle(L, 2), *check_Color(L, 3));
    return 0;
}

static int f_ImageDrawRectangleLines(lua_State *L) {
    ImageDrawRectangleLines(check_Image(L, 1), *check_Rectangle(L, 2), (int)luaL_checknumber(L, 3), *check_Color(L, 4));
    return 0;
}

static int f_ImageDraw(lua_State *L) {
    ImageDraw(check_Image(L, 1), *check_Image(L, 2), *check_Rectangle(L, 3), *check_Rectangle(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_ImageDrawText(lua_State *L) {
    ImageDrawText(check_Image(L, 1), luaL_checkstring(L, 2), (int)luaL_checknumber(L, 3), (int)luaL_checknumber(L, 4), (int)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_ImageDrawTextEx(lua_State *L) {
    ImageDrawTextEx(check_Image(L, 1), *check_Font(L, 2), luaL_checkstring(L, 3), *check_Vector2(L, 4), (float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), *check_Color(L, 7));
    return 0;
}


// Texture loading functions ---------------------------------------------------

static int f_LoadTexture(lua_State *L) {
    return push_Texture(L, LoadTexture(luaL_checkstring(L, 1)));
}


// Texture configuration functions ---------------------------------------------

static int f_GenTextureMipmaps(lua_State *L) {
    GenTextureMipmaps(check_Texture(L, 1));
    return 0;
}

static int f_SetTextureFilter(lua_State *L) {
    SetTextureFilter(*check_Texture(L, 1), luaL_checkinteger(L, 2));
    return 0;
}

static int f_SetTextureWrap(lua_State *L) {
    SetTextureWrap(*check_Texture(L, 1), luaL_checkinteger(L, 2));
    return 0;
}


//==[[ module: rtext ]]=========================================================

// Font loading/unloading functions --------------------------------------------

static int f_GetFontDefault(lua_State *L) {
    return push_Font(L, GetFontDefault());
}

static int f_LoadFont(lua_State *L) {
    return push_Font(L, LoadFont(luaL_checkstring(L, 1)));
}

static int f_LoadFontFromString(lua_State *L) {
    size_t length;
    const char *type = luaL_checkstring(L, 1);
    const char *data = luaL_checklstring(L, 2, &length);
    int fontSize = luaL_optinteger(L, 3, 20);
    return push_Font(L, LoadFontFromMemory(type, (unsigned char*)data, (int)length, fontSize, NULL, 0));
}


// Text drawing functions ------------------------------------------------------

static int f_DrawFPS(lua_State *L) {
    DrawFPS(luaL_checknumber(L, 1), luaL_checknumber(L, 2));
    return 0;
}

static int f_DrawText(lua_State *L) {
    DrawText(luaL_checkstring(L, 1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

static int f_DrawTextEx(lua_State *L) {
    DrawTextEx(*check_Font(L, 1), luaL_checkstring(L, 2), *check_Vector2(L, 3), (float)luaL_checknumber(L, 4), (float)luaL_checknumber(L, 5), *check_Color(L, 6));
    return 0;
}

static int f_DrawTextPro(lua_State *L) {
    DrawTextPro(*check_Font(L, 1), luaL_checkstring(L, 2), *check_Vector2(L, 3), *check_Vector2(L, 4), (float)luaL_checknumber(L, 5), (float)luaL_checknumber(L, 6), (float)luaL_checknumber(L, 7), *check_Color(L, 8));
    return 0;
}

static int f_DrawTextCodepoint(lua_State *L) {
    DrawTextCodepoint(*check_Font(L, 1), luaL_checkinteger(L, 2), *check_Vector2(L, 3), (float)luaL_checknumber(L, 4), *check_Color(L, 5));
    return 0;
}

// Text font info functions ----------------------------------------------------

static int f_MeasureText(lua_State *L) {
    lua_pushinteger(L, MeasureText(luaL_checkstring(L, 1), luaL_checknumber(L, 2)));
    return 0;
}

static int f_MeasureTextEx(lua_State *L) {
    return push_Vector2(L, MeasureTextEx(*check_Font(L, 1), luaL_checkstring(L, 2), (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4)));
}


//==[[ module: rmodels ]]=======================================================


//==[[ module: raudio ]]========================================================


//==[[ Lua module definition ]]=================================================

static const luaL_Reg Vector2_meta[] = {
    { "__tostring", f_Vector2__tostring },
    { "__index", f_Vector2__index },
    { "__newindex", f_Vector2__newindex },
    { "__add", f_Vector2__add },
    { "__sub", f_Vector2__sub },
    { "__mul", f_Vector2__mul },
    { "__div", f_Vector2__div },
    { "__unm", f_Vector2__unm },
    { "Length", f_Vector2_Length },
    { "Distance", f_Vector2_Distance },
    { "Normal", f_Vector2_Normal },
    { "Angle", f_Vector2_Angle },
    { "?x", f_Vector2_get_x },
    { "=x", f_Vector2_set_x },
    { "?y", f_Vector2_get_y },
    { "=y", f_Vector2_set_y },
    { NULL, NULL }
};

static const luaL_Reg Vector3_meta[] = {
    { "__tostring", f_Vector3__tostring },
    { "__index", f_Vector3__index },
    { "__newindex", f_Vector3__newindex },
    { "__add", f_Vector3__add },
    { "__sub", f_Vector3__sub },
    { "__mul", f_Vector3__mul },
    { "__div", f_Vector3__div },
    { "__unm", f_Vector3__unm },
    { "__eq", f_Vector3__eq },
    { "Length", f_Vector3_Length },
    { "Distance", f_Vector3_Distance },
    { "Normal", f_Vector3_Normal },
    { "?x", f_Vector3_get_x },
    { "=x", f_Vector3_set_x },
    { "?y", f_Vector3_get_y },
    { "=y", f_Vector3_set_y },
    { "?z", f_Vector3_get_z },
    { "=z", f_Vector3_set_z },
    { NULL, NULL }
};

static const luaL_Reg Color_meta[] = {
    { "__tostring", f_Color__tostring },
    { "__index", f_Color__index },
    { "__newindex", f_Color__newindex },
    { "Fade", f_Color_Fade },
    { "?r", f_Color_get_r },
    { "=r", f_Color_set_r },
    { "?g", f_Color_get_g },
    { "=g", f_Color_set_g },
    { "?b", f_Color_get_b },
    { "=b", f_Color_set_b },
    { "?a", f_Color_get_a },
    { "=a", f_Color_set_a },
    { NULL, NULL }
};

static const luaL_Reg Rectangle_meta[] = {
    { "__tostring", f_Rectangle__tostring },
    { "__index", f_Rectangle__index },
    { "__newindex", f_Rectangle__newindex },
    { "?x", f_Rectangle_get_x },
    { "=x", f_Rectangle_set_x },
    { "?y", f_Rectangle_get_y },
    { "=y", f_Rectangle_set_y },
    { "?width", f_Rectangle_get_width },
    { "=width", f_Rectangle_set_width },
    { "?height", f_Rectangle_get_height },
    { "=height", f_Rectangle_set_height },
    { NULL, NULL }
};

static const luaL_Reg Image_meta[] = {
    { "__tostring", f_Image__tostring },
    { "__gc", f_Image__gc },
    { "__index", f_Image__index },

    { "Export", f_ExportImage },

    { "Copy", f_ImageCopy },
    { "FromImage", f_ImageFromImage },
    { "Text", f_ImageText },
    { "TextEx", f_ImageTextEx },
    { "Format", f_ImageFormat },
    { "ToPOT", f_ImageToPOT },
    { "Crop", f_ImageCrop },
    { "AlphaCrop", f_ImageAlphaCrop },
    { "AlphaClear", f_ImageAlphaClear },
    { "AlphaMask", f_ImageAlphaMask },
    { "AlphaPremultiply", f_ImageAlphaPremultiply },
    { "Resize", f_ImageResize },
    { "ResizeNN", f_ImageResizeNN },
    { "ResizeCanvas", f_ImageResizeCanvas },
    { "Mipmaps", f_ImageMipmaps },
    { "Dither", f_ImageDither },
    { "FlipVertical", f_ImageFlipVertical },
    { "FlipHorizontal", f_ImageFlipHorizontal },
    { "RotateCW", f_ImageRotateCW },
    { "RotateCCW", f_ImageRotateCCW },
    { "ColorTint", f_ImageColorTint },
    { "ColorInvert", f_ImageColorInvert },
    { "ColorGrayscale", f_ImageColorGrayscale },
    { "ColorContrast", f_ImageColorContrast },
    { "ColorBrightness", f_ImageColorBrightness },
    { "ColorReplace", f_ImageColorReplace },
    { "LoadColors", f_LoadImageColors },
    { "LoadPalette", f_LoadImagePalette },
    { "GetAlphaBorder", f_GetImageAlphaBorder },
    { "GetColor", f_GetImageColor },

    { "ClearBackground", f_ImageClearBackground },
    { "DrawPixel", f_ImageDrawPixel },
    { "DrawPixelV", f_ImageDrawPixelV },
    { "DrawLine", f_ImageDrawLine },
    { "DrawLineV", f_ImageDrawLineV },
    { "DrawCircle", f_ImageDrawCircle },
    { "DrawCircleV", f_ImageDrawCircleV },
    { "DrawRectangle", f_ImageDrawRectangle },
    { "DrawRectangleV", f_ImageDrawRectangleV },
    { "DrawRectangleRec", f_ImageDrawRectangleRec },
    { "DrawRectangleLines", f_ImageDrawRectangleLines },
    { "Draw", f_ImageDraw },
    { "DrawText", f_ImageDrawText },
    { "DrawTextEx", f_ImageDrawTextEx },

    { NULL, NULL }
};

static const luaL_Reg Texture_meta[] = {
    { "__tostring", f_Texture__tostring },
    { "__gc", f_Texture__gc },
    { "__index", f_Texture__index },
    { "?width", f_Texture_get_width },
    { "?height", f_Texture_get_height },
    { NULL, NULL }
};

static const luaL_Reg Font_meta[] = {
    { "__tostring", f_Font__tostring },
    { "__gc", f_Font__gc },
    { NULL, NULL }
};

static const luaL_Reg raylib_funcs[] = {
    // Object creation ---------------------------------------------------------
    { "Vector2", f_Vector2 },
    { "Vector3", f_Vector3 },
    { "Color", f_Color },
    { "Rectangle", f_Rectangle },
    // module: core ------------------------------------------------------------
        // Window-related functions
        { "InitWindow", f_InitWindow },
        { "WindowShouldClose", f_WindowShouldClose },
        { "CloseWindow", f_CloseWindow },
        { "IsWindowReady", f_IsWindowReady },
        { "IsWindowFullscreen", f_IsWindowFullscreen },
        { "IsWindowHidden", f_IsWindowHidden },
        { "IsWindowMaximized", f_IsWindowMaximized },
        { "IsWindowMinimized", f_IsWindowMinimized },
        { "IsWindowFocused", f_IsWindowFocused },
        { "IsWindowResized", f_IsWindowResized },
        { "IsWindowState", f_IsWindowState },
        { "SetWindowState", f_SetWindowState },
        { "ClearWindowState", f_ClearWindowState },
        { "ToggleFullscreen", f_ToggleFullscreen },
        { "MaximizeWindow", f_MaximizeWindow },
        { "MinimizeWindow", f_MinimizeWindow },
        { "RestoreWindow", f_RestoreWindow },
        { "SetWindowIcon", f_SetWindowIcon },
        { "SetWindowTitle", f_SetWindowTitle },
        { "SetWindowPosition", f_SetWindowPosition },
        { "SetWindowMinSize", f_SetWindowMinSize },
        { "SetWindowSize", f_SetWindowSize },
        { "SetWindowOpacity", f_SetWindowOpacity },
        { "GetScreenSize", f_GetScreenSize },
        { "GetRenderSize", f_GetRenderSize },
        { "SetClipboardText", f_SetClipboardText },
        { "GetClipboardText", f_GetClipboardText },
        // Custom frame control functions
        { "SwapScreenBuffer", f_SwapScreenBuffer },
        { "PollInputEvents", f_PollInputEvents },
        { "WaitTime", f_WaitTime },
        // Cursor-related functions
        { "ShowCursor", f_ShowCursor },
        { "HideCursor", f_HideCursor },
        { "IsCursorHidden", f_IsCursorHidden },
        { "EnableCursor", f_EnableCursor },
        { "DisableCursor", f_DisableCursor },
        { "IsCursorOnScreen", f_IsCursorOnScreen },
        // Drawing-related functions
        { "ClearBackground", f_ClearBackground },
        { "BeginDrawing", f_BeginDrawing },
        { "EndDrawing", f_EndDrawing },
        { "BeginBlendMode", f_BeginBlendMode },
        { "EndBlendMode", f_EndBlendMode },
        { "BeginScissorMode", f_BeginScissorMode },
        { "EndScissorMode", f_EndScissorMode },
        // Timing-related functions
        { "SetTargetFPS", f_SetTargetFPS },
        { "GetFPS", f_GetFPS },
        { "GetFrameTime", f_GetFrameTime },
        { "GetTime", f_GetTime },
        // Files management functions ------------------------------------------
        { "LoadFileData", f_LoadFileData },
        { "SaveFileData", f_SaveFileData },
        { "FileExists", f_FileExists },
        { "DirectoryExists", f_DirectoryExists },
        { "IsFileExtension", f_IsFileExtension },
        { "GetFileLength", f_GetFileLength },
        { "GetFileExtension", f_GetFileExtension },
        { "GetFileName", f_GetFileName },
        { "GetFileNameWithoutExt", f_GetFileNameWithoutExt },
        { "GetDirectoryPath", f_GetDirectoryPath },
        { "GetPrevDirectoryPath", f_GetPrevDirectoryPath },
        { "GetWorkingDirectory", f_GetWorkingDirectory },
        { "GetApplicationDirectory", f_GetApplicationDirectory },
        { "ChangeDirectory", f_ChangeDirectory },
        { "IsPathFile", f_IsPathFile },
        { "LoadDirectoryFiles", f_LoadDirectoryFiles },
        { "LoadDirectoryFilesEx", f_LoadDirectoryFilesEx },
        { "IsFileDropped", f_IsFileDropped },
        { "LoadDroppedFiles", f_LoadDroppedFiles },
        { "GetFileModTile", f_GetFileModTime },
        // Compression/Encoding functionality ----------------------------------
        { "CompressData", f_CompressData },
        { "DecompressData", f_DecompressData },
        { "EncodeDataBase64", f_EncodeDataBase64 },
        { "DecodeDataBase64", f_DecodeDataBase64 },
        // Input-related functions: keyboard -----------------------------------
        { "IsKeyPressed", f_IsKeyPressed },
        { "IsKeyDown", f_IsKeyDown },
        { "IsKeyReleased", f_IsKeyReleased },
        { "IsKeyUp", f_IsKeyUp },
        { "SetExitKey", f_SetExitKey },
        { "GetKeyPressed", f_GetKeyPressed },
        { "GetCharPressed", f_GetCharPressed },
        // Input-related functions: gamepads -----------------------------------
        { "IsGamepadAvailable", f_IsGamepadAvailable },
        { "GetGamepadName", f_GetGamepadName },
        { "IsGamepadButtonPressed", f_IsGamepadButtonPressed },
        { "IsGamepadButtonDown", f_IsGamepadButtonDown },
        { "IsGamepadButtonReleased", f_IsGamepadButtonReleased },
        { "IsGamepadButtonUp", f_IsGamepadButtonUp },
        { "GetGamepadButtonPressed", f_GetGamepadButtonPressed },
        { "GetGamepadAxisCount", f_GetGamepadAxisCount },
        { "GetGamepadAxisMovement", f_GetGamepadAxisMovement },
        // // Input-related functions: mouse -----------------------------------
        { "IsMouseButtonPressed", f_IsMouseButtonPressed },
        { "IsMouseButtonDown", f_IsMouseButtonDown },
        { "IsMouseButtonReleased", f_IsMouseButtonReleased },
        { "IsMouseButtonUp", f_IsMouseButtonUp },
        { "GetMousePosition", f_GetMousePosition },
        { "GetMouseDelta", f_GetMouseDelta },
        { "SetMousePosition", f_SetMousePosition },
        { "SetMouseOffset", f_SetMouseOffset },
        { "SetMouseScale", f_SetMouseScale },
        { "GetMouseWheelMove", f_GetMouseWheelMove },
        { "SetMouseCursor", f_SetMouseCursor },
    // module: rshapes ---------------------------------------------------------
        { "SetShapesTexture", f_SetShapesTexture },
        // Basic Shapes Drawing Functions --------------------------------------
        { "DrawPixel", f_DrawPixel },
        { "DrawPixelV", f_DrawPixelV },
        { "DrawLine", f_DrawLine },
        { "DrawLineV", f_DrawLineV },
        { "DrawLineEx", f_DrawLineEx },
        { "DrawLineBezier", f_DrawLineBezier },
        { "DrawLineBezierQuad", f_DrawLineBezierQuad },
        { "DrawLineBezierCubic", f_DrawLineBezierCubic },
        { "DrawLineStrip", f_DrawLineStrip },
        { "DrawCircle", f_DrawCircle },
        { "DrawCircleSector", f_DrawCircleSector },
        { "DrawCircleSectorLines", f_DrawCircleSectorLines },
        { "DrawCircleGradient", f_DrawCircleGradient },
        { "DrawCircleV", f_DrawCircleV },
        { "DrawCircleLines", f_DrawCircleLines },
        { "DrawEllipse", f_DrawEllipse },
        { "DrawEllipseLines", f_DrawEllipseLines },
        { "DrawRing", f_DrawRing },
        { "DrawRingLines", f_DrawRingLines },
        { "DrawRectangle", f_DrawRectangle },
        { "DrawRectangleV", f_DrawRectangleV },
        { "DrawRectangleRec", f_DrawRectangleRec },
        { "DrawRectanglePro", f_DrawRectanglePro },
        { "DrawRectangleGradientV", f_DrawRectangleGradientV },
        { "DrawRectangleGradientH", f_DrawRectangleGradientH },
        { "DrawRectangleGradientEx", f_DrawRectangleGradientEx },
        { "DrawRectangleLines", f_DrawRectangleLines },
        { "DrawRectangleLinesEx", f_DrawRectangleLinesEx },
        { "DrawRectangleRounded", f_DrawRectangleRounded },
        { "DrawRectangleRoundedLines", f_DrawRectangleRoundedLines },
        { "DrawTriangle", f_DrawTriangle },
        { "DrawTriangleLines", f_DrawTriangleLines },
        { "DrawTriangleFan", f_DrawTriangleFan },
        { "DrawTriangleStrip", f_DrawTriangleStrip },
        { "DrawPoly", f_DrawPoly },
        { "DrawPolyLines", f_DrawPolyLines },
        { "DrawPolyLinesEx", f_DrawPolyLinesEx },
        // Basic shapes collision detection functions --------------------------
        { "CheckCollisionRecs", f_CheckCollisionRecs },
        { "CheckCollisionCircles", f_CheckCollisionCircles },
        { "CheckCollisionCircleRec", f_CheckCollisionCircleRec },
        { "CheckCollisionPointRec", f_CheckCollisionPointRec },
        { "CheckCollisionPointCircle", f_CheckCollisionPointCircle },
        { "CheckCollisionPointTriangle", f_CheckCollisionPointTriangle },
        { "CheckCollisionLines", f_CheckCollisionLines },
        { "CheckCollisionPointLine", f_CheckCollisionPointLine },
        { "GetCollisionRec", f_GetCollisionRec },
    // module: rtextures -------------------------------------------------------
        // Image loading functions ---------------------------------------------
        { "LoadImage", f_LoadImage },
        { "LoadImageRaw", f_LoadImageRaw },
        { "LoadImageAnim", f_LoadImageAnim },
        { "LoadImageFromString", f_LoadImageFromString },
        { "LoadImageFromTexture", f_LoadImageFromTexture },
        { "LoadImageFromScreen", f_LoadImageFromScreen },
        { "ExportImage", f_ExportImage },
        // Image generation functions ------------------------------------------
        { "GenImageColor", f_GenImageColor },
        { "GenImageGradientV", f_GenImageGradientV },
        { "GenImageGradientH", f_GenImageGradientH },
        { "GenImageGradientRadial", f_GenImageGradientRadial },
        { "GenImageChecked", f_GenImageChecked },
        { "GenImageWhiteNoise", f_GenImageWhiteNoise },
        { "GenImageCellular", f_GenImageCellular },
        // Image manipulation functions ----------------------------------------
        { "ImageCopy", f_ImageCopy },
        { "ImageFromImage", f_ImageFromImage },
        { "ImageText", f_ImageText },
        { "ImageTextEx", f_ImageTextEx },
        { "ImageFormat", f_ImageFormat },
        { "ImageToPOT", f_ImageToPOT },
        { "ImageCrop", f_ImageCrop },
        { "ImageAlphaCrop", f_ImageAlphaCrop },
        { "ImageAlphaClear", f_ImageAlphaClear },
        { "ImageAlphaMask", f_ImageAlphaMask },
        { "ImageAlphaPremultiply", f_ImageAlphaPremultiply },
        { "ImageResize", f_ImageResize },
        { "ImageResizeNN", f_ImageResizeNN },
        { "ImageResizeCanvas", f_ImageResizeCanvas },
        { "ImageMipmaps", f_ImageMipmaps },
        { "ImageDither", f_ImageDither },
        { "ImageFlipVertical", f_ImageFlipVertical },
        { "ImageFlipHorizontal", f_ImageFlipHorizontal },
        { "ImageRotateCW", f_ImageRotateCW },
        { "ImageRotateCCW", f_ImageRotateCCW },
        { "ImageColorTint", f_ImageColorTint },
        { "ImageColorInvert", f_ImageColorInvert },
        { "ImageColorGrayscale", f_ImageColorGrayscale },
        { "ImageColorContrast", f_ImageColorContrast },
        { "ImageColorBrightness", f_ImageColorBrightness },
        { "ImageColorReplace", f_ImageColorReplace },
        { "LoadImageColors", f_LoadImageColors },
        { "LoadImagePalette", f_LoadImagePalette },
        { "GetImageAlphaBorder", f_GetImageAlphaBorder },
        { "GetImageColor", f_GetImageColor },
        // Image drawing functions ---------------------------------------------
        { "ImageClearBackground", f_ImageClearBackground },
        { "ImageDrawPixel", f_ImageDrawPixel },
        { "ImageDrawPixelV", f_ImageDrawPixelV },
        { "ImageDrawLine", f_ImageDrawLine },
        { "ImageDrawLineV", f_ImageDrawLineV },
        { "ImageDrawCircle", f_ImageDrawCircle },
        { "ImageDrawCircleV", f_ImageDrawCircleV },
        { "ImageDrawRectangle", f_ImageDrawRectangle },
        { "ImageDrawRectangleV", f_ImageDrawRectangleV },
        { "ImageDrawRectangleRec", f_ImageDrawRectangleRec },
        { "ImageDrawRectangleLines", f_ImageDrawRectangleLines },
        { "ImageDraw", f_ImageDraw },
        { "ImageDrawText", f_ImageDrawText },
        { "ImageDrawTextEx", f_ImageDrawTextEx },
        // Texture loading functions -------------------------------------------
        { "LoadTexture", f_LoadTexture },
        // Texture configuration functions -------------------------------------
        { "GenTextureMipmaps", f_GenTextureMipmaps },
        { "SetTextureFilter", f_SetTextureFilter },
        { "SetTextureWrap", f_SetTextureWrap },
    // module: rtext -----------------------------------------------------------
        // Font loading/unloading functions ------------------------------------
        { "GetFontDefault", f_GetFontDefault },
        { "LoadFont", f_LoadFont },
        { "LoadFontFromString", f_LoadFontFromString },
        // Text drawing functions ----------------------------------------------
        { "DrawFPS", f_DrawFPS },
        { "DrawText", f_DrawText },
        { "DrawTextEx", f_DrawTextEx },
        { "DrawTextPro", f_DrawTextPro },
        { "DrawTextCodepoint", f_DrawTextCodepoint },
        // Text font info functions --------------------------------------------
        { "MeasureText", f_MeasureText },
        { "MeasureTextEx", f_MeasureTextEx },
    // sentinel ----------------------------------------------------------------
    { NULL, NULL }
};

static const struct {
    const char *name;
    const int value;
} raylib_values[] = {
    // Keyboard keys -----------------------------------------------------------
        { "KEY_NULL", KEY_NULL },
        // Alphanumeric keys
        { "KEY_APOSTROPHE", KEY_APOSTROPHE },
        { "KEY_COMMA", KEY_COMMA },
        { "KEY_MINUS", KEY_MINUS },
        { "KEY_PERIOD", KEY_PERIOD },
        { "KEY_SLASH", KEY_SLASH },
        { "KEY_ZERO", KEY_ZERO },
        { "KEY_ONE", KEY_ONE },
        { "KEY_TWO", KEY_TWO },
        { "KEY_THREE", KEY_THREE },
        { "KEY_FOUR", KEY_FOUR },
        { "KEY_FIVE", KEY_FIVE },
        { "KEY_SIX", KEY_SIX },
        { "KEY_SEVEN", KEY_SEVEN },
        { "KEY_EIGHT", KEY_EIGHT },
        { "KEY_NINE", KEY_NINE },
        { "KEY_SEMICOLON", KEY_SEMICOLON },
        { "KEY_EQUAL", KEY_EQUAL },
        { "KEY_A", KEY_A },
        { "KEY_B", KEY_B },
        { "KEY_C", KEY_C },
        { "KEY_D", KEY_D },
        { "KEY_E", KEY_E },
        { "KEY_F", KEY_F },
        { "KEY_G", KEY_G },
        { "KEY_H", KEY_H },
        { "KEY_I", KEY_I },
        { "KEY_J", KEY_J },
        { "KEY_K", KEY_K },
        { "KEY_L", KEY_L },
        { "KEY_M", KEY_M },
        { "KEY_N", KEY_N },
        { "KEY_O", KEY_O },
        { "KEY_P", KEY_P },
        { "KEY_Q", KEY_Q },
        { "KEY_R", KEY_R },
        { "KEY_S", KEY_S },
        { "KEY_T", KEY_T },
        { "KEY_U", KEY_U },
        { "KEY_V", KEY_V },
        { "KEY_W", KEY_W },
        { "KEY_X", KEY_X },
        { "KEY_Y", KEY_Y },
        { "KEY_Z", KEY_Z },
        { "KEY_LEFT_BRACKET", KEY_LEFT_BRACKET },
        { "KEY_BACKSLASH", KEY_BACKSLASH },
        { "KEY_RIGHT_BRACKET", KEY_RIGHT_BRACKET },
        { "KEY_GRAVE", KEY_GRAVE },
        // Function keys
        { "KEY_SPACE", KEY_SPACE },
        { "KEY_ESCAPE", KEY_ESCAPE },
        { "KEY_ENTER", KEY_ENTER },
        { "KEY_TAB", KEY_TAB },
        { "KEY_BACKSPACE", KEY_BACKSPACE },
        { "KEY_INSERT", KEY_INSERT },
        { "KEY_DELETE", KEY_DELETE },
        { "KEY_RIGHT", KEY_RIGHT },
        { "KEY_LEFT", KEY_LEFT },
        { "KEY_DOWN", KEY_DOWN },
        { "KEY_UP", KEY_UP },
        { "KEY_PAGE_UP", KEY_PAGE_UP },
        { "KEY_PAGE_DOWN", KEY_PAGE_DOWN },
        { "KEY_HOME", KEY_HOME },
        { "KEY_END", KEY_END },
        { "KEY_CAPS_LOCK", KEY_CAPS_LOCK },
        { "KEY_SCROLL_LOCK", KEY_SCROLL_LOCK },
        { "KEY_NUM_LOCK", KEY_NUM_LOCK },
        { "KEY_PRINT_SCREEN", KEY_PRINT_SCREEN },
        { "KEY_PAUSE", KEY_PAUSE },
        { "KEY_F1", KEY_F1 },
        { "KEY_F2", KEY_F2 },
        { "KEY_F3", KEY_F3 },
        { "KEY_F4", KEY_F4 },
        { "KEY_F5", KEY_F5 },
        { "KEY_F6", KEY_F6 },
        { "KEY_F7", KEY_F7 },
        { "KEY_F8", KEY_F8 },
        { "KEY_F9", KEY_F9 },
        { "KEY_F10", KEY_F10 },
        { "KEY_F11", KEY_F11 },
        { "KEY_F12", KEY_F12 },
        { "KEY_LEFT_SHIFT", KEY_LEFT_SHIFT },
        { "KEY_LEFT_CONTROL", KEY_LEFT_CONTROL },
        { "KEY_LEFT_ALT", KEY_LEFT_ALT },
        { "KEY_LEFT_SUPER", KEY_LEFT_SUPER },
        { "KEY_RIGHT_SHIFT", KEY_RIGHT_SHIFT },
        { "KEY_RIGHT_CONTROL", KEY_RIGHT_CONTROL },
        { "KEY_RIGHT_ALT", KEY_RIGHT_ALT },
        { "KEY_RIGHT_SUPER", KEY_RIGHT_SUPER },
        { "KEY_KB_MENU", KEY_KB_MENU },
        // Keypad keys
        { "KEY_KP_0", KEY_KP_0 },
        { "KEY_KP_1", KEY_KP_1 },
        { "KEY_KP_2", KEY_KP_2 },
        { "KEY_KP_3", KEY_KP_3 },
        { "KEY_KP_4", KEY_KP_4 },
        { "KEY_KP_5", KEY_KP_5 },
        { "KEY_KP_6", KEY_KP_6 },
        { "KEY_KP_7", KEY_KP_7 },
        { "KEY_KP_8", KEY_KP_8 },
        { "KEY_KP_9", KEY_KP_9 },
        { "KEY_KP_DECIMAL", KEY_KP_DECIMAL },
        { "KEY_KP_DIVIDE", KEY_KP_DIVIDE },
        { "KEY_KP_MULTIPLY", KEY_KP_MULTIPLY },
        { "KEY_KP_SUBTRACT", KEY_KP_SUBTRACT },
        { "KEY_KP_ADD", KEY_KP_ADD },
        { "KEY_KP_ENTER", KEY_KP_ENTER },
        { "KEY_KP_EQUAL", KEY_KP_EQUAL },
        // Android key buttons
        { "KEY_BACK", KEY_BACK },
        { "KEY_MENU", KEY_MENU },
        { "KEY_VOLUME_UP", KEY_VOLUME_UP },
        { "KEY_VOLUME_DOWN", KEY_VOLUME_DOWN },
    // Mouse buttons -----------------------------------------------------------
        { "MOUSE_BUTTON_LEFT", MOUSE_BUTTON_LEFT },
        { "MOUSE_BUTTON_RIGHT", MOUSE_BUTTON_RIGHT },
        { "MOUSE_BUTTON_MIDDLE", MOUSE_BUTTON_MIDDLE },
        { "MOUSE_BUTTON_SIDE", MOUSE_BUTTON_SIDE },
        { "MOUSE_BUTTON_EXTRA", MOUSE_BUTTON_EXTRA },
        { "MOUSE_BUTTON_FORWARD", MOUSE_BUTTON_FORWARD },
        { "MOUSE_BUTTON_BACK", MOUSE_BUTTON_BACK },
    // Gamepad buttons ---------------------------------------------------------
        { "GAMEPAD_BUTTON_UNKNOWN", GAMEPAD_BUTTON_UNKNOWN },
        { "GAMEPAD_BUTTON_LEFT_FACE_UP", GAMEPAD_BUTTON_LEFT_FACE_UP },
        { "GAMEPAD_BUTTON_LEFT_FACE_RIGHT", GAMEPAD_BUTTON_LEFT_FACE_RIGHT },
        { "GAMEPAD_BUTTON_LEFT_FACE_DOWN", GAMEPAD_BUTTON_LEFT_FACE_DOWN },
        { "GAMEPAD_BUTTON_LEFT_FACE_LEFT", GAMEPAD_BUTTON_LEFT_FACE_LEFT },
        { "GAMEPAD_BUTTON_RIGHT_FACE_UP", GAMEPAD_BUTTON_RIGHT_FACE_UP },
        { "GAMEPAD_BUTTON_RIGHT_FACE_RIGHT", GAMEPAD_BUTTON_RIGHT_FACE_RIGHT },
        { "GAMEPAD_BUTTON_RIGHT_FACE_DOWN", GAMEPAD_BUTTON_RIGHT_FACE_DOWN },
        { "GAMEPAD_BUTTON_RIGHT_FACE_LEFT", GAMEPAD_BUTTON_RIGHT_FACE_LEFT },
        { "GAMEPAD_BUTTON_LEFT_TRIGGER_1", GAMEPAD_BUTTON_LEFT_TRIGGER_1 },
        { "GAMEPAD_BUTTON_LEFT_TRIGGER_2", GAMEPAD_BUTTON_LEFT_TRIGGER_2 },
        { "GAMEPAD_BUTTON_RIGHT_TRIGGER_1", GAMEPAD_BUTTON_RIGHT_TRIGGER_1 },
        { "GAMEPAD_BUTTON_RIGHT_TRIGGER_2", GAMEPAD_BUTTON_RIGHT_TRIGGER_2 },
        { "GAMEPAD_BUTTON_MIDDLE_LEFT", GAMEPAD_BUTTON_MIDDLE_LEFT },
        { "GAMEPAD_BUTTON_MIDDLE", GAMEPAD_BUTTON_MIDDLE },
        { "GAMEPAD_BUTTON_MIDDLE_RIGHT", GAMEPAD_BUTTON_MIDDLE_RIGHT },
        { "GAMEPAD_BUTTON_LEFT_THUMB", GAMEPAD_BUTTON_LEFT_THUMB },
        { "GAMEPAD_BUTTON_RIGHT_THUMB", GAMEPAD_BUTTON_RIGHT_THUMB },
    // Gamepad axis ------------------------------------------------------------
        { "GAMEPAD_AXIS_LEFT_X", GAMEPAD_AXIS_LEFT_X },
        { "GAMEPAD_AXIS_LEFT_Y", GAMEPAD_AXIS_LEFT_Y },
        { "GAMEPAD_AXIS_RIGHT_X", GAMEPAD_AXIS_RIGHT_X },
        { "GAMEPAD_AXIS_RIGHT_Y", GAMEPAD_AXIS_RIGHT_Y },
        { "GAMEPAD_AXIS_LEFT_TRIGGER", GAMEPAD_AXIS_LEFT_TRIGGER },
        { "GAMEPAD_AXIS_RIGHT_TRIGGER", GAMEPAD_AXIS_RIGHT_TRIGGER },
    // sentinel ----------------------------------------------------------------
    { NULL, 0 }
};

static const struct {
    const char *name;
    const Color color;
} raylib_colors[] = {
    { "LIGHTGRAY", LIGHTGRAY },
    { "GRAY", GRAY },
    { "DARKGRAY", DARKGRAY },
    { "YELLOW", YELLOW },
    { "GOLD", GOLD },
    { "ORANGE", ORANGE },
    { "PINK", PINK },
    { "RED", RED },
    { "MAROON", MAROON },
    { "GREEN", GREEN },
    { "LIME", LIME },
    { "DARKGREEN", DARKGREEN },
    { "SKYBLUE", SKYBLUE },
    { "BLUE", BLUE },
    { "DARKBLUE", DARKBLUE },
    { "PURPLE", PURPLE },
    { "VIOLET", VIOLET },
    { "DARKPURPLE", DARKPURPLE },
    { "BEIGE", BEIGE },
    { "BROWN", BROWN },
    { "DARKBROWN", DARKBROWN },
    { "WHITE", WHITE },
    { "BLACK", BLACK },
    { "BLANK", BLANK },
    { "MAGENTA", MAGENTA },
    { "RAYWHITE", RAYWHITE },
    { NULL, {} }
};

static void InitRayLua(lua_State *L) {
    // push object metatables
    push_meta(L, "Vector2", Vector2_meta);
    push_meta(L, "Vector3", Vector3_meta);
    push_meta(L, "Color", Color_meta);
    push_meta(L, "Rectangle", Rectangle_meta);
    push_meta(L, "Image", Image_meta);
    push_meta(L, "Texture", Texture_meta);
    push_meta(L, "Font", Font_meta);
    // register our functions
    lua_pushglobaltable(L);
    luaL_setfuncs(L, raylib_funcs, 0);
    // register values
    for (int i = 0; raylib_values[i].name != NULL; ++i) {
        lua_pushinteger(L, raylib_values[i].value);
        lua_setfield(L, -2, raylib_values[i].name);
    }
    // register colors
    for (int i = 0; raylib_colors[i].name != NULL; ++i) {
        push_Color(L, raylib_colors[i].color);
        lua_setfield(L, -2, raylib_colors[i].name);
    }
    lua_pop(L, 1);
}


//==[[ main ]]==================================================================

static int RunLuaCode(lua_State *L) {
    if (luaL_loadfile(L, "init.lua") != LUA_OK)
        lua_error(L);
    lua_call(L, 0, 0);
    return 0;
}

int main(void) {
    // create new Lua state
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    InitRayLua(L);
    // push 'debug.traceback' as error handler
    lua_getglobal(L, "debug");
    lua_getfield(L, -1, "traceback");
    lua_remove(L, -2);
    // run the Lua code in a protected environment to catch errors
    lua_pushcfunction(L, RunLuaCode);
    if (lua_pcall(L, 0, 0, -2) != LUA_OK)
        fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_close(L);
    return 0;
}
