/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "filelist.h"

#define FREE(x) free((x)), (x) = NULL;

static const int grow_step = 100;

static bool
filelist_grow(filelist_t *list, int size) {
  char **p = NULL;

  assert(list != NULL);

  if (size <= 0)
    return false;
  if (size <= list->caps)
    return false;

  if ((p = realloc(list->files, size * sizeof(char *))) == NULL)
    return false;

  list->files = p,
  list->caps  = size;

  return true;
}

filelist_t *
filelist_create(int size) {
  filelist_t *tmp = NULL;

  if ((tmp = calloc(1, sizeof(filelist_t))) == NULL)
    return NULL;

  if (size == 0)
    return tmp;

  if (!filelist_grow(tmp, size))
    FREE(tmp);

  return tmp;
}

const char *
filelist_get(filelist_t *list, int num) {
  assert(list != NULL);

  if (num < 1 || num > list->size)
    return NULL;

  return list->files[num - 1];
}

int
filelist_append(filelist_t *list, const char *path) {
  assert(list != NULL);

  if (list->size >= list->caps) {
    int newcaps = list->caps ? list->caps * 2 : grow_step;
    if (!filelist_grow(list, newcaps))
      return 0;
  }

  if ((list->files[list->size] = strdup(path)) == NULL)
    return 0;

  list->size++;
  return list->size;
}

bool
filelist_set(filelist_t *list, int num, const char *path) {
  char *p = NULL;
  assert(list != NULL);

  if (num-- < 1)
    return false;

  if (num >= list->caps) {
    if (!filelist_grow(list, num + grow_step))
      return false;
  }

  if (path == NULL) {
    FREE(list->files[num]);
    return true;
  }

  if ((p = strdup(path)) == NULL)
    return false;

  FREE(list->files[num]);
  list->files[num] = p;
  return true;
}

void
filelist_del(filelist_t *list, int num) {
  assert(list != NULL);

  if (num-- < 1)
    return;
  if (num > list->size)
    return;
  if (list->files[num] == NULL)
    return;
  FREE(list->files[num]);
  if ((num + 1) == list->size)
    list->size--;
  return;
}

void
filelist_free(filelist_t *list) {
  assert(list != NULL);

  for (int i = 0; i < list->size; i++) {
    if (list->files[i] == NULL)
      continue;
    FREE(list->files[i]);
  }
  FREE(list->files);
  list->size = 0;
  list->caps = 0;
  return;
}
