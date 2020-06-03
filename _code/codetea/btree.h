#ifndef __BTREE__
#define __BTREE__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * non-recursion
 * non-thread safe
 */

struct btree;

/**
 * return value
 *
 * 0: equal
 * -1: v1 < v2
 * 1: v1 > v2
 */
typedef int (*compare)(void *v1, void *v2);
typedef void (*dump)(void *value);

struct btree *btree_new(void);
void btree_delete(struct btree *btree);
unsigned int btree_height(struct btree *btree);
unsigned int btree_node_count(struct btree *btree);

int btree_insert(struct btree *btree, void *value, compare cmp);
int btree_remove(struct btree *btree, void *value, compare cmp);

int btree_max(struct btree *btree, void **value);
int btree_min(struct btree *btree, void **value);
int btree_search(struct btree *btree, void *value, compare cmp);

void btree_in_order(struct btree *btree, dump dmp);

/**
 * printf tree shape
 */
void btree_bfs_dump(struct btree *btree, dump dmp);

#ifdef __cplusplus
}
#endif
#endif