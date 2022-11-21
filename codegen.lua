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
// Image manipulation functions
RLAPI Image ImageCopy(Image image);                                                                      // Create an image duplicate (useful for transformations)
RLAPI Image ImageFromImage(Image image, Rectangle rec);                                                  // Create an image from another image piece
RLAPI Image ImageText(const char *text, int fontSize, Color color);                                      // Create an image from text (default font)
RLAPI Image ImageTextEx(Font font, const char *text, float fontSize, float spacing, Color tint);         // Create an image from text (custom sprite font)
RLAPI void ImageFormat(Image *image, int newFormat);                                                     // Convert image data to desired format
RLAPI void ImageToPOT(Image *image, Color fill);                                                         // Convert image to POT (power-of-two)
RLAPI void ImageCrop(Image *image, Rectangle crop);                                                      // Crop an image to a defined rectangle
RLAPI void ImageAlphaCrop(Image *image, float threshold);                                                // Crop image depending on alpha value
RLAPI void ImageAlphaClear(Image *image, Color color, float threshold);                                  // Clear alpha channel to desired color
RLAPI void ImageAlphaMask(Image *image, Image alphaMask);                                                // Apply alpha mask to image
RLAPI void ImageAlphaPremultiply(Image *image);                                                          // Premultiply alpha channel
RLAPI void ImageBlurGaussian(Image *image, int blurSize);                                                // Apply Gaussian blur using a box blur approximation
RLAPI void ImageResize(Image *image, int newWidth, int newHeight);                                       // Resize image (Bicubic scaling algorithm)
RLAPI void ImageResizeNN(Image *image, int newWidth,int newHeight);                                      // Resize image (Nearest-Neighbor scaling algorithm)
RLAPI void ImageResizeCanvas(Image *image, int newWidth, int newHeight, int offsetX, int offsetY, Color fill);  // Resize canvas and fill with color
RLAPI void ImageMipmaps(Image *image);                                                                   // Compute all mipmap levels for a provided image
RLAPI void ImageDither(Image *image, int rBpp, int gBpp, int bBpp, int aBpp);                            // Dither image data to 16bpp or lower (Floyd-Steinberg dithering)
RLAPI void ImageFlipVertical(Image *image);                                                              // Flip image vertically
RLAPI void ImageFlipHorizontal(Image *image);                                                            // Flip image horizontally
RLAPI void ImageRotateCW(Image *image);                                                                  // Rotate image clockwise 90deg
RLAPI void ImageRotateCCW(Image *image);                                                                 // Rotate image counter-clockwise 90deg
RLAPI void ImageColorTint(Image *image, Color color);                                                    // Modify image color: tint
RLAPI void ImageColorInvert(Image *image);                                                               // Modify image color: invert
RLAPI void ImageColorGrayscale(Image *image);                                                            // Modify image color: grayscale
RLAPI void ImageColorContrast(Image *image, float contrast);                                             // Modify image color: contrast (-100 to 100)
RLAPI void ImageColorBrightness(Image *image, int brightness);                                           // Modify image color: brightness (-255 to 255)
RLAPI void ImageColorReplace(Image *image, Color color, Color replace);                                  // Modify image color: replace color
RLAPI Color *LoadImageColors(Image image);                                                               // Load color data from image as a Color array (RGBA - 32bit)
RLAPI Color *LoadImagePalette(Image image, int maxPaletteSize, int *colorCount);                         // Load colors palette from image as a Color array (RGBA - 32bit)
RLAPI void UnloadImageColors(Color *colors);                                                             // Unload color data loaded with LoadImageColors()
RLAPI void UnloadImagePalette(Color *colors);                                                            // Unload colors palette loaded with LoadImagePalette()
RLAPI Rectangle GetImageAlphaBorder(Image image, float threshold);                                       // Get image alpha border rectangle
RLAPI Color GetImageColor(Image image, int x, int y);                                                    // Get image pixel color at (x, y) position
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
