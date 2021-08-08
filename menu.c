#include "menu.h"

void menu_home(int* figures) {
	printf(	"---------------------------\n"	);
	printf(	"--          Menu:        --\n"	);
	printf(	"-- 1. Coke           %3d --\n"  
			"-- 2. Water          %3d --\n"
			"-- 3. Milk           %3d --\n"
			"-- 0. Quit               --\n", figures[0], figures[1], figures[2]);
	printf(	"---------------------------\n"	);
	printf(	"Your choice: "	);
}

void menu_bye() {
	printf("Bye\n");
}
