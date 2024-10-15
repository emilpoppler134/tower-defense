#include "include/server.h"

package_t init_package()
{
  package_t package = {
    .action = ACTION_NULL,
    .entity = init_entity()
  };
  return package;
}