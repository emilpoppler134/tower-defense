#ifndef __TILES_H__
#define __TILES_H__

#include "raylib.h"

typedef enum tile_type_e
{
  TILE_GRASS,
  TILE_TREE_LEFT,
  TILE_TREE_RIGHT,
  TILE_FLOWER_LEFT,
  TILE_FLOWER_RIGHT,
  TILE_PLANT,
  TILE_ROCK_SMALL,
  TILE_ROCK_BIG,
  TILE_PATH_CORNER_TOP_LEFT,
  TILE_PATH_CORNER_TOP_RIGHT,
  TILE_PATH_CORNER_BOTTOM_LEFT,
  TILE_PATH_CORNER_BOTTOM_RIGHT,
  TILE_PATH_VERTICAL,
  TILE_PATH_HORIZONTAL,
  TILE_NUM_TYPES
} tile_type_e;

typedef enum direction_e
{
  DIR_NONE,
  DIR_UP,
  DIR_RIGHT,
  DIR_DOWN,
  DOR_LEFT
} direction_e;

bool can_place_on_tile[TILE_NUM_TYPES] = {
  true,     // Grass
  false,    // Tree left
  false,    // Tree right
  false,    // Flower left
  false,    // Flower right
  false,    // Plant
  false,    // Rock small
  false,    // Rock big
  false,    // Path corner top left
  false,    // Path corner top right
  false,    // Path corner bottom left
  false,    // Path corner bottom right
  false,    // Path vertical
  false     // Path horizontal
};

Vector2 directions[5] = {
  (Vector2) {0, 0},   // None
  (Vector2) {0, -1},  // Up
  (Vector2) {1, 0},   // Right
  (Vector2) {0, 1},   // Down
  (Vector2) {-1, 0}   // Left
};

#endif