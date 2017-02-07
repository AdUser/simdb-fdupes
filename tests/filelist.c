#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/filelist.h"

int main() {
  filelist_t *list = NULL;
  int ret = 0;

  list = filelist_create(0);

  assert(list != NULL);
  assert(list->size == 0);
  assert(list->caps == 0);
  assert(list->files == NULL);

  ret = filelist_append(list, "test");
  assert(ret == 1);
  assert(list->size == 1);
  assert(list->caps >= list->size);
  assert(list->files != NULL);
  assert(strcmp(list->files[0], "test") == 0);

  filelist_del(list, 1);
  assert(list->size == 0);
  assert(list->caps >= list->size);

  /* trigger list expand */
  for (int i = list->caps; i >= 0; i--) {
    ret = filelist_append(list, "test");
    assert(ret > 0);
  }

  filelist_set(list, 17, "test2");
  filelist_set(list, 17, "test3");

  filelist_free(list);
  assert(list->size == 0);
  assert(list->caps == 0);
  free(list);

  return EXIT_SUCCESS;
}
