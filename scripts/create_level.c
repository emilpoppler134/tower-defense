#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/raylib.h"

#define ROWS 10
#define COLS 15
#define LEVEL_FILE_PATH "../../resources/levels/level_1"

typedef enum direction_e {
  DIR_NONE,
  DIR_UP,
  DIR_RIGHT,
  DIR_DOWN,
  DOR_LEFT
} direction_e;

typedef struct level_t
{
  int tiles[ROWS][COLS];
  direction_e path[ROWS][COLS];
  Vector2 enemy_starting_tile;
  Vector2 enemy_starting_direction;
} level_t;

void SerializeLevel(const char *path, level_t *level)
{
	FILE *file = fopen(path, "wb");
	if (!file)
	{
		perror("Failed to open level file");
		exit(1);
	}

	if (fwrite(level, sizeof(level_t), 1, file) != 1)
	{
		perror("Failed to write level data");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

void InitializeLevel(level_t *level)
{
	int tiles[ROWS][COLS] = {
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
		{0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  7,  8, 13, 13, 9, 0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  0,  12, 0,  0,  12, 0,  0,  0,  7,  0,  0,  0,  0,  0},
    {0,  0,  12, 0,  0,  12, 0,  0,  0,  8, 13, 13, 13, 13, 13},
    {13, 13, 11, 0,  0,  12, 0,  0,  0,  12, 0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  10, 13, 13, 13, 11, 0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
    {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0},
	};

  direction_e path[ROWS][COLS] = {
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_RIGHT,  DIR_RIGHT, DIR_RIGHT, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_UP, DIR_NONE, DIR_NONE, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_UP, DIR_NONE, DIR_NONE, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_RIGHT,  DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT},
    {DIR_RIGHT, DIR_RIGHT, DIR_UP, DIR_NONE, DIR_NONE, DIR_DOWN, DIR_NONE, DIR_NONE, DIR_NONE, DIR_UP, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_RIGHT, DIR_UP, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE},
    {DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE,  DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE}
  };

  level->enemy_starting_tile = (Vector2){-1, 5};
  level->enemy_starting_direction = (Vector2){1, 0};

	memcpy(level->tiles, tiles, sizeof(int) * ROWS * COLS);
	memcpy(level->path, path, sizeof(direction_e) * ROWS * COLS);
}

int main()
{
	level_t level;
	InitializeLevel(&level);
	SerializeLevel(LEVEL_FILE_PATH, &level);

	return 0;
}