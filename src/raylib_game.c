#include "raylib.h"
#include "screens.h"  // 保留这个引用，防止报错

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// ---------------------------------------------------------
// 1. 为了防止 GitHub 编译报错，必须保留的全局变量 (占位符)
// ---------------------------------------------------------
GameScreen currentScreen = 0;
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

// ---------------------------------------------------------
// 2. 贪吃蛇游戏核心代码
// ---------------------------------------------------------
#define MAX_SNAKE_LENGTH 256
const int SQUARE_SIZE = 31;
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 450;

typedef struct Snake {
    Vector2 position;
    Vector2 speed;
    Color color;
} Snake;

typedef struct Food {
    Vector2 position;
    bool active;
    Color color;
} Food;

// 游戏全局变量
static Snake snake[MAX_SNAKE_LENGTH] = { 0 };
static int snakeLength = 0;
static Food fruit = { 0 };
static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

// ---------------------------------------------------------
// 3. 游戏逻辑函数
// ---------------------------------------------------------

void InitGameLogic(void)
{
    framesCounter = 0;
    gameOver = false;
    pause = false;
    snakeLength = 1;

    // 初始化蛇
    snake[0].position = (Vector2){ SCREEN_WIDTH/2, SCREEN_HEIGHT/2 };
    snake[0].speed = (Vector2){ SQUARE_SIZE, 0 };
    snake[0].color = DARKBLUE;

    for (int i = 1; i < MAX_SNAKE_LENGTH; i++) {
        snake[i].position = (Vector2){ -100, -100 };
        snake[i].speed = (Vector2){ 0, 0 };
        snake[i].color = BLUE;
    }

    // 初始化水果
    fruit.active = true;
    fruit.color = RED;
    fruit.position = (Vector2){ (float)GetRandomValue(0, (SCREEN_WIDTH / SQUARE_SIZE) - 1) * SQUARE_SIZE, 
                                (float)GetRandomValue(0, (SCREEN_HEIGHT / SQUARE_SIZE) - 1) * SQUARE_SIZE };
}

void UpdateGameLogic(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // --- 手机触屏 + 键盘控制 ---
            int gesture = GetGestureDetected();
            
            if ((IsKeyPressed(KEY_RIGHT) || gesture == GESTURE_SWIPE_RIGHT) && (snake[0].speed.x == 0)) 
            { snake[0].speed = (Vector2){ SQUARE_SIZE, 0 }; }
            if ((IsKeyPressed(KEY_LEFT) || gesture == GESTURE_SWIPE_LEFT) && (snake[0].speed.x == 0)) 
            { snake[0].speed = (Vector2){ -SQUARE_SIZE, 0 }; }
            if ((IsKeyPressed(KEY_UP) || gesture == GESTURE_SWIPE_UP) && (snake[0].speed.y == 0)) 
            { snake[0].speed = (Vector2){ 0, -SQUARE_SIZE }; }
            if ((IsKeyPressed(KEY_DOWN) || gesture == GESTURE_SWIPE_DOWN) && (snake[0].speed.y == 0)) 
            { snake[0].speed = (Vector2){ 0, SQUARE_SIZE }; }

            // 移动速度控制 (每10帧移动一次)
            if ((framesCounter % 10) == 0) 
            {
                for (int i = snakeLength - 1; i > 0; i--) snake[i].position = snake[i - 1].position;
                snake[0].position.x += snake[0].speed.x;
                snake[0].position.y += snake[0].speed.y;
            }

            // 撞墙检测
            if ((snake[0].position.x >= SCREEN_WIDTH) || (snake[0].position.x < 0) ||
                (snake[0].position.y >= SCREEN_HEIGHT) || (snake[0].position.y < 0))
            {
                gameOver = true;
            }

            // 撞身体检测
            for (int i = 1; i < snakeLength; i++) {
                if ((snake[0].position.x == snake[i].position.x) && (snake[0].position.y == snake[i].position.y))
                    gameOver = true;
            }

            // 吃水果
            if ((snake[0].position.x == fruit.position.x) && (snake[0].position.y == fruit.position.y)) {
                snakeLength++;
                fruit.position = (Vector2){ (float)GetRandomValue(0, (SCREEN_WIDTH / SQUARE_SIZE) - 1) * SQUARE_SIZE, 
                                            (float)GetRandomValue(0, (SCREEN_HEIGHT / SQUARE_SIZE) - 1) * SQUARE_SIZE };
            }
            framesCounter++;
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP)) InitGameLogic();
    }
}

void DrawGameLogic(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        for (int i = 0; i < snakeLength; i++) DrawRectangleV(snake[i].position, (Vector2){ (float)SQUARE_SIZE, (float)SQUARE_SIZE }, (i == 0) ? DARKBLUE : BLUE);
        DrawRectangleV(fruit.position, (Vector2){ (float)SQUARE_SIZE, (float)SQUARE_SIZE }, RED);
    }
    else
    {
        DrawText("GAME OVER", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT/2 - 40, 40, RED);
        DrawText("Tap/Enter to Restart", SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 + 20, 20, DARKGRAY);
    }

    EndDrawing();
}

// 供主循环调用的总函数
void UpdateDrawFrame(void)
{
    UpdateGameLogic();
    DrawGameLogic();
}

// ---------------------------------------------------------
// 4. 主函数 (程序的入口)
// ---------------------------------------------------------
int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "My Snake Game");

    InitGameLogic();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    return 0;
}
