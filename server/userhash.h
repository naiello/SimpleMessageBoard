#ifndef USER_HASH_H_
#define USER_HASH_H_

#define USERHASH_SZ 10
#define USERHASH_STRLEN 20

#include <stddef.h>

typedef struct userinfo_t {
	char name[USERHASH_STRLEN];
	char pass[USERHASH_STRLEN];
	struct userinfo_t *next;
} userinfo;

typedef struct userhash_t {
	size_t size;
	size_t items;
	userinfo **table;
} userhash;

userhash userhash_create();
userinfo *userhash_find(userhash, const char *);
void userhash_add(userhash, const char *, const char *);
void userhash_free(userhash);

#endif
