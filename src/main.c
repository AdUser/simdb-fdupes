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
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ftw.h>
#include <getopt.h>

#include <magic.h>
#include <simdb.h>

#include "filelist.h"
#include "group.h"

/* opts */
enum msglevel { normal = 0, verbose, debug } msglevel = normal;
const char *root = NULL;

/* vars */
magic_t mcookie;
filelist_t *flist;

/* funcs */
static void
usage(int exitcode) {
  fprintf(stderr,
    "Usage: simdb-fdupes [path]\n"
    "  -h         This help\n"
    "  -d <int>   Max difference in images (in percents: 0-50)\n"
    "  -v         Verbose messages\n"
  );
  exit(exitcode);
}

static void
log_msg(enum msglevel l, const char *fmt, ...) {
  va_list ap;

  if (l > msglevel)
    return;

  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}

static int
progressbar(const char *prefix, int width, int num, int total, int lastpct) {
  char bar[width + 1];
  int pct = 0, fill = 0;

  if (width <= 0 || total <= 0)
    return 0;

  pct = 100 * ((float) num / total);
  if (pct == lastpct)
    return lastpct;

  if (pct == 100) {
    lastpct = pct;
  }

  fill = (int) ((pct * width) / 100);
  memset(bar, ' ', sizeof(char) * width);
  memset(bar, '=', sizeof(char) * fill);
  bar[fill]  = '>';
  bar[width] = '\0';
  log_msg(verbose, "\r* %s: % 3d%% [%s]", prefix, pct, bar);

  return pct;
}

static int
ftw_handler(const char *path, const struct stat *sb, int typeflag) {
  const char *mime = NULL;
  (void)(sb); /* unused */
  if (typeflag == FTW_D)
    return 0;
  if (typeflag == FTW_DNR) {
    fprintf(stderr, "! can't read: %s\n", path);
    return 0;
  }
  if (typeflag == FTW_NS) {
    fprintf(stderr, "! can't stat: %s\n", path);
    return 0;
  }
  mime = magic_file(mcookie, path);
  if (mime == NULL) {
    fprintf(stderr, "! can't detect mimetype of file %s\n", path);
    return 0;
  }
  if (strncmp(mime, "image/", 6) != 0)
    return 0; /* not an image */
  log_msg(debug, "~ found image file: %s\n", path);
  if (!filelist_append(flist, path)) {
    fprintf(stderr, "! can't add file to queue: out of memory");
    return -1; /* stop ftw() */
  }
  return 0;
}

static void
make_samples(filelist_t *list, simdb_t *simdb) {
  const char *path;
  int ret = 0, pct = 0;

  assert(list != NULL);
  assert(simdb != NULL);

  for (int num = 1; num <= list->size; num++) {
    path = filelist_get(list, num);
    pct = progressbar("making samples", 50, num, list->size, pct);
    ret = simdb_record_add(simdb, num, path, 0);
    if (ret < 0) {
      fprintf(stderr, "\r! can't add file #%d '%s' -- %s\n", num, path, simdb_error(ret));
      simdb_record_del(simdb, num);
      filelist_del(flist, num);
    }
  }
  log_msg(verbose, "\n"); /* force newline after progress messages */

  return;
}

static group_t *
make_groups(filelist_t *list, simdb_t *simdb, int maxdiff) {
  simdb_search_t search;
  group_t *groups = NULL, *group, **map = NULL;
  int pct = 0, inum, gnum = 1; /* next group number */

  assert(list != NULL);
  assert(simdb != NULL);

  if ((map = calloc(flist->size + 1, sizeof(group_t *))) == NULL) {
    fprintf(stderr, "! can't allocate groups map: out-of-memory\n");
    return NULL;
  }

  simdb_search_init(&search);
  search.d_ratio  = maxdiff / (float) 100;
  search.d_bitmap = maxdiff / (float) 100;
  for (int num = 1; num < list->size; num++) {
    if (!filelist_get(flist, num))
      continue; /* file was not sampled */
    if (map[num])
      continue; /* this image already in some group */
    pct = progressbar("grouping images", 50, num, list->size, pct);
    simdb_search_byid(simdb, &search, num);
    if (search.found <= 0)
      continue; /* nothing similar found in database */
    group = NULL;
    /* try to find existing group */
    for (int i = 0; i < search.found; i++) {
      inum = search.matches[i].num;
      if (map[inum] == NULL)
        continue;
      /* found some group */
      group = map[inum];
      break;
    }
    /* create new group if not found any */
    if (!group) {
      if ((group = group_create(gnum++, 0)) == NULL) {
        fprintf(stderr, "\n! can't create new image group: out-of-memory\n");
        break;
      }
      group->next = groups;
      groups = group;
    }
    group_append(group, num);
    /* place in map pointer to group for each found image */
    for (int i = 0; i < search.found; i++) {
      inum = search.matches[i].num;
      group_append(group, inum);
      map[inum] = group;
    }
  }
  log_msg(verbose, "\n"); /* force newline after progress messages */

  simdb_search_free(&search);
  free(map);

  return groups;
}

static void
print_groups(filelist_t *list, group_t *groups) {
  int inum = 0;

  assert(list != NULL);

  for (group_t *group = groups; group != NULL; group = group->next) {
    for (int i = 0; i < group->size; i++) {
      inum = group->ids[i];
      puts(filelist_get(list, inum));
    }
    puts(""); /* force newline after group */
  }
}

static void
free_groups(group_t *groups) {
  group_t *group = groups, *next = NULL;

  while (group != NULL) {
    next = group->next;
    group_free(group);
    free(group);
    group = next;
  }
}

int main(int argc, char **argv) {
  simdb_t *simdb = NULL;
  group_t *groups = NULL;
  char tempdb[] = "/tmp/simdb-XXXXXX";
  char path[PATH_MAX] = "";
  int opt = -1, ret = 0, maxdiff = 7;

  if (argc <= 1)
    usage(EXIT_FAILURE);

  while ((opt = getopt(argc, argv, "hd:v")) != -1) {
    switch (opt) {
      case 'v':
        if (msglevel < debug)
          msglevel++;
        break;
      case 'd':
        maxdiff = atoi(optarg);
        break;
      case 'h':
        usage(EXIT_SUCCESS);
        break;
      default :
        usage(EXIT_FAILURE);
        break;
    }
  }

  if (optind < argc) {
    root = argv[optind];
  } else {
    usage(EXIT_FAILURE);
  }

  if (maxdiff < 0 || maxdiff > 50) {
    fprintf(stderr, "! '-d' option should be in range [0, 50]\n");
    return EXIT_FAILURE;
  }

  /* resolve root path */
  if (realpath(root, path) == NULL) {
    perror("Can't resolve given path");
    exit(EXIT_FAILURE);
  }

  /* load magic database */
  if ((mcookie = magic_open(MAGIC_MIME_TYPE)) == NULL) {
    perror("can't open magic database");
    return EXIT_FAILURE;
  }
  if (magic_load(mcookie, NULL) < 0) {
    fprintf(stderr, "! can't load magic database: %s\n", magic_error(mcookie));
    return EXIT_FAILURE;
  }

  /* make images filelist */
  if ((flist = filelist_create(1000)) == NULL) {
    fprintf(stderr, "! can't create filelist struct: out-of-memory?\n");
    return EXIT_FAILURE;
  }
  log_msg(verbose, "* scanning for images\n");
  if (ftw(path, &ftw_handler, 20) < 0) {
    fprintf(stderr, "! ftw() error, aborting\n");
    return EXIT_FAILURE;
  } else {
    log_msg(verbose, "* found %d images after initial scan\n", flist->size);
  }
  magic_close(mcookie);

  mkstemp(tempdb);
  unlink(tempdb);

  if (!simdb_create(tempdb)) {
    fprintf(stderr, "! can't create temporary simdb\n");
    return EXIT_FAILURE;
  }

  if ((simdb = simdb_open(tempdb, SIMDB_FLAG_WRITE | SIMDB_FLAG_LOCKNB, &ret)) == NULL) {
    fprintf(stderr, "! can't open temporary simdb: %s\n", simdb_error(ret));
    return EXIT_FAILURE;
  }

  make_samples(flist, simdb);

  groups = make_groups(flist, simdb, maxdiff);

  if (groups) {
    log_msg(verbose, "* found image groups:\n");
    print_groups(flist, groups);
    free_groups(groups);
  }

  simdb_close(simdb);
  unlink(tempdb);

  filelist_free(flist);
  free(flist);

  return EXIT_SUCCESS;
}
