#include <stdio.h>
#include "../include/raylib.h"
#include "../include/raymath.h"
const float GRAVITY = 9.81f;
const float JUMP_POWER = 8.0f;
const Color DEFUALT_PLAYER_COLOR = {255, 255, 255, 255};
const Color LOVELY_COLOR = {62, 70, 55, 255}; 
const float PLAYER_RADIUS = 0.5f;
const float PLAYER_HEIGHT = 2.0f;
Vector2 inputDirection;
int renderWidth = 320;
int renderHeight = 240;
int screenWidth = 1366;
int screenHeight = 768;
RenderTexture2D renderTarget;
Model playerModel;
Model levelModel;
Matrix levelTransform;
typedef struct {
    Vector3 position;
    Vector3 size;
    Vector3 wishDirection;
    Vector3 velocity;
    Color color;
    float moveSpeed;
    bool isOnGround;
} Player;
typedef struct {
    Camera3D rawCamera;
    Vector3 forward;
    Vector3 right;
    Vector3 targetPosition;
} PlayerCamera;
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
float FindFloor(Vector3 position, Vector3* outNormal) {
    Ray floorRay = {
        .position = {position.x, position.y + 100.0f, position.z},
        .direction = {0, -1, 0}
    };
    float closestDist = 10000.0f;
    Vector3 floorNormal = {0, 1, 0};
    bool foundFloor = false;
    for (int i = 0; i < levelModel.meshCount; i++) {
        RayCollision hit = GetRayCollisionMesh(floorRay, levelModel.meshes[i], levelTransform);
        if (hit.hit) {
            if (hit.normal.y > 0.01f) {
                float dist = hit.distance;
                if (dist < closestDist) {
                    closestDist = dist;
                    floorNormal = hit.normal;
                    foundFloor = true;
                }
            }
        }
    }
    if (foundFloor) {
        *outNormal = floorNormal;
        return floorRay.position.y - closestDist;
    }
    *outNormal = (Vector3){0, 1, 0};
    return -10000.0f;
}
int FindWallCollisions(Vector3* position, float radius) {
    int wallCount = 0;
    const int numDirections = 8;
    for (int dir = 0; dir < numDirections; dir++) {
        float angle = (float)dir / (float)numDirections * PI * 2.0f;
        Vector3 direction = {cosf(angle), 0, sinf(angle)};
        for (int h = 0; h < 3; h++) {
            float heightOffset = (float)h / 2.0f * PLAYER_HEIGHT;
            Ray wallRay = {
                .position = {position->x, position->y + heightOffset, position->z},
                .direction = direction
            };
            for (int m = 0; m < levelModel.meshCount; m++) {
                RayCollision hit = GetRayCollisionMesh(wallRay, levelModel.meshes[m], levelTransform);
                if (hit.hit && hit.distance < radius) {
                    if (fabsf(hit.normal.y) < 0.9f) {
                        float penetration = radius - hit.distance;
                        position->x += hit.normal.x * penetration;
                        position->z += hit.normal.z * penetration;
                        wallCount++;
                        goto next_direction;
                    }
                }
            }
        }
        next_direction:;
    }
    return wallCount;
}
void HandlePlayer(Player* player, PlayerCamera* camera, float delta) {
    if (!player->isOnGround) { player->velocity.y -= GRAVITY * delta; }
    if (player->isOnGround && IsKeyDown(KEY_SPACE)) {
        player->velocity.y = JUMP_POWER;
        player->isOnGround = false;
    }
    Vector3 horizontalVelocity = {
        player->wishDirection.x * player->moveSpeed,
        0,
        player->wishDirection.z * player->moveSpeed
    };
    player->position.x += horizontalVelocity.x * delta;
    player->position.z += horizontalVelocity.z * delta;
    for (int i = 0; i < 3; i++) {
        int walls = FindWallCollisions(&player->position, PLAYER_RADIUS);
        if (walls == 0) break;
    }
    player->position.y += player->velocity.y * delta;
    Vector3 floorNormal;
    float floorHeight = FindFloor(player->position, &floorNormal);
    if (floorHeight > -9999.0f) {
        float distToGround = player->position.y - floorHeight;
        if (distToGround <= 0.1f && player->velocity.y <= 0.0f) {
            player->position.y = floorHeight;
            player->velocity.y = 0.0f;
            player->isOnGround = true;
        }
        else if (distToGround < 0.0f) {
            player->position.y = floorHeight;
            player->velocity.y = 0.0f;
        }
        else {
            player->isOnGround = false;
        }
    } else {
        player->isOnGround = false;
    }
    if (player->position.y < -10.0f) {
        player->position = (Vector3){0, 5, 0};
        player->velocity = (Vector3){0, 0, 0};
    }
}
float WrapAngle(float a) {
    while (a > PI) a -= PI*2.0f;
    while (a < -PI) a += PI*2.0f;
    return a;
}
float LerpAngle(float a, float b, float t) {
    float diff = WrapAngle(b - a);
    return a + diff * t;
}
void DrawPlayer(Model playerModel, Vector3 position, Vector3 wishDirection) {
    static float yaw = 0.0f;
    static float targetYaw = 0.0f;
    if (Vector3LengthSqr(wishDirection) > 0.0001f) {
        wishDirection = Vector3Normalize(wishDirection);
        targetYaw = atan2f(-wishDirection.x, -wishDirection.z);
    }
    float turnSpeed = 8.0f * GetFrameTime();
    yaw = LerpAngle(yaw, targetYaw, turnSpeed);
    DrawModelEx(
        playerModel,
        position,
        (Vector3){ 0, 1, 0 },
        yaw * RAD2DEG,
        (Vector3){ 1, 1, 1 },
        WHITE
    );
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
    PlayerCamera camera = {
        .rawCamera = (Camera3D){
            .position = {0.0f, 3.0f, 5.0f},
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
        .position = {0.0f, 5.0f, 0.0f},
        .size = {1.0f, 2.0f, 1.0f},
        .wishDirection = {0.0f, 0.0f, 0.0f},
        .velocity = {0.0f, 0.0f, 0.0f},
        .color = DEFUALT_PLAYER_COLOR,
        .moveSpeed = 5.0f,
        .isOnGround = false
    };
    playerModel = LoadModel("assets/ShadowSlink.gltf");
    levelModel = LoadModel("assets/Bogmire Arena/bogmire-arena.obj");
    levelTransform = MatrixScale(2.0f, 2.0f, 2.0f);
    SetTargetFPS(0);
    while (!WindowShouldClose()) {
        float delta = GetFrameTime();
        inputDirection = GetInputDirection();
        camera.rawCamera.position = Vector3Add(player.position, (Vector3){0.0f, 4, 4.0f});
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
        player.wishDirection = Vector3Normalize(Vector3Add(
            Vector3Scale(camera.forward, inputDirection.y), 
            Vector3Scale(camera.right, inputDirection.x)
        ));
        HandlePlayer(&player, &camera, delta);
        BeginTextureMode(renderTarget);
            ClearBackground(LOVELY_COLOR);
            BeginMode3D(camera.rawCamera);
                DrawGrid(40, 4.0f);
                DrawPlayer(playerModel, player.position, player.wishDirection);
                DrawModel(levelModel, (Vector3){0.0f, 0.0f, 0.0f}, 2.0f, WHITE);
            EndMode3D();
        EndTextureMode();
        BeginDrawing();
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
            DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
