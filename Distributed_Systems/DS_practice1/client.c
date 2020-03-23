#include "message.h"
#include "array.h"
#include <stdio.h>
#include <string.h>
#include <string.h>

#define MAXSIZE 256


int main() {
	/* Defining error messages */
	char denay_create[MAXSIZE] = "The server could not create the vector\n";
	char denay_set[MAXSIZE] = "The server could not update the selected position\n";
	char denay_get[MAXSIZE] = "The server could not get the value from the selected position\n";
	char denay_close[MAXSIZE] = "The server could not destroy the vector\n";
	

	if (init("vector1", 0) == -1)
		printf(denay_create);
	
	char names[20];
	for (int i = 0; i < 101; i++) {
		sprintf(names, "vector%d", i);
		if (init(names, i) == -1)
			printf(denay_create);
	}

	if (init("vector3", 100) == -1)
		printf(denay_create);

	if (set("vector1", 0, -40) == -1)
		printf(denay_set);
	if (set("vector2", 120, 30) == -1)
		printf(denay_set);
	int ret;
	if (get("vector1", 0, &ret) == -1)
		printf(denay_get);
	printf("vector1[0] = %d\n", ret);

	if (destroy("vector50") == -1)
		printf(denay_close);
	if (destroy("vector70") == -1)
		printf(denay_close);
	// This new vector should be created in position 50 rather than 70
	if (init("whatever", 100))
		printf(denay_create);
		
	if (destroy("vector1") == -1)
		printf(denay_close);
	if (destroy("vector2") == -1)
		printf(denay_close);
	if (destroy("vector3") == -1)
		printf(denay_close);
		
	return(0);
}
