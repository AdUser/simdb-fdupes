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

#include "group.h"

#define FREE(x) free((x)), (x) = NULL;

static const int grow_step = 10;

static bool
group_grow(group_t *group, int size) {
  int *p = NULL;

  assert(group != NULL);

  if (size <= 0)
    return false;
  if (size <= group->caps)
    return false;

  if ((p = realloc(group->ids, size * sizeof(int))) == NULL)
    return false;

  group->ids  = p,
  group->caps = size;

  return true;
}

group_t *
group_create(int num, int size) {
  group_t *tmp = NULL;

  if ((tmp = calloc(1, sizeof(group_t))) == NULL)
    return NULL;
  tmp->num = num;

  if (size == 0)
    return tmp;

  if (!group_grow(tmp, size))
    FREE(tmp);

  return tmp;
}

bool
group_append(group_t *group, int id) {
  assert(group != NULL);

  for (int i = 0; i < group->size; i++) {
    if (group->ids[i] == id)
      return true;
  }

  if (group->size >= group->caps) {
    int newcaps = group->caps ? group->caps * 2 : grow_step;
    if (!group_grow(group, newcaps))
      return false;
  }

  group->ids[group->size] = id,
  group->size++;

  return true;
}

void
group_free(group_t *group) {
  assert(group != NULL);

  FREE(group->ids);
  group->size = 0,
  group->caps = 0;
}
