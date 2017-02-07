#ifndef GROUP_H_
#define GROUP_H_ 1

/** group of similar images */
typedef struct group_t {
  struct group_t *next; /**< pointer to next group in list */
  int num;  /**< group number */
  int size; /**< images count in this group */
  int caps; /**< allocated slots for images */
  int *ids; /**< images ids */
} group_t;

/**
 * @brief Creates new image group
 * @param num  Group number
 * @param size Initial preallocated slots
 * @returns Pointer to new group of NULL on error
 */
group_t * group_create(int num, int size);

/**
 * @brief Append new image id to given group
 * @param group Pointer to group struct
 * @param id Image id to add (dublicates will be ignored)
 * @returns true on success, false on error
 */
bool group_append(group_t *group, int id);

/**
 * @brief Remove all group members and free allocated memory
 * @param group Pointer to group struct
 */
void group_free(group_t *group);

#endif /* GROUP_H_ */
