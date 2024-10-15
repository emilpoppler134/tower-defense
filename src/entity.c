#include "include/entity.h"

entity_t init_entity()
{
  entity_t entity = {
    .position = (Vector2){0, 0},
    .direction = (Vector2){0, 0},
    .speed = 0,
    .rotation = 0,
    .type = 0,
    .radius = 0,
    .cost = 0,
    .interval = (time_interval_t){
      .interval = 0,
      .last_time = 0
    }
  };

  return entity;
}

dynamic_entity_array init_entity_array()
{
  dynamic_entity_array arr;
  arr.data = (entity_t*)malloc(sizeof(entity_t));
  arr.count = 0;
  arr.capacity = 1;
  return arr;
}

void push(dynamic_entity_array *arr, entity_t item)
{
  if (arr->count == arr->capacity)
  {
    arr->capacity++;
    arr->data = (entity_t*)realloc(arr->data, arr->capacity * sizeof(entity_t));
  }
  arr->data[arr->count] = item;
  arr->count++;
}

void remove_at(dynamic_entity_array *arr, int index)
{
  if (index < 0 || index >= arr->count)
  {
    return;
  }

  entity_t* new_arr = (entity_t*)malloc((arr->capacity - 1) * sizeof(entity_t));
  if (!new_arr)
  {
    exit(1);
    return;
  }

  int new_arr_index = 0;

  for (int i = 0; i < arr->count; i++)
  {
    if (i != index)
    {
      new_arr[new_arr_index] = arr->data[i];
      new_arr_index++;
    }
  }

  free(arr->data);
  arr->data = new_arr;
  arr->count--;
  arr->capacity--;
}

// Comparison function for qsort
int compare(const void* a, const void* b) {
  const entity_t* entityA = (const entity_t*)a;
  const entity_t* entityB = (const entity_t*)b;

  // Compare based on the 'type' field
  return entityA->type - entityB->type;
}

void sort(dynamic_entity_array* entities) {
  // Check if there are entities to sort
  if (entities->count <= 0) {
    return; // Nothing to sort
  }

  // Use qsort to sort the entities based on 'type'
  qsort(entities->data, entities->count, sizeof(entity_t), compare);
}

// Resources
void deserialize_entities(dynamic_entity_array *entities, char* entity_type)
{
  DIR *directory;
  struct dirent *entry;

  // Open the directory
  directory = opendir(TextFormat("./src/assets/%s", entity_type));

  if (directory == NULL) {
    perror("Error opening directory");
    exit(1);
  }

  // Read each entry in the directory
  while ((entry = readdir(directory)) != NULL)
  {
    if (entry->d_type == DT_REG) // Check if it's a regular file
    {
      entity_t entity;

      FILE *file = fopen(TextFormat("./src/assets/%s/%s", entity_type, entry->d_name), "rb");
      if (!file)
      {
        perror("Failed to open level file");
        exit(1);
      }

      if (fread(&entity, sizeof(entity_t), 1, file) != 1)
      {
        perror("Failed to read level data");
        fclose(file);
        exit(1);
      }

      push(entities, entity);

      fclose(file);
    }
  }

  sort(entities);

  // Close the directory
  closedir(directory);
}