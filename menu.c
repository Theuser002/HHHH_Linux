#include "menu.h"

void menu_home(int* figures) {
	printf(	"---------------------------\n"	);
	printf(	"--          Menu:        --\n"	);
	printf(	"-- 1. Pepsi          %3d --\n"  
			"-- 2. Coca           %3d --\n"
			"-- 3. Lala           %3d --\n"
			"-- 0. Quit               --\n", figures[0], figures[1], figures[2]);
	printf(	"---------------------------\n"	);
	printf(	"Your choice: "	);
}

void menu_bye() {
	printf("Bye\n");
}
