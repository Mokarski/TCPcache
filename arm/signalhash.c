#include <stdlib.h>
#include <string.h>
#include "signalhash.h"

unsigned hash_function(char *str, size_t size) {
	int len = (size == 0 ? strlen(str) : size);
	unsigned i, hash = 7;
	for (i = 0; i < len; i++) {
		hash = (hash * 31 + str[i]) % HASH_TABLE_SIZE;
	}
	return hash;
}

struct hash_s *hash_create(struct hash_s **hash) {
	*hash = malloc(sizeof(struct hash_s));
	memset((*hash)->hash_table, 0, sizeof(void*) * HASH_TABLE_SIZE);
	return *hash;
}

void hash_destroy(struct hash_s **hash) {
	if(*hash) {
		free(*hash);
		*hash = NULL;
	}
}

struct hash_item_s *hash_add(struct hash_s *hash, struct Signal *signal_array, int idx) {
	struct hash_item_s *item = malloc(sizeof(struct hash_item_s));
	item->idx = idx;
	item->hash_value = hash_function(signal_array[idx].Name, 0);
	item->next = NULL;
	if(hash->hash_table[item->hash_value]) {
		item->next = hash->hash_table[item->hash_value];
	}
	hash->hash_table[item->hash_value] = item;
}

struct hash_item_s *hash_del(struct hash_s *hash, struct Signal *signal_array, int idx) {
	// I don't want to delete
}

struct Signal *hash_find(struct hash_s *hash, struct Signal *signal_array, char *name) {
	int function = hash_function(name, 0);
	struct hash_item_s *first_item = hash->hash_table[function];
	while(first_item) {
		struct Signal *s = &signal_array[first_item->idx];
		if(!strcmp(s->Name, name)) {
			return s;
		}
		first_item = first_item->next;
	}
	return NULL;
}

struct hash_item_s *hash_add_by_prefix(struct hash_s *hash, struct Signal *signal_array, int idx) {
	struct hash_item_s *item = malloc(sizeof(struct hash_item_s));
	size_t dot = strchr(signal_array[idx].Name, '.') - signal_array[idx].Name;
	int i;
	item->idx = idx;
	item->hash_value = hash_function(signal_array[idx].Name, dot);
	item->next = NULL;
	if(hash->hash_table[item->hash_value]) {
		item->next = hash->hash_table[item->hash_value];
	}
	hash->hash_table[item->hash_value] = item;
	return item;
}

struct hash_item_s *hash_del_by_prefix(struct hash_s *hash, struct Signal *signal_array, int idx) {
}

struct hash_item_s *hash_find_by_prefix(struct hash_s *hash, char *name) {
	size_t dot = strchr(name, '.') - name;
	int function = hash_function(name, dot);
	struct hash_item_s *first_item = hash->hash_table[function];
	return first_item;
}
