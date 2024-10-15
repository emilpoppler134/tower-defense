#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

#include "raylib.h"
#include "interval.h"

typedef struct entity_t
{
  // Entity
  Vector2 position;
  Vector2 direction;
  float speed;
  float rotation;
  int type;
  int radius;
  int cost;
  time_interval_t interval;
} entity_t;

typedef struct dynamic_entity_array
{
  entity_t* data;
  int count;
  int capacity;
} dynamic_entity_array;


entity_t init_entity();
dynamic_entity_array init_entity_array();

void push(dynamic_entity_array *arr, entity_t item);
void remove_at(dynamic_entity_array *arr, int index);

// Resources
void deserialize_entities(dynamic_entity_array *entities, char* entity_type);

#endif