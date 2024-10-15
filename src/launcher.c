#include "include/launcher.h"

bool player_connected = false;

void *connect_thread(void *arg)
{
  int *client_socket_ptr = (int *)arg;
  int client_socket = *client_socket_ptr;
  int server_socket = 0;

  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
  {
    perror("Error in socket");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("Error in bind");
    exit(1);
  }

  if (listen(server_socket, 10) == 0) 
  {
    printf("Waiting for players...\n");
  } else {
    perror("Error in listen");
    exit(1);
  }

  addr_size = sizeof(client_addr);
  client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);

  // Set the flag to indicate that the code should run
  player_connected = true;

  *client_socket_ptr = client_socket;

  return NULL;
}

game_mode_e open_launcher(int *client_socket)
{
  // Initialization
  //--------------------------------------------------------------------------------------
  game_mode_e game_mode = MODE_NULL;
  launch_state_e state = LAUNCH_STATE_GAME_MODE;

  pthread_t connect_tid;

  int screen_width = 480;
  int screen_height = 640;

  InitWindow(screen_width, screen_height, "Bloons TD Launcher");

  // Loading the background
  Texture2D background = LoadTexture("./src/assets/launcher_bg.png");
  background.width = screen_width;
  background.height = screen_height;

  char *local_ip;
  struct ifaddrs *ifap, *ifa;
    
  if (getifaddrs(&ifap) == -1) {
    perror("getifaddrs");
    return -1;
  }

  for (ifa = ifap; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET) {
      struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
      char addr_str[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, &sa->sin_addr, addr_str, INET_ADDRSTRLEN);
      local_ip = addr_str;
    }
  }

  char text[256] = { 0 }; // Text input buffer
  int cursorPos = 0; // Cursor position

  //--------------------------------------------------------------------------------------

  while (!WindowShouldClose())
  {
    // Input
    //----------------------------------------------------------------------------------
    if (state == LAUNCH_STATE_SOCKET_JOIN)
    {
      // Check for keyboard input
      if (IsKeyPressed(KEY_ENTER))
      {
        int server_socket = 0;

        struct sockaddr_in server_addr;

        *client_socket = socket(AF_INET, SOCK_STREAM, 0);

        if (*client_socket < 0)
        {
          perror("Error in socket");
          exit(1);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(8080);
        server_addr.sin_addr.s_addr = inet_addr(text);

        if (connect(*client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
        {
          perror("Error in connect");
          exit(1);
        }

        game_mode = MODE_MULTIPLAYER;
        break;
      }
      if (IsKeyPressed(KEY_BACKSPACE) && cursorPos > 0)
      {
        memmove(&text[cursorPos - 1], &text[cursorPos], strlen(text) - cursorPos + 1);
        cursorPos--;
      }
      int key = GetKeyPressed();
      if (key >= 32 && key <= 125 && strlen(text) < 255)
      {
        memmove(&text[cursorPos + 1], &text[cursorPos], strlen(text) - cursorPos + 1);
        text[cursorPos] = (char)key;
        cursorPos++;
      }
    }

    if (state == LAUNCH_STATE_SOCKET_INIT)
    {
      if (IsMouseButtonPressed(0))
      {
        Rectangle create_lobby_button = {screen_width / 2 - 190, screen_height / 2 - 60 - 20, 380, 80};
        if (CheckCollisionPointRec(GetMousePosition(), create_lobby_button))
        {
          state = LAUNCH_STATE_SOCKET_CREATE;
          // Create the receive thread
          if (pthread_create(&connect_tid, NULL, connect_thread, client_socket) != 0)
          {
            perror("Error creating receive thread");
            exit(1);
          }
        }

        Rectangle join_lobby_button = {screen_width / 2 - 190, screen_height / 2 + 60 - 20, 380, 80};
        if (CheckCollisionPointRec(GetMousePosition(), join_lobby_button))
        {
          state = LAUNCH_STATE_SOCKET_JOIN;
        }
      }
    }

    if (state == LAUNCH_STATE_GAME_MODE)
    {
      if (IsMouseButtonPressed(0))
      {
        Rectangle singleplayer_button = {screen_width / 2 - 190, screen_height / 2 - 60 - 20, 380, 80};
        if (CheckCollisionPointRec(GetMousePosition(), singleplayer_button))
        {
          game_mode = MODE_SINGLEPLAYER;
          break;
        }

        Rectangle multiplayer_button = {screen_width / 2 - 190, screen_height / 2 + 60 - 20, 380, 80};
        if (CheckCollisionPointRec(GetMousePosition(), multiplayer_button))
        {
          state = LAUNCH_STATE_SOCKET_INIT;
        }
      }
    }
    //----------------------------------------------------------------------------------
    
    // Update
    //----------------------------------------------------------------------------------
    if (player_connected)
    {
      game_mode = MODE_MULTIPLAYER;
      break;
    }
    //----------------------------------------------------------------------------------
    
    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

      ClearBackground(RAYWHITE);
      DrawTexture(background, 0, 0, WHITE);

      if (state == LAUNCH_STATE_GAME_MODE)
      {
        DrawRectangle(screen_width / 2 - 190, screen_height / 2 - 60 - 20, 380, 80, (Color){255, 255, 255, 200});
        DrawText("Singleplayer", screen_width / 2 - MeasureText("Singleplayer", 28) / 2, screen_height / 2 - 60 - 20 + 22, 28, BLACK);
        
        DrawRectangle(screen_width / 2 - 190, screen_height / 2 + 60 - 20, 380, 80, (Color){255, 255, 255, 200});
        DrawText("Multiplayer", screen_width / 2 - MeasureText("Multiplayer", 28) / 2, screen_height / 2 + 60 - 20 + 22, 28, BLACK);
      }

      if (state == LAUNCH_STATE_SOCKET_INIT)
      {
        DrawRectangle(screen_width / 2 - 190, screen_height / 2 - 60 - 20, 380, 80, (Color){255, 255, 255, 200});
        DrawText("Create game", screen_width / 2 - MeasureText("Create game", 28) / 2, screen_height / 2 - 60 - 20 + 22, 28, BLACK);
        
        DrawRectangle(screen_width / 2 - 190, screen_height / 2 + 60 - 20, 380, 80, (Color){255, 255, 255, 200});
        DrawText("Join game", screen_width / 2 - MeasureText("Join game", 28) / 2, screen_height / 2 + 60 - 20 + 22, 28, BLACK);
      }

      if (state == LAUNCH_STATE_SOCKET_CREATE)
      {
        DrawText(TextFormat("IP Address: %s", local_ip), 0, 0, 22, WHITE);
        DrawText("Waiting for player to join...", 0, 36, 22, WHITE);
      }

      if (state == LAUNCH_STATE_SOCKET_JOIN)
      {
        DrawText("Ip Address", screen_width / 2 - MeasureText("Ip Address", 22) / 2, screen_height / 2 - 64, 22, WHITE);
        // Draw text input box
        DrawRectangle(screen_width / 2 - 180, screen_height / 2 - 20, 360, 40, RAYWHITE);
        DrawRectangleLines(screen_width / 2 - 180, screen_height / 2 - 20, 360, 40, BLACK);
        
        // Draw text
        DrawText(text, screen_width / 2 - MeasureText(text, 20) / 2, screen_height / 2 - 20 + 10, 20, BLACK);

        // Draw cursor
        int cursorX = MeasureText(text, 20);
        DrawLine(screen_width / 2 - MeasureText(text, 20) / 2 + cursorX, screen_height / 2 - 20 + 10, screen_width / 2 - MeasureText(text, 20) / 2 + cursorX, screen_height / 2 - 20 + 10 + 20, BLACK);
      }

    EndDrawing();
    //----------------------------------------------------------------------------------
  }

  UnloadTexture(background);

  CloseWindow();
  return game_mode;
}