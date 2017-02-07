#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/group.h"

int main() {
  group_t *group;
  bool ret;

  group = group_create(7, 0);
  assert(group != NULL);
  assert(group->num == 7);
  assert(group->ids == NULL);
  assert(group->caps == 0);
  assert(group->size == 0);

  ret = group_append(group, 17);
  assert(ret == true);
  assert(group->size == 1);
  assert(group->caps >= group->size);
  assert(group->ids != NULL);

  ret = group_append(group, 19);
  assert(ret == true);
  assert(group->size == 2);

  ret = group_append(group, 21);
  assert(ret == true);
  assert(group->size == 3);

  ret = group_append(group, 17); /* duplicate */
  assert(ret == true);
  assert(group->size == 3);

  /* trigger group expand */
  for (int i = group->caps, n = 46; i >= 0; i--, n++)
    group_append(group, n);

  group_free(group);
  assert(group->size == 0);
  assert(group->caps == 0);

  free(group);

  return EXIT_SUCCESS;
}
