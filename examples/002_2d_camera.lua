-- initialization
local screenWidth <const> = 800
local screenHeight <const> = 450
local MAX_BUILDINGS <const> = 100

InitWindow(screenWidth, screenHeight, 'raylib [core] example - basic window')
SetTargetFPS(60) -- set our game to run at 60 frames per second

local player = Rectangle(400, 280, 40, 40)
local buildings = {}
local spacing = 0
for i = 1, MAX_BUILDINGS do
    local rectangle = Rectangle()
    rectangle.width = math.random(50, 200)
    rectangle.height = math.random(100, 800)
    rectangle.y = screenHeight - 130.0 - rectangle.height
    rectangle.x = -6000 + spacing
    spacing = spacing + rectangle.width
    buildings[i] = {
        rectangle = rectangle,
        color = Color(math.random(200, 240), math.random(200, 240), math.random(200, 240)),
    }
end

local camera = Camera2D()
camera.target = Vector2(player.x + 20, player.y + 20)
camera.offset = Vector2(screenWidth / 2, screenHeight / 2)
camera.rotation = 0.0
camera.zoom = 1.0

while not WindowShouldClose() do
    -- Update
    if IsKeyDown(KEY_RIGHT) then player.x = player.x + 2 end
    if IsKeyDown(KEY_LEFT) then player.x = player.x - 2 end

    -- Camera follows the player
    camera.target = Vector2(player.x + 20, player.y + 20)

    -- Camera rotation controls
    if IsKeyDown(KEY_A) then camera.rotation = camera.rotation - 1 end
    if IsKeyDown(KEY_S) then camera.rotation = camera.rotation + 1 end

    -- Limit camera rotation to 80 degrees (-40 to 40)
    if camera.rotation > 40 then
        camera.rotation = 40
    elseif camera.rotation < -40 then
        camera.rotation = -40
    end

    -- Camera zoom controls
    camera.zoom = camera.zoom + (GetMouseWheelMove().y * 0.05)
    if camera.zoom > 3 then
        camera.zoom = 3
    elseif camera.zoom < 0.1 then
        camera.zoom = 0.1
    end

    -- Camera reset (zoom and rotation)
    if IsKeyPressed(KEY_R) then
        camera.zoom = 1.0
        camera.rotation = 0.0
    end

    BeginDrawing()
        ClearBackground(RAYWHITE)

        BeginMode2D(camera)
            DrawRectangle(-6000, 320, 13000, 8000, DARKGRAY)
            for _, building in pairs(buildings) do
                DrawRectangleRec(building.rectangle, building.color)
            end
            DrawRectangleRec(player, RED)
            DrawLine(camera.target.x, -screenHeight*10, camera.target.x, screenHeight*10, GREEN)
            DrawLine(-screenWidth*10, camera.target.y, screenWidth*10, camera.target.y, GREEN)
        EndMode2D()

        DrawText("SCREEN AREA", 640, 10, 20, RED)

        DrawRectangle(0, 0, screenWidth, 5, RED)
        DrawRectangle(0, 5, 5, screenHeight - 10, RED)
        DrawRectangle(screenWidth - 5, 5, 5, screenHeight - 10, RED)
        DrawRectangle(0, screenHeight - 5, screenWidth, 5, RED)

        DrawRectangle( 10, 10, 250, 113, SKYBLUE:Fade(0.5))
        DrawRectangleLines( 10, 10, 250, 113, BLUE)

        DrawText('Free 2d camera controls:', 20, 20, 10, BLACK)
        DrawText('- Right/Left to move Offset', 40, 40, 10, DARKGRAY)
        DrawText('- Mouse Wheel to Zoom in-out', 40, 60, 10, DARKGRAY)
        DrawText('- A / S to Rotate', 40, 80, 10, DARKGRAY)
        DrawText('- R to reset Zoom and Rotation', 40, 100, 10, DARKGRAY)

        DrawText(string.format('%.2fKiB', collectgarbage('count')), 0, 0, 20, DARKGRAY)
    EndDrawing()
end
CloseWindow()
