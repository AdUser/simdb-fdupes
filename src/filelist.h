#ifndef FILELIST_H_
#define FILELIST_H_ 1

/** Filelist of potential images */
typedef struct {
  int size; /**< files in list */
  int caps; /**< allocated slots count */
  char **files; /**< Path's array */
} filelist_t;

/**
 * @brief Create new filelist
 * @param size Preallocated list slots count
 * @returns Pointer to created list or NULL on error
 */
filelist_t * filelist_create(int size);

/**
 * @brief Get image path by id
 * @param list Pointer to filelist
 * @param num  Image number
 * @returns Pointer to image path or NULL if slot is empty
 */
const char * filelist_get (filelist_t *list, int num);

/**
 * @brief Add new path to filelist
 * @param list Pointer to filelist
 * @param path Path to image
 * @returns 0 on error or >0 on success as slot number
 */
int filelist_append (filelist_t *list, const char *path);

/**
 * @brief Replace given slot with new path
 * @param list Pointer to filelist
 * @param num  Image number
 * @param path Path to image
 * @returns true on success, false on error
 */
bool filelist_set (filelist_t *list, int num, const char *path);

/**
 * @brief Remove path with given slot number
 * @param list Pointer to filelist
 * @param num  Image number
 */
void filelist_del (filelist_t *list, int num);

/**
 * @brief Remove all filelist members and free memory
 * @param list Pointer to filelist
 * @note memory for filelist_t struct not freed
 */
void filelist_free(filelist_t *list);

#endif /* FILELIST_H_ */
