#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <pthread.h>

#include "server.h"
#include "raylib.h"

typedef enum game_mode_e
{
  MODE_NULL,
  MODE_SINGLEPLAYER,
  MODE_MULTIPLAYER,
  MODE_NUM
} game_mode_e;

typedef enum launch_state_e
{
  LAUNCH_STATE_GAME_MODE,
  LAUNCH_STATE_SOCKET_INIT,
  LAUNCH_STATE_SOCKET_CREATE,
  LAUNCH_STATE_SOCKET_JOIN,
  LAUNCH_STATE_NUM,
} launch_state_e;

game_mode_e open_launcher(int *client_socket);

#endif
