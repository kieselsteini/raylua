--[[----------------------------------------------------------------------------
--]]----------------------------------------------------------------------------
local templates = {
    body = 'static int f_%s(lua_State *L) {\n%s\n}\n\n',
    args = {
        ['int'] = '(int)luaL_checknumber(L, %d)',
        ['float'] = '(float)luaL_checknumber(L, %d)',
        ['const char*'] = 'luaL_checkstring(L, %d)',
        ['Vector2'] = '*check_Vector2(L, %d)',
        ['Vector3'] = '*check_Vector3(L, %d)',
        ['Rectangle'] = '*check_Rectangle(L, %d)',
        ['Color'] = '*check_Color(L, %d)',
        ['Image'] = '*check_Image(L, %d)',
        ['Image*'] = 'check_Image(L, %d)',
        ['Font'] = '*check_Font(L, %d)',
    },
    returns = {
        ['void'] = '    %s;\n    return 0;',
        ['Image'] = '    return push_Image(L, %s);',
        ['Color'] = '    return push_Color(L, %s);',
        ['Rectangle'] = '    return push_Rectangle(L, %s);'
    }
}

local tmp_code = [[
RLAPI void ImageClearBackground(Image *dst, Color color);                                                // Clear image background with given color
RLAPI void ImageDrawPixel(Image *dst, int posX, int posY, Color color);                                  // Draw pixel within an image
RLAPI void ImageDrawPixelV(Image *dst, Vector2 position, Color color);                                   // Draw pixel within an image (Vector version)
RLAPI void ImageDrawLine(Image *dst, int startPosX, int startPosY, int endPosX, int endPosY, Color color); // Draw line within an image
RLAPI void ImageDrawLineV(Image *dst, Vector2 start, Vector2 end, Color color);                          // Draw line within an image (Vector version)
RLAPI void ImageDrawCircle(Image *dst, int centerX, int centerY, int radius, Color color);               // Draw circle within an image
RLAPI void ImageDrawCircleV(Image *dst, Vector2 center, int radius, Color color);                        // Draw circle within an image (Vector version)
RLAPI void ImageDrawRectangle(Image *dst, int posX, int posY, int width, int height, Color color);       // Draw rectangle within an image
RLAPI void ImageDrawRectangleV(Image *dst, Vector2 position, Vector2 size, Color color);                 // Draw rectangle within an image (Vector version)
RLAPI void ImageDrawRectangleRec(Image *dst, Rectangle rec, Color color);                                // Draw rectangle within an image
RLAPI void ImageDrawRectangleLines(Image *dst, Rectangle rec, int thick, Color color);                   // Draw rectangle lines within an image
RLAPI void ImageDraw(Image *dst, Image src, Rectangle srcRec, Rectangle dstRec, Color tint);             // Draw a source image within a destination image (tint applied to source)
RLAPI void ImageDrawText(Image *dst, const char *text, int posX, int posY, int fontSize, Color color);   // Draw text (using default font) within an image (destination)
RLAPI void ImageDrawTextEx(Image *dst, Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text (custom sprite font) within an image (destination)
]]

function load_raylib()
    local fp <close> = assert(io.open('raylib.h', 'rb'))
    return assert(fp:read('a'))
end

function parse_function(line)
    local ret_type, ret_ptr, name, arg_str = string.match(line, 'RLAPI ([^%s]+) (%*?)(%w+)%(([^%)]+)%)')
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
            return string.format(templates.body, name, ret_body), string.format('    { "%s", f_%s },', name, name)
        end
    end
end

function parse_code()
    local c_code = {}
    local c_defs = {}
    local raylib = tmp_code --load_raylib()
    for line in string.gmatch(raylib, '[^\r\n]+') do
        local code, def = parse_function(line)
        if code and def then
            c_code[#c_code + 1] = code
            c_defs[#c_defs + 1] = def
        end
    end
    print(table.concat(c_code))
    print()
    print(table.concat(c_defs, '\n'))
end

parse_code()
