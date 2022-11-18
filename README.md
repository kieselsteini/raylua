# rayLua - a simple new Lua binding for raylib

## Why?
Why yet another Lua binding for raylib if there are so many others already out there?

Well most Lua bindings I have found on GitHub were using the LuaJIT "ffi" package, which is a viable option when using LuaJIT, but not for usage with vanilla Lua. And then, all the other bindings seemed to be abandoned for years and programmed against an ancient version of raylib. I also wanted to use the real C structures for objects and not parse Lua tables on the fly when using them.

Last but not least, fun. I just discovered raylib and I enjoy Lua programming, so I thought I would be a perfect side-project for me (and the upcoming raylib game jam).

## State

Currently **167** functions are implemented. All *Unload* functions are not implemented as Lua will collect unused objects and recycle the memory itself.

- module: rcore 🚧
    - Window-related functions ✅ (80%)
        - GetWindowHandle ❌ (useless in Lua)
        - GetScreenWidth / GetScreenHeight 🌔 **GetScreenSize**
        - GetRenderWidth / GetRenderHeight 🌔 **GetRenderSize**
        - all monitor functions missing!!!
    - Custom frame control functions ✅ (100%)
    - Cursor-related functions ✅ (100%)
    - Drawing-related functions 🚧 (41%)
        - BeginMode2D / EndMode2D ❌
        - BeginMode3D / EndMode3D ❌
        - BeginTextureMode / EndTextureMode ❌
        - BeginShaderMode / EndShaderMode ❌
        - BeginVrStereoMode / EndVrStereoMode ❌
    - VR stereo config functions for VR simulator ❌
    - Shader management functions ❌
    - Screen-space-related functions ❌
    - Timing-related functions ✅ (100%)
    - Misc. functions ❌
    - Files management functions ✅ (100%)
    - Compression/Encoding functionality ✅ (100%)
    - Input-related functions: keyboard ✅ (100%)
    - Input-related functions: gamepads ✅ (100%)
    - Input-related functions: mouse ✅ (100%)
    - Input-related functions: touch ❌
- module: rgestures ❌
- module: rcamera ❌
- module: rshapes ✅ (100%)
- module: rtextures 🚧
    - Image loading functions ❌
    - Image generation functions ✅ (100%)
    - Image manipulation functions ❌
    - Image drawing functions ❌
    - Texture loading functions 🚧 (25%)
        - LoadTextureFromImage ❌
        - LoadTextureCubemap ❌
        - LoadRenderTexture ❌
    - Texture configuration functions ✅ (100%)
    - Texture drawing functions ❌
    - Color/pixel related functions 🚧
        - **Fade** is implemeted on the **Color** object
- module: rtext 🚧
    - Font loading/unloading functions 🚧 (42%)
        - LoadFontEx ❌
        - LoadFontFromImage ❌
        - LoadFontFromMemory 🌔 **LoadFontFromString**
        - LoadFontData ❌
        - GenImageFontAtlas ❌
    - Text drawing functions ✅ (83%)
        - DrawTextCodepoints ❌
    - Text font info functions 🚧
        - GetGlyphIndex ❌
        - GetGlyphInfo ❌
        - GetGlyphAtlasRec ❌
    - Text codepoints management functions (unicode characters) ❌ **use utf8.* module**
    - Text strings management functions (no UTF-8 strings, only byte chars) ❌ **use string.* module**
- module: rmodels ❌
- module: raudio ❌
- structs (objects)
    - **Vector2** ✅
        - almost complete implementation
        - some bits missing like DotProduct etc.
    - **Vector3** 🚧
        - basic member access done
        - basic math operations done
    - **Vector4** ❌
    - **Quaternion** ❌
    - **Matrix** ❌
    - **Color** ✅
        - almost complete
        - missing color conversion methods
    - **Rectangle** 🚧
        - basic member access done
    - **Image** 🚧
        - just started with implementation
    - **Texture** 🚧
        - just started with implementation
    - **RenderTexture** ❌
    - **NPatchInfo** ❌
    - **GlyphInfo** ❌
    - **Font** 🚧
        - just started with implementation
    - **Camera** ❌
    - **Camera2D** ❌
    - **Mesh** ❌
    - **Shader** ❌
    - **MaterialMap** ❌
    - **Material** ❌
    - **Model** ❌
    - **Transform** ❌
    - **BoneInfo** ❌
    - **ModelAnimation** ❌
    - **Ray** ❌
    - **RayCollision** ❌
    - **BoundingBox** ❌
    - **Wave** ❌
    - **Sound** ❌
    - **Music** ❌
    - **AudioStream** ❌
    - **VrDeviceInfo** ❌
    - **VrStereoConfig** ❌
    - **FilePathList** ❌
        - will probably never implemented, as this can be easily represented as a Lua table with strings

