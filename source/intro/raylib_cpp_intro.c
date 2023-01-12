/**
 * @file benlib_intro.h
 * @author Ramon Santamaria (@raysan5), update by Bensuperpc
 * (bensuperpc@gmail.com)
 * @brief Raylib-cpp logo animation
 * @version 1.0.0
 * @date 2022-05-28
 *
 * zlib/libpng License
 *
 */

#include "raylib_cpp_intro.h"

void play_intro_raylib_cpp(const int screenWidth, const int screenHeight)
{
    int logoPositionX = screenWidth / 2 - 128;
    int logoPositionY = screenHeight / 2 - 128;

    int framesCounter = 0;
    int lettersCount = 0;

    int topSideRecWidth = 16;
    int leftSideRecHeight = 16;

    int bottomSideRecWidth = 16;
    int rightSideRecHeight = 16;

    int state = 0;  // Tracking animation states (State Machine)
    float alpha = 1.0f;  // Useful for fading

    // Main game loop
    while (!WindowShouldClose())  // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (state == 0)  // State 0: Small box blinking
        {
            framesCounter++;

            if (framesCounter == 120) {
                state = 1;
                framesCounter = 0;  // Reset counter... will be used later...
            }
        } else if (state == 1)  // State 1: Top and left bars growing
        {
            topSideRecWidth += 4;
            leftSideRecHeight += 4;

            if (topSideRecWidth == 256)
                state = 2;
        } else if (state == 2)  // State 2: Bottom and right bars growing
        {
            bottomSideRecWidth += 4;
            rightSideRecHeight += 4;

            if (bottomSideRecWidth == 256)
                state = 3;
        } else if (state == 3)  // State 3: Letters appearing (one by one)
        {
            framesCounter++;

            if (framesCounter / 12)  // Every 12 frames, one more letter!
            {
                lettersCount++;
                framesCounter = 0;
            }

            if (lettersCount >= 10)  // When all letters have appeared, just fade out everything
            {
                alpha -= 0.02f;

                if (alpha <= 0.0f) {
                    return;
                }
            }
        }

        if (IsKeyPressed(KEY_ENTER)) {
            return;
        }

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

        ClearBackground(RAYWHITE);

        if (state == 0) {
            if ((framesCounter / 15) % 2)
                DrawRectangle(logoPositionX, logoPositionY, 16, 16, DARKBLUE);
        } else if (state == 1) {
            DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, DARKBLUE);
            DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, DARKBLUE);
        } else if (state == 2) {
            DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, DARKBLUE);
            DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, DARKBLUE);

            DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, DARKBLUE);
            DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, DARKBLUE);
        } else if (state == 3) {
            DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(DARKBLUE, alpha));
            DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(DARKBLUE, alpha));

            DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(DARKBLUE, alpha));
            DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(DARKBLUE, alpha));

            DrawRectangle(screenWidth / 2 - 112, screenHeight / 2 - 112, 224, 224, Fade(RAYWHITE, alpha));

            DrawText(TextSubtext(" raylib", 0, lettersCount), screenWidth / 2 - 60, (int)((float)screenHeight / 2), 50, Fade(DARKBLUE, alpha));
            DrawText(TextSubtext("cpp", 0, lettersCount), screenWidth / 2 - 66, (int)((float)screenHeight / 2) + 40, 50, Fade(DARKBLUE, alpha));
        }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }
}
