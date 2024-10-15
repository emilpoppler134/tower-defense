#include <stdio.h>
#include <stdlib.h>

#include "include/entity.h"

#define TILE_SIZE 80
#define PLAYER_FILE_PATH "../../resources/players/player_3"

void serialize_player(const char *path, entity_t *player)
{
	FILE *file = fopen(path, "wb");
	if (!file)
	{
		perror("Failed to open player file");
		exit(1);
	}

	if (fwrite(player, sizeof(entity_t), 1, file) != 1)
	{
		perror("Failed to write player data");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

void initialize_player(entity_t *player)
{
  player->type = 2;
  player->cost = 750;
	player->radius = 200;
	player->interval = init_time_interval(0.25);
}

int main()
{
	entity_t player = init_entity();
	initialize_player(&player);
	serialize_player(PLAYER_FILE_PATH, &player);

	return 0;
}