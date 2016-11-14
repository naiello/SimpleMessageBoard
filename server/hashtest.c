#include <stdio.h>

#include "userhash.h"

int main()
{
	userinfo *info;
	userhash users = userhash_create();
	userhash_add(users, "user1", "pass1");
	userhash_add(users, "user2", "pass2");
	userhash_add(users, "useq3", "pass1"); // hashes to same location as user2
	if ((info = userhash_find(users, "user1"))) {
		printf("user1: %s\n", info->pass);
	} else {
		printf("user1: NOT FOUND\n");
	}
	if ((info = userhash_find(users, "user2"))) {
		printf("user2: %s\n", info->pass);
	} else {
		printf("user2: NOT FOUND\n");
	}
	if ((info = userhash_find(users, "useq3"))) {
		printf("useq3: %s\n", info->pass);
	} else {
		printf("useq3: NOT FOUND\n");
	}
	if ((info = userhash_find(users, "user4"))) { // doesn't exist
		printf("user4: %s\n", info->pass);
	} else {
		printf("user4: NOT FOUND\n");
	}
	userhash_free(users);

	return 0;
}
