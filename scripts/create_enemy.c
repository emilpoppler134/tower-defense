#include <stdio.h>
#include <stdlib.h>

#include "include/entity.h"

#define TILE_SIZE 80
#define ENEMY_FILE_PATH "../../resources/enemies/enemy_7"

void serialize_enemy(const char *path, entity_t *enemy)
{
	FILE *file = fopen(path, "wb");
	if (!file)
	{
		perror("Failed to open enemy file");
		exit(1);
	}

	if (fwrite(enemy, sizeof(entity_t), 1, file) != 1)
	{
		perror("Failed to write enemy data");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

void initialize_enemy(entity_t *enemy)
{
  enemy->type = 6;
	enemy->speed = 800.0f;
}

int main()
{
	entity_t enemy = init_entity();
	initialize_enemy(&enemy);
	serialize_enemy(ENEMY_FILE_PATH, &enemy);

	return 0;
}