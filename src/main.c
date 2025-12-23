#include <stdio.h>
#include "../include/raylib.h"
#include "../include/raymath.h"
#define SOA_SCORE(struct_size, hot_bytes) \ (((double)((struct_size + 63) / 64)) / ((double)(hot_bytes) / 64.0))

const float GRAVITY = 9.81f;
const float JUMP_POWER = 8.0f;
const Color DEFUALT_PLAYER_COLOR = {255, 255, 255, 255};
int renderWidth = 320;
int renderHeight = 240;
int screenWidth = 1366;
int screenHeight = 768;
RenderTexture2D renderTarget;
typedef struct {
    Vector3 position;
    Vector3 size;
    Vector3 wishDirection;
    Vector3 velocity;
    Color color;
    float moveSpeed;
    bool isOnGround;
} Player;
Vector2 inputDirection;
typedef struct {
    Camera3D rawCamera;
    Vector3 forward;
    Vector3 right;
    Vector3 targetPosition;
} GameCamera;
Vector2 GetInputDirection(void) {
    Vector2 direction = {
        IsKeyDown(KEY_D) - IsKeyDown(KEY_A),
        IsKeyDown(KEY_S) - IsKeyDown(KEY_W)
    };
    return direction;
}
void ComputeRenderResolutionForWindowAspect(
    int windowPixelWidth,
    int windowPixelHeight,
    int fixedRenderPixelHeight,
    int* outRenderPixelWidth,
    int* outRenderPixelHeight
) {
    float windowAspectRatio = (float)windowPixelWidth / (float)windowPixelHeight;
    *outRenderPixelHeight = fixedRenderPixelHeight;
    *outRenderPixelWidth = (int)((float)fixedRenderPixelHeight * windowAspectRatio);
}
void HandlePlayer(Player* player, GameCamera* camera, float delta) {
    if (!player->isOnGround) { player->velocity.y -= GRAVITY * delta; }
    if (player->isOnGround && IsKeyDown(KEY_SPACE)) {
        player->velocity.y = 8.0f;
        player->isOnGround = false;
    }
    player->velocity.x = player->wishDirection.x * player->moveSpeed;
    player->velocity.z = player->wishDirection.z * player->moveSpeed;
    player->position.x += player->velocity.x * delta;
    player->position.y += player->velocity.y * delta;
    player->position.z += player->velocity.z * delta;
    if (player->position.y < 0.0f) {
        player->position.y = 0.0f;
        player->velocity.y = 0.0f;
        player->isOnGround = true;
    }
}
int main(void) {
    InitWindow(screenWidth, screenHeight, "Coward 3D!");
    ComputeRenderResolutionForWindowAspect(
        screenWidth,
        screenHeight,
        renderHeight,
        &renderWidth,
        &renderHeight
    );
    renderTarget = LoadRenderTexture(renderWidth, renderHeight);
    SetTextureFilter(renderTarget.texture, TEXTURE_FILTER_POINT);
    GameCamera camera = {
        .rawCamera = (Camera3D){
            .position = {0.0f, 10.0f, 10.0f},
            .target = {0.0f, 0.0f, 0.0f},
            .up = {0.0f, 1.0f, 0.0f},
            .fovy = 90.0f, 
            .projection = CAMERA_PERSPECTIVE
        },
        .forward = (Vector3){0.0f, 0.0f, 0.0f},
        .right = (Vector3){0.0f, 0.0f, 0.0f},
        .targetPosition = (Vector3){0.0f, 0.0f, 0.0f}
    };
    Player player = {
        .position = {0.0f, 0.0f, 0.0f},
        .size = {1.0f, 2.0f, 1.0f},
        .wishDirection = {0.0f, 0.0f, 0.0f},
        .velocity = {0.0f, 0.0f, 0.0f},
        .color = DEFUALT_PLAYER_COLOR,
        .moveSpeed = 5.0f,
        .isOnGround = true
    };
    SetTargetFPS(240);
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        inputDirection = GetInputDirection();
        camera.targetPosition = Vector3Add(player.position, (Vector3){0.0f, player.size.y / 2.0f, 0.0f});
        camera.rawCamera.target = Vector3Lerp(
            camera.rawCamera.target, 
            camera.targetPosition, 
            2.0f * delta); 
        camera.forward = Vector3Subtract(camera.rawCamera.position, camera.rawCamera.target);
        camera.forward.y = 0;
        camera.forward = Vector3Normalize(camera.forward);
        camera.right = Vector3CrossProduct(camera.rawCamera.up, camera.forward);
        camera.right = Vector3Normalize(camera.right);
        player.wishDirection = Vector3Normalize(Vector3Add(Vector3Scale(camera.forward, inputDirection.y), Vector3Scale(camera.right, inputDirection.x)));
        HandlePlayer(&player, &camera, delta);
        BeginTextureMode(renderTarget);
            ClearBackground((Color){116, 122, 131, 255});
            BeginMode3D(camera.rawCamera);
                DrawGrid(40, 4.0f);
                DrawCubeV(
                    Vector3Add(player.position, (Vector3){0.0f, player.size.y / 2.0f, 0.0f}),
                    player.size,
                    player.color
                );
            EndMode3D();
        EndTextureMode();
        BeginDrawing();
            ClearBackground((Color){116, 122, 131, 255});
            Rectangle sourceRenderTextureRect = {
                0.0f,
                0.0f,
                (float)renderTarget.texture.width,
                -(float)renderTarget.texture.height
            };
            float renderToWindowScale = fminf(
                (float)screenWidth  / (float)renderWidth,
                (float)screenHeight / (float)renderHeight
            );
            Rectangle destinationWindowRect = {
                (screenWidth  - (renderWidth  * renderToWindowScale)) * 0.5f,
                (screenHeight - (renderHeight * renderToWindowScale)) * 0.5f,
                renderWidth  * renderToWindowScale,
                renderHeight * renderToWindowScale
            };
            DrawTexturePro(
                renderTarget.texture,
                sourceRenderTextureRect,
                destinationWindowRect,
                (Vector2){ 0.0f, 0.0f },
                0.0f,
                WHITE
            );
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
