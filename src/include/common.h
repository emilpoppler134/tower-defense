#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <float.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "raylib.h"
#include "raymath.h"

#include "entity.h"
#include "level.h"

#define TILE_SIZE 80
#define COLS 15
#define ROWS 10

typedef enum state_e
{
  STATE_START_SCREEN,
  STATE_GAME_OVER,
  STATE_BUY,
  STATE_PLACE,
  STATE_REMOVE,
  STATE_NUM
} state_e;

typedef struct level_t
{ 
  tile_type_e tiles[ROWS][COLS];
  direction_e path[ROWS][COLS];
  Vector2 enemy_starting_tile;
  Vector2 enemy_starting_direction;
} level_t;

typedef struct game_t
{
  int bank;
  int hp;
  int enemy_type;
  int killed_enemy_count;
  int enemy_spawn_speed;
  dynamic_entity_array players;
  dynamic_entity_array enemies;
  dynamic_entity_array bullets;
} game_t;

#endif
