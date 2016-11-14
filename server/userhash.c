#include "userhash.h"

#include <string.h>
#include <stdlib.h>

size_t _userhash_hash(const char *user) {
	int h = 0;
	do {
		h += *user;
	} while (*(++user));

	return h;
}

userhash userhash_create() {
	userhash users = { USERHASH_SZ, 0, NULL };
	users.table = calloc(users.size, sizeof(userinfo *));
	return users;
}

void userhash_free(userhash users) {
	userinfo **row;
	userinfo *user;
	userinfo *next;

	for (row = users.table; row < users.table + users.size; row++) {
		if (*row) {
			user = *row;
			do {
				next = user->next;
				free(user);
				user = next;
			} while (user);
		}
	}
	free(users.table);
}	


userinfo *userhash_find(userhash users, const char *name) {
	userinfo *user = users.table[_userhash_hash(name) % users.size];
	while (user && strcmp(user->name, name)) {
		user = user->next;
	}
	return user;
}

void userhash_add(userhash users, const char *name, const char *pass) {
	userinfo *newuser = malloc(sizeof(userinfo));
	strncpy(newuser->name, name, USERHASH_STRLEN);
	strncpy(newuser->pass, pass, USERHASH_STRLEN);
	
	int hash = _userhash_hash(name) % users.size;
	userinfo *user = users.table[hash];
	newuser->next = user;
	users.table[hash] = newuser;
}
