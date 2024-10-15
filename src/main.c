#include "include/common.h"
#include "include/entity.h"
#include "include/interval.h"
#include "include/launcher.h"
#include "include/level.h"
#include "include/server.h"

#include "entity.c"
#include "interval.c"
#include "launcher.c"
#include "server.c"

void deserialize_level(level_t *level)
{
  FILE *file = fopen("./src/assets/levels/level_1", "rb");
  if (!file)
  {
    perror("Failed to open level file");
    exit(1);
  }

  if (fread(level, sizeof(level_t), 1, file) != 1)
  {
    perror("Failed to read level data");
    fclose(file);
    exit(1);
  }

  fclose(file);
}

bool is_position_empty(dynamic_entity_array *players, int tile_x, int tile_y)
{
  for (int i = 0; i < players->count; i++)
  {
    entity_t *player = &players->data[i];

    // Create the bounds
    Rectangle player_bounds = {player->position.x, player->position.y, TILE_SIZE, TILE_SIZE};
    Rectangle mouse_bounds = {tile_x * TILE_SIZE, tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

    // Check if the players's bounding box overlaps with the mouse's bounding box
    if (CheckCollisionRecs(player_bounds, mouse_bounds))
    {
      return false;
    }
  }
  
  return true;
}

// Define a mutex to protect the shared data structure
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
package_t shared_package;

bool should_run_code = false;

void *receive_thread(void *arg)
{
  int client_socket = *((int *)arg);

  while (1)
  {
    package_t package;
    size_t size = sizeof(package_t);
    unsigned char buffer[size];

    ssize_t bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0)
    {
      if (bytes_received == 0)
      {
        printf("Client disconnected.\n");
      }
      else
      {
        perror("Error in recv");
      }

      pthread_exit(NULL);
    }

    memcpy(&package, buffer, size);

    // Lock the mutex before updating the shared data
    pthread_mutex_lock(&mutex);
    shared_package = package;

    // Set the flag to indicate that the code should run
    should_run_code = true;

    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

// Main
//--------------------------------------------------------------------------------------
int main()
{
  // Initialization
  //--------------------------------------------------------------------------------------
  int client_socket = 0;
  game_mode_e game_mode = open_launcher(&client_socket);

  if (game_mode == MODE_NULL) 
  {
    return 0;
  }

  printf("%d\n", client_socket);

  const int screen_width = 1200;
  const int screen_height = 800;

  InitWindow(screen_width, screen_height, "Bloons TD");

  state_e state = STATE_START_SCREEN;

  level_t level;
  deserialize_level(&level); // deserialize level

  // Setting the game settings
  game_t game;
  game.bank = 100;
  game.hp = 100;
  game.enemy_type = 0;
  game.killed_enemy_count = 0;
  game.enemy_spawn_speed = 3;

  // Init the arrays for all entities
  game.players = init_entity_array();
  game.enemies = init_entity_array();
  game.bullets = init_entity_array();

  // Loading the tileset
  Texture2D tileset = LoadTexture("./src/assets/tileset.png");
  tileset.width = TILE_SIZE * 10;
  tileset.height = TILE_SIZE * 10;

  // Loading the players texture
  Texture2D entity_texture = LoadTexture("./src/assets/entities.png");
  entity_texture.width = TILE_SIZE * 15;
  entity_texture.height = TILE_SIZE * 15;

  // Loading the players texture
  Texture2D shovel_texture = LoadTexture("./src/assets/shovel.png");
  shovel_texture.width = 612;
  shovel_texture.height = 612;

  dynamic_entity_array player_types = init_entity_array();
  deserialize_entities(&player_types, "players");

  dynamic_entity_array enemy_types = init_entity_array();
  deserialize_entities(&enemy_types, "enemies");

  entity_t placeing_player = init_entity();

  time_interval_t enemy_spawn_interval = init_time_interval(game.enemy_spawn_speed);
  int spawned_enemies = 0;

  pthread_t receive_tid;

  if (game_mode == MODE_MULTIPLAYER)
  {
    // Create the receive thread
    if (pthread_create(&receive_tid, NULL, receive_thread, &client_socket) != 0)
    {
      perror("Error creating receive thread");
      exit(1);
    }
  }
  //--------------------------------------------------------------------------------------


  while (!WindowShouldClose())
  {
    // Input
    //----------------------------------------------------------------------------------
    if (IsMouseButtonPressed(0))
    {
      int mouse_x = GetMouseX();
      int mouse_y = GetMouseY();

      // Calculate the tile coordinates based on mouse position
      int tile_x = mouse_x / TILE_SIZE;
      int tile_y = mouse_y / TILE_SIZE;

      // Check if the calculated tile coordinates are within the valid range
      if (tile_x >= 0 && tile_x < COLS && tile_y >= 0 && tile_y < ROWS)
      {
        switch (state)
        {
          case STATE_START_SCREEN:
          {
            Rectangle button_bounds = {screen_width / 2 - 190, screen_height / 2 - 40, 380, 80};

            if (CheckCollisionPointRec(GetMousePosition(), button_bounds))
            {
              if (game_mode == MODE_MULTIPLAYER)
              {
                package_t package = init_package();
                package.action = ACTION_START_GAME;

                size_t size = sizeof(package_t);
                send(client_socket, &package, size, 0);
              }
                  
              state = STATE_BUY;
            }
          } break;

          case STATE_BUY:
          {
            for (int i = 0; i < player_types.count; i++)
            {
              entity_t player = player_types.data[i];
              Rectangle button_bounds = {160 + i * TILE_SIZE, screen_height - TILE_SIZE, TILE_SIZE, TILE_SIZE};

              if (CheckCollisionPointRec(GetMousePosition(), button_bounds))
              {
                if (game.bank >= player.cost)
                {
                  placeing_player = player;
                  state = STATE_PLACE;
                }
              }
            }

            Rectangle shovel_button_bounds = {screen_width - TILE_SIZE, screen_height - TILE_SIZE, TILE_SIZE, TILE_SIZE};
            if (CheckCollisionPointRec(GetMousePosition(), shovel_button_bounds))
            {
              state = STATE_REMOVE;
            }
          } break;

          case STATE_PLACE:
          {
            if (is_position_empty(&game.players, tile_x, tile_y))
            {
              if (can_place_on_tile[level.tiles[tile_y][tile_x]])
              {
                if (game.bank >= placeing_player.cost)
                {
                  game.bank -= placeing_player.cost;

                  // Create a new player
                  entity_t player = init_entity();
                  player.position = (Vector2){tile_x * TILE_SIZE, tile_y * TILE_SIZE};
                  player.type = placeing_player.type;
                  player.radius = placeing_player.radius;
                  player.cost = placeing_player.cost;
                  player.interval = placeing_player.interval;
                  push(&game.players, player);

                  if (game_mode == MODE_MULTIPLAYER)
                  {
                    package_t package = init_package();
                    package.action = ACTION_PLACE;
                    package.entity = player;

                    size_t size = sizeof(package_t);
                    send(client_socket, &package, size, 0);
                  }

                  state = STATE_BUY;
                }
              }
            }
          } break;

          case STATE_REMOVE:
          {
            if (!is_position_empty(&game.players, tile_x, tile_y))
            {
              for (int i = 0; i < game.players.count; i++)
              {
                entity_t *player = &game.players.data[i];

                int player_tile_x = player->position.x / TILE_SIZE;
                int player_tile_y = player->position.y / TILE_SIZE;

                if (tile_x == player_tile_x && tile_y == player_tile_y)
                {
                  game.bank += player->cost / 2;

                  if (game_mode == MODE_MULTIPLAYER)
                  {
                    package_t package = init_package();
                    package.action = ACTION_REMOVE;
                    package.index = i;

                    size_t size = sizeof(package_t);
                    send(client_socket, &package, size, 0);
                  }

                  remove_at(&game.players, i);
                  break;
                }
              }

              state = STATE_BUY;
            }
          } break;

          default:
          break;
        }
      }
    }

    if (IsKeyPressed(KEY_TAB))
    {
      if (state == STATE_PLACE || state == STATE_REMOVE)
      {
        placeing_player = init_entity();
        state = STATE_BUY;
      }
    }
    //----------------------------------------------------------------------------------


    // Update
    //----------------------------------------------------------------------------------
    if (should_run_code)
    {
      // Check for received packages in the main thread
      pthread_mutex_lock(&mutex);
      package_t received_package = shared_package;
      pthread_mutex_unlock(&mutex);

      // Handle the received package in the main thread
      switch (received_package.action)
      {
        case ACTION_PLACE:
        {
          entity_t player = received_package.entity;
          push(&game.players, player);
          game.bank -= player.cost;
        } break;

        case ACTION_REMOVE:
        {
          remove_at(&game.players, received_package.index);
        } break;

        case ACTION_START_GAME:
        {
          state = STATE_BUY;
        } break;

        default:
          break;
      }

      should_run_code = false;
    }

    
    if (state != STATE_START_SCREEN && state != STATE_GAME_OVER)
    {
      // Enemy spawn loop
      if (check_time_interval(&enemy_spawn_interval))
      {
        // Create an enemy
        entity_t enemy = init_entity();
        enemy.position = (Vector2){level.enemy_starting_tile.x * TILE_SIZE, level.enemy_starting_tile.y * TILE_SIZE};
        enemy.direction = level.enemy_starting_direction;
        enemy.speed = enemy_types.data[game.enemy_type].speed;
        enemy.type = game.enemy_type;
        push(&game.enemies, enemy);

        spawned_enemies++;
        
        if (spawned_enemies % 20 == 0)
        {
          game.enemy_type++;
        }
      }

      // Player loop
      for (int i = 0; i < game.players.count; i++)
      {
        entity_t *player = &game.players.data[i];
        Vector2 player_center = {player->position.x + TILE_SIZE / 2, player->position.y + TILE_SIZE / 2};
        
        // Player shooting loop
        if (check_time_interval(&player->interval))
        {
          // Create an array with all of the enemies that is in the players radius
          dynamic_entity_array enemies_in_radius = init_entity_array();

          for (int j = 0; j < game.enemies.count; j++)
          {
            entity_t enemy = game.enemies.data[j];
            Rectangle enemy_bounds = {enemy.position.x, enemy.position.y, TILE_SIZE, TILE_SIZE};

            // If enemy is inside player radius
            if (CheckCollisionCircleRec(player_center, player->radius, enemy_bounds))
            {
              push(&enemies_in_radius, enemy); // Push to the dynamic array
            }
          }
          
          float lowest_enemy_type = FLT_MAX;
          Vector2 lowest_enemy_direction = {0, 0};

          for (int j = 0; j < enemies_in_radius.count; j++)
          {
            entity_t *enemy = &enemies_in_radius.data[j];
            Vector2 enemy_center = {enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2};

            // Calculate the distance between the player and the enemy
            float distance_to_enemy = Vector2Distance(player_center, enemy_center);

            // Check if this enemy is lower than the previously lowest enemy
            if (enemy->type < lowest_enemy_type)
            {
              lowest_enemy_type = enemy->type;
              // Calculate the direction vector from the player to the enemy
              lowest_enemy_direction = Vector2Normalize(Vector2Subtract(enemy_center, player_center));
            }
          }

          if (enemies_in_radius.count > 0) {
            float rotation_decimal = (float)(atan2(lowest_enemy_direction.y, lowest_enemy_direction.x) / (2 * PI));
            float rotation = rotation_decimal * 360;
            player->rotation = rotation + 90;

            // Create a bullet and set the direction and speed
            entity_t bullet = init_entity();
            bullet.position = (Vector2){player->position.x, player->position.y};
            bullet.direction = lowest_enemy_direction;
            bullet.speed = 2000.0f;
            bullet.rotation = rotation;
            bullet.type = player->type;
            push(&game.bullets, bullet);
          }
        }
      }

      // Enemy loop
      for (int i = 0; i < game.enemies.count; i++)
      {
        entity_t *enemy = &game.enemies.data[i];

        // Calculate the distance to move based on the elapsed time
        float delta_time = GetFrameTime();
        float distance_to_move = enemy->speed * delta_time;
        
        // Update enamy position based on direction and speed
        enemy->position.x += enemy->direction.x * distance_to_move;
        enemy->position.y += enemy->direction.y * distance_to_move;

        int tile_x = (int)enemy->position.x / TILE_SIZE;
        int tile_y = (int)enemy->position.y / TILE_SIZE;

        if ((int)enemy->position.y % TILE_SIZE == 0) {
          direction_e direction = level.path[tile_y][tile_x];
          enemy->direction = directions[direction];
        }

        // Check if the enemy is out of bounds and remove it
        if (enemy->position.x >= screen_width)
        {
          game.hp -= 10;
          remove_at(&game.enemies, i);

          if (game.hp <= 0)
          {
            state = STATE_GAME_OVER;
          }
          break;
        }
      }

      // Bullet loop
      for (int i = 0; i < game.bullets.count; i++)
      {
        entity_t *bullet = &game.bullets.data[i];

        float delta_time = GetFrameTime();
        float distance_to_move = bullet->speed * delta_time;
        
        // Update bullet position based on direction and speed
        bullet->position.x += bullet->direction.x * distance_to_move;
        bullet->position.y += bullet->direction.y * distance_to_move;

        // Iterate through enemies to check for hits
        for (int j = 0; j < game.enemies.count; j++)
        {
          entity_t *enemy = &game.enemies.data[j];

          // Calculate the bounding boxes of the bullet and enemy
          Rectangle bullet_bounds = {bullet->position.x, bullet->position.y, TILE_SIZE, TILE_SIZE};
          Rectangle enemy_bounds = {enemy->position.x, enemy->position.y, TILE_SIZE, TILE_SIZE};

          // Check if the bullet's bounding box overlaps with the enemy's bounding box
          if (CheckCollisionRecs(bullet_bounds, enemy_bounds))
          {
            Vector2 enemy_center = {enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2};
            Vector2 bullet_center = {bullet->position.x + TILE_SIZE / 2, bullet->position.y + TILE_SIZE / 2};

            if (CheckCollisionCircles(enemy_center, TILE_SIZE / 4, bullet_center, TILE_SIZE / 4))
            {
              // Lower enamy hp
              enemy->type--;
              enemy->speed = enemy_types.data[enemy->type].speed;

              // If enemy gets killed
              if (enemy->type < 0)
              {
                game.killed_enemy_count += 1; // Add one to the killed enemy counter
                game.bank += 25; // Add to bank
                remove_at(&game.enemies, j); // Remove the enemy
              }

              // Remove the bullet
              remove_at(&game.bullets, i);
            }
          }
        }

        // Check if the bullet is out of bounds and remove it
        if (bullet->position.x >= screen_width ||
          bullet->position.x <= 0 - TILE_SIZE ||
          bullet->position.y >= screen_height ||
          bullet->position.y <= 0)
        {
          remove_at(&game.bullets, i);
        }
      }
    }
    //----------------------------------------------------------------------------------


    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
      ClearBackground(RAYWHITE);
      
      // Draw the tilemap
      for (int y = 0; y < ROWS; y++) {
        for (int x = 0; x < COLS; x++) {
          int tile = level.tiles[y][x];

          Rectangle dest_rect = { x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
          Rectangle source_rect = {(tile % (tileset.width / TILE_SIZE)) * TILE_SIZE, (tile / (tileset.width / TILE_SIZE)) * TILE_SIZE, TILE_SIZE, TILE_SIZE };

          DrawTexturePro(tileset, source_rect, dest_rect, Vector2Zero(), 0.0f, WHITE);
        }
      }

      DrawFPS(10, 10);

      // Draw the players
      for (int i = 0; i < game.players.count; i++)
      {
        entity_t *player = &game.players.data[i];

        DrawTexturePro(entity_texture,
          (Rectangle){80, player->type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
          (Rectangle){player->position.x + TILE_SIZE / 2, player->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
          player->rotation,
          WHITE);

        DrawCircle(player->position.x + TILE_SIZE / 2,
          player->position.y + TILE_SIZE / 2,
          player->radius,
          (Color){255, 255, 255, 10});
      }
      
      // Draw the enemies
      for (int i = 0; i < game.enemies.count; i++)
      {
        entity_t *enemy = &game.enemies.data[i];
        
        DrawTexturePro(entity_texture,
          (Rectangle){TILE_SIZE * 5, enemy->type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
          (Rectangle){enemy->position.x + TILE_SIZE / 2, enemy->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
          0,
          WHITE);
      }

      // Draw the bullets
      for (int i = 0; i < game.bullets.count; i++)
      {
        entity_t *bullet = &game.bullets.data[i];

        DrawTexturePro(entity_texture,
          (Rectangle){TILE_SIZE * 10, bullet->type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
          (Rectangle){bullet->position.x + TILE_SIZE / 2, bullet->position.y + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
          (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
          bullet->rotation,
          WHITE);
      }

      if (state == STATE_BUY)
      {
        DrawText(TextFormat("Kills: %d", game.killed_enemy_count), screen_width - 120, 10, 28, WHITE);

        DrawRectangle(0, screen_height - TILE_SIZE, screen_width, TILE_SIZE, (Color){0, 0, 0, 100});
        DrawText(TextFormat("Bank: %d", game.bank), 15, screen_height - 65, 22, WHITE);
        DrawText(TextFormat("Hp: %d", game.hp), 15, screen_height - 35, 22, WHITE);

        for (int i = 0; i < player_types.count; i++)
        {
          entity_t *player = &player_types.data[i];

          DrawTexturePro(entity_texture,
            (Rectangle){0, i * TILE_SIZE, TILE_SIZE, TILE_SIZE},
            (Rectangle){160 + i * TILE_SIZE + TILE_SIZE / 2, screen_height - TILE_SIZE / 2, TILE_SIZE - 16, TILE_SIZE - 16},
            (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
            0,
            game.bank >= player->cost ? WHITE : (Color){255, 255, 255, 100});

          const char* cost_text = TextFormat("%d", player->cost);
          DrawText(cost_text, (160 + i * TILE_SIZE + (TILE_SIZE / 2) - (MeasureText(cost_text, 20) / 2)), (screen_height - 20), 20, WHITE);
        }

        DrawTexturePro(shovel_texture,
        (Rectangle){0, 0, shovel_texture.width, shovel_texture.height},
        (Rectangle){screen_width - TILE_SIZE / 2, screen_height - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
        (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
        0,
        WHITE);
      }

      if (state == STATE_PLACE)
      {
        DrawText("Press TAB to cancel", 8, 36, 22, WHITE);

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // Calculate the tile coordinates based on mouse position
        int tile_x = mouse_x / TILE_SIZE;
        int tile_y = mouse_y / TILE_SIZE;

        // Check if the calculated tile coordinates are within the valid range
        if (tile_x >= 0 && tile_x < COLS && tile_y >= 0 && tile_y < ROWS)
        {
          bool can_place = is_position_empty(&game.players, tile_x, tile_y) && can_place_on_tile[level.tiles[tile_y][tile_x]];

          DrawTexturePro(entity_texture,
            (Rectangle){80, placeing_player.type * TILE_SIZE, TILE_SIZE, TILE_SIZE},
            (Rectangle){tile_x * TILE_SIZE + TILE_SIZE / 2, tile_y * TILE_SIZE + TILE_SIZE / 2, TILE_SIZE, TILE_SIZE},
            (Vector2){TILE_SIZE / 2, TILE_SIZE / 2},
            0,
            can_place ? (Color){255, 255, 255, 200} : (Color){255, 255, 255, 75});

          DrawCircle(tile_x * TILE_SIZE + TILE_SIZE / 2,
            tile_y * TILE_SIZE + TILE_SIZE / 2,
            placeing_player.radius,
            (Color){255, 255, 255, 25});
        }
      }

      if (state == STATE_REMOVE)
      {
        DrawText("Press TAB to cancel", 8, 36, 22, WHITE);

        int mouse_x = GetMouseX();
        int mouse_y = GetMouseY();

        // Calculate the tile coordinates based on mouse position
        int tile_x = mouse_x / TILE_SIZE;
        int tile_y = mouse_y / TILE_SIZE;

        if (tile_x >= 0 && tile_x < COLS && tile_y >= 0 && tile_y < ROWS)
        {
          DrawRectangle(tile_x * TILE_SIZE, tile_y * TILE_SIZE, TILE_SIZE, TILE_SIZE, (Color){255, 255, 255, 25});
        }
      }

      if (state == STATE_START_SCREEN)
      {
        DrawRectangle(0, 0, screen_width, screen_height, (Color){0, 0, 0, 100});

        DrawRectangle(screen_width / 2 - 190, screen_height / 2 - 40, 380, 80, (Color){255, 255, 255, 200});
        DrawText("Play", screen_width / 2 - MeasureText("Play", 28) / 2, screen_height / 2 - 40 + 22, 28, BLACK);
      }

      if (state == STATE_GAME_OVER)
      {
        DrawRectangle(0, 0, screen_width, screen_height, (Color){0, 0, 0, 100});
        DrawText("GAME OVER", screen_width / 2 - MeasureText("GAME OVER", 42) / 2, screen_height / 2 - 40 + 22, 42, BLACK);
      }

    EndDrawing();
    //----------------------------------------------------------------------------------

    if (game_mode == MODE_MULTIPLAYER)
    {
      usleep(10000);
    }
  }

  if (game_mode == MODE_MULTIPLAYER)
  { 
    close(client_socket);
  }

  UnloadTexture(tileset);
  UnloadTexture(entity_texture);
  UnloadTexture(shovel_texture);

  free(game.players.data);
  free(game.enemies.data);
  free(game.bullets.data);

  CloseWindow();
  return 0;
}