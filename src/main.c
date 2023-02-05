//
// Created by AJ Austinson on 2/4/23.
//

#include <assert.h>
#include <inttypes.h>
#include <raylib.h>
#include <raymath.h>

#include "flecs.h"

#define ATLAS_CELLS_ACROSS 49
#define CELL_PLAYER 25
#define ROOM_CELL_WIDTH 40
#define ROOM_CELL_HEIGHT 22

typedef Rectangle CSourceRect;
typedef Vector2 CPosition;

typedef struct {
	Vector2 destination;
	float speed;
} CLerpDestination;

typedef struct {
	int x;
	int y;
} CTileCoords;

typedef struct {
	Texture2D texture;
} CTexture2D;

ECS_COMPONENT_DECLARE(CSourceRect);
ECS_COMPONENT_DECLARE(CPosition);
ECS_COMPONENT_DECLARE(CTexture2D);
ECS_COMPONENT_DECLARE(CLerpDestination);
ECS_COMPONENT_DECLARE(CTileCoords);

ECS_TAG_DECLARE(TileMovable);
ECS_TAG_DECLARE(TileBreakable);

void RenderSprites(ecs_iter_t *it) {
	const CPosition *p = ecs_field(it, CPosition, 1);
	const CSourceRect *rect = ecs_field(it, CSourceRect, 2);
	const CTexture2D *tex =  ecs_singleton_get(it->world, CTexture2D);

	for (int i = 0; i < it->count; i++) {
		DrawTextureRec(tex->texture, rect[i], p[i], WHITE);
	}
}

void UpdateLerpDestinations(ecs_iter_t *it) {
	CPosition *p = ecs_field(it, CPosition, 1);
	const CLerpDestination *d = ecs_field(it, CLerpDestination, 2);
	for (int i = 0; i < it->count; i++) {
		p[i] = Vector2Lerp(p[i], d[i].destination, it->delta_time * d[i].speed);
	}
}

void UpdatePosition(ecs_iter_t *it) {
	CPosition *p = ecs_field(it, CPosition, 1);
	const CTileCoords *tc = ecs_field(it, CTileCoords, 2);
	for (int i = 0; i < it->count; i++) {
		p[i].x = 16.0f * (float)tc[i].x;
		p[i].y = 16.0f * (float)tc[i].y;
	}
}

int main(int argc, char** argv) {
	ecs_world_t *world = ecs_init();

	ECS_COMPONENT_DEFINE(world, CSourceRect);
	ECS_COMPONENT_DEFINE(world, CPosition);
	ECS_COMPONENT_DEFINE(world, CTexture2D);
	ECS_COMPONENT_DEFINE(world, CLerpDestination);
	ECS_COMPONENT_DEFINE(world, CTileCoords);

	ECS_TAG_DEFINE(world, TileBreakable);
	ECS_TAG_DEFINE(world, TileMovable);

	InitWindow(1280, 704, "CardinalRogue");

	ECS_SYSTEM(world, RenderSprites, EcsOnStore, [in] CPosition, [in] CSourceRect);
	ECS_SYSTEM(world, UpdateLerpDestinations, EcsOnUpdate, CPosition, [in] CLerpDestination);
	ECS_SYSTEM(world, UpdatePosition, EcsPreUpdate, CPosition, [in] CTileCoords);

	Camera2D cam = (Camera2D) {
		.target = (Vector2) { 0, 0 },
		.rotation = 0.0f,
		.offset = (Vector2) { 0, 0 },
		.zoom = 2.0f
	};

	ecs_entity_t player = ecs_new_id(world);
	ecs_set(world, player, CSourceRect, {16*25, 0, 16, 16});
	ecs_set(world, player, CTileCoords, {2, 2});
	ecs_add(world, player, CPosition);

	Texture2D tex = LoadTexture("data/kenney_1bit.png");
	ecs_singleton_set(world, CTexture2D, {tex});

	Vector2 screenCenter = (Vector2) {
		.x = 1280.0f / 4.0f,
		.y = 720.0f / 4.0f
	};
	for(int y = 0; y < 22; y++) {
		for(int x = 0; x < 40; x++) {
			ecs_entity_t tile = ecs_new_id(world);
			ecs_set(world, tile, CTileCoords, { x, y });
			ecs_set(world, tile, CPosition, { x * 16, y * 16 });
			if(Vector2Distance(screenCenter, (Vector2){ x * 16, y * 16 }) < 200) {
				ecs_add_id(world, tile, TileMovable);
			}
			else {
				ecs_set(world, tile, CSourceRect, { 160, 272, 16, 16});
				ecs_add_id(world, tile, TileBreakable);
			}
		}
	}

	while(!WindowShouldClose()) {
		float dt = GetFrameTime();
		double t = GetTime();

		const CPosition *p = ecs_get(world, player, CPosition);
		Vector2 toPosition = (Vector2)*p;
		if(IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) {
			toPosition.y -= 16.0f;
		}
		if(IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN)) {
			toPosition.y += 16.0f;
		}
		if(IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT)) {
			toPosition.x -= 16.0f;
		}
		if(IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT)) {
			toPosition.x += 16.0f;
		}

		if(!Vector2Equals(*p, toPosition)) {
			ecs_set(world, player, CLerpDestination, { toPosition, 20.0f });
		}

		ecs_run(world, UpdateLerpDestinations, dt, NULL);

		ClearBackground(BLACK);
		BeginDrawing();
		BeginMode2D(cam);

		ecs_run(world, RenderSprites, dt, NULL);

		EndMode2D();
		EndDrawing();
	}

	CloseWindow();
	ecs_fini(world);
	return 0;
}
