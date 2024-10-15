#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "raylib.h"
#include "entity.h"

typedef enum action_e
{
  ACTION_NULL,
  ACTION_PLACE,
  ACTION_REMOVE,
  ACTION_START_GAME,
} action_e;

typedef struct package_t
{
  action_e action;
  entity_t entity;
  int index;
} package_t;

typedef struct socket_t
{
  int server_socket;
  int client_socket;
} socket_t;

package_t init_package();

#endif