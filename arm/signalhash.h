#pragma once

#include "signals.h"

#define HASH_TABLE_SIZE	1024

struct hash_item_s {
	int idx;
	char name[150];
	unsigned hash_value;
	struct hash_item_s *next;
};

struct hash_s {
	struct hash_item_s *hash_table[HASH_TABLE_SIZE];
};

struct hash_s *hash_create(struct hash_s **hash);
void   hash_destroy(struct hash_s **hash);
struct hash_item_s *hash_add(struct hash_s *hash, struct Signal *signal_array, int idx);
struct hash_item_s *hash_del(struct hash_s *hash, struct Signal *signal_array, int idx);
struct Signal *hash_find(struct hash_s *hash, struct Signal *signal_array, char *name);

struct hash_item_s *hash_add_by_prefix(struct hash_s *hash, struct Signal *signal_array, int idx);
struct hash_item_s *hash_del_by_prefix(struct hash_s *hash, struct Signal *signal_array, int idx);
struct hash_item_s *hash_find_by_prefix(struct hash_s *hash, char *name);
