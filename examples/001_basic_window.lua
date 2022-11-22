-- initialization
local screenWidth <const> = 800
local screenHeight <const> = 450

InitWindow(screenWidth, screenHeight, 'raylib [core] example - basic window')
SetTargetFPS(60) -- set our game to run at 60 frames per second
while not WindowShouldClose() do
    BeginDrawing()
        ClearBackground(RAYWHITE)
        DrawText('Congrats! You created your first window!', 190, 200, 20, LIGHTGRAY)
    EndDrawing()
end
CloseWindow()
