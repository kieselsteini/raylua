# rayLua - a simple new Lua binding for raylib

## Why?
Why yet another Lua binding for raylib if there are so many others already out there?

Well most Lua bindings I have found on GitHub were using the LuaJIT "ffi" package, which is a viable option when using LuaJIT, but not for usage with vanilla Lua. And then, all the other bindings seemed to be abandoned for years and programmed against an ancient version of raylib. I also wanted to use the real C structures for objects and not parse Lua tables on the fly when using them.

Last but not least, fun. I just discovered raylib and I enjoy Lua programming, so I thought I would be a perfect side-project for me (and the upcoming raylib game jam).

## State

- module: rcore 🚧
    - Window-related functions ✅ (80%)
        - GetWindowHandle ❌ (useless in Lua)
        - GetScreenWidth / GetScreenHeight 🌔 **GetScreenSize**
        - GetRenderWidth / GetRenderHeight 🌔 **GetRenderSize**
        - all monitor functions missing!!!
    - Custom frame control functions ✅ (100%)
    - Cursor-related functions ✅ (100%)
    - Drawing-related functions 🚧 (64%)
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
    - Input-related functions: touch ✅ (100%)
- module: rgestures ✅ (100%)
- module: rcamera ✅ (100%)
- module: rshapes ✅ (100%)
- module: rtextures 🚧
    - Image loading functions ✅ (100%)
        - LoadImageFromMemory 🌔 **LoadImageFromString**
    - Image generation functions ✅ (100%)
    - Image manipulation functions ✅ (100%)
    - Image drawing functions ✅ (100%)
    - Texture loading functions 🚧 (25%)
        - LoadTextureFromImage ❌
        - LoadTextureCubemap ❌
        - LoadRenderTexture ❌
    - Texture configuration functions ✅ (100%)
    - Texture drawing functions ✅ (100%)
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
- module: raudio ✅ (AudioStream)
    - Audio device management functions ✅ (100%)
    - Wave/Sound loading/unloading functions ✅
        - LoadWaveFromMemory 🌔 **LoadWaveFromString**
    - Music management functions ✅
        - LoadMusicStreamFromMemory 🌔 **LoadMusicStreamFromString**
    - AudioStream management functions ❌
- structs (objects)
    - **Vector2** ✅
    - **Vector3** ✅
    - **Vector4** ❌
    - **Quaternion** ❌
    - **Matrix** ❌
    - **Color** ✅
    - **Rectangle** ✅
    - **Image** ✅
    - **Texture** ✅
    - **RenderTexture** ❌
    - **NPatchInfo** ❌
    - **GlyphInfo** ❌
    - **Font** ✅
    - **Camera3D** ✅
    - **Camera2D** ✅
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
    - **Wave** ✅
    - **Sound** ✅
    - **Music** ✅
    - **AudioStream** ❌
    - **VrDeviceInfo** ❌
    - **VrStereoConfig** ❌
    - **FilePathList** 🌔
        - will be converted "in-place" to Lua tables in functions which will return this
