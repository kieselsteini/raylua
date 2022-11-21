--[[----------------------------------------------------------------------------
--]]----------------------------------------------------------------------------
local templates = {
    body = 'static int f_%s(lua_State *L) {\n%s\n}\n\n',
    args = {
        ['void'] = '',
        ['int'] = 'luaL_checkinteger(L, %d)',
        ['float'] = '(float)luaL_checknumber(L, %d)',
        ['const char*'] = 'luaL_checkstring(L, %d)',
        ['bool'] = 'lua_toboolean(L, %d)',
        ['Vector2'] = '*check_Vector2(L, %d)',
        ['Vector3'] = '*check_Vector3(L, %d)',
        ['Rectangle'] = '*check_Rectangle(L, %d)',
        ['Color'] = '*check_Color(L, %d)',
        ['Image'] = '*check_Image(L, %d)',
        ['Image*'] = 'check_Image(L, %d)',
        ['Font'] = '*check_Font(L, %d)',
        ['Texture'] = '*check_Texture(L, %d)',
        ['Texture2D'] = '*check_Texture(L, %d)',
        ['TextureCubemap'] = '*check_Texture(L, %d)',
        ['Camera2D'] = '*check_Camera2D(L, %d)',
        ['Camera3D'] = '*check_Camera3D(L, %d)',
        ['Wave'] = '*check_Wave(L, %d)',
        ['Sound'] = '*check_Sound(L, %d)',
        ['Music'] = '*check_Music(L, %d)',
    },
    returns = {
        ['void'] = '    %s;\n    return 0;',
        ['bool'] = '    lua_pushboolean(L, %s);\n    return 1;',
        ['float'] = '    lua_pushnumber(L, %s);\n     return 1;',
        ['int'] = '    lua_pushinteger(L, %s);\n     return 1;',
        ['Vector2'] = '    return push_Vector2(L, %s);',
        ['Vector3'] = '    return push_Vector3(L, %s);',
        ['Image'] = '    return push_Image(L, %s);',
        ['Color'] = '    return push_Color(L, %s);',
        ['Texture'] = '    return push_Texture(L, %s);',
        ['Rectangle'] = '    return push_Rectangle(L, %s);',
        ['Wave'] = '    return push_Wave(L, %s);',
        ['Sound'] = '    return push_Sound(L, %s);',
        ['Music'] = '    return push_Music(L, %s);',
    },
    getter = 'static int f_%s_get_%s(lua_State *L) {\n%s\n}\n\n',
    setter = 'static int f_%s_set_%s(lua_State *L) {\n%s\n}\n\n',
}

local tmp_code = [[ ]]

function load_raylib()
    local fp <close> = assert(io.open('raylib.h', 'rb'))
    return assert(fp:read('a'))
end

function parse_function(line)
    local ret_type, ret_ptr, name, arg_str = string.match(line, 'RAYGUIAPI ([^%s]+) (%*?)(%w+)%(([^%)]+)%)')
    if ret_type and name then
        if not string.match(name, 'Unload.*') then
            -- parse all arguments
            local i = 1
            local arg_body = {}
            for arg_type, arg_ptr in string.gmatch(arg_str, '%s*([^,%*]+) (%*?)') do
                local arg_tpl = templates.args[arg_type .. (arg_ptr or '')]
                if arg_tpl then
                    arg_body[i] = string.format(arg_tpl, i)
                    i = i + 1
                else
                    arg_body = nil
                    break
                end
            end
            -- check if we have valid arguments and valid return type
            local ret_tpl = templates.returns[ret_type .. (ret_ptr or '')]
            local ret_body
            if ret_tpl and arg_body then
                ret_body = string.format(ret_tpl, string.format('%s(%s)', name, table.concat(arg_body, ', ')))
            else
                ret_body = '    return luaL_error(L, "not implemented");'
            end
            return string.format(templates.body, name, ret_body), string.format('        { "%s", f_%s },', name, name)
        end
    end
end

function parse_code()
    local c_code = {}
    local c_defs = {}
    local raylib = tmp_code --load_raylib()

    -- parse C function definitions
    for line in string.gmatch(raylib, '[^\r\n]+') do
        local code, def = parse_function(line)
        if code and def then
            c_code[#c_code + 1] = code
            c_defs[#c_defs + 1] = def
        end
    end

    -- parse structs
    for name, struct in string.gmatch(raylib, 'typedef struct (%w+) {([^}]+)}') do
        for field_type, field_name in string.gmatch(struct, '(%w+) (%w+);') do
            local getter = string.format(templates.returns[field_type], string.format('check_%s(L, 1)->%s', name, field_name))
            --local setter = string.format('    check_%s(L, 1)->%s = %s;\n    return 0;', name, field_name, string.format(templates.args[field_type], 2))
            c_code[#c_code + 1] = string.format(templates.getter, name, field_name, getter)
            --c_code[#c_code + 1] = string.format(templates.setter, name, field_name, setter)
            c_defs[#c_defs + 1] = string.format('    { "?%s", f_%s_get_%s },', field_name, name, field_name)
            --c_defs[#c_defs + 1] = string.format('    { "=%s", f_%s_set_%s },', field_name, name, field_name)
        end
    end

    -- parse enums
    for enums in string.gmatch(raylib, 'typedef enum {([^}]+)}') do
        for name in string.gmatch(enums, '([^%s,/]+).-\n') do
            c_defs[#c_defs + 1] = string.format('    { "%s", %s },', name, name)
        end
    end

    print(table.concat(c_code))
    print()
    print(table.concat(c_defs, '\n'))
end

parse_code()
