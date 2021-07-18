#include "drink.h"

<<<<<<< HEAD
void readDrinkInfo(drink all_drink[20], int *max_drink)
{
	FILE *f = fopen("drink.txt", "r");
	int i = 0;
	while (!feof(f))
	{
		fscanf(f, "%d %s %d\n",
			   &all_drink[i].no,
			   all_drink[i].brand,
			   &all_drink[i].price);
=======
void readDrinkInfo(drink all_drink[20], int *max_drink) {
	FILE *f = fopen("drink.txt", "r");
	int i = 0;
	while(!feof(f)) {
		fscanf(f, "%d %s %d\n",
				&all_drink[i].no,
				all_drink[i].brand,
				&all_drink[i].price);
>>>>>>> 21b86c0812b79a6f059ad077f71499eb55ad30d3
		i++;
	}

	*max_drink = i;
}

<<<<<<< HEAD
char *no2brand(drink all_drink[20], int max_drink, int no)
{
	int i = 0;
	for (; i < max_drink; i++)
	{
		if (all_drink[i].no == no)
		{
			return all_drink[i].brand;
		}
	}

	return NULL;
}

int brand2no(drink *all_drink, int max_drink, char *drink)
{
	int i = 0;
	for (; i < max_drink; i++)
	{
		if (strcmp(all_drink[i].brand, drink) == 0)
		{
=======


char * no2brand(drink all_drink[20],int max_drink, int no) {
	int i = 0;
	for (; i < max_drink; i++) {
		if (all_drink[i].no == no) {
			return all_drink[i].brand;
		}
	}
	
	return NULL;
}

int brand2no(drink* all_drink,int max_drink, char* drink) {
	int i = 0;
	for (; i < max_drink; i++) {
		if (strcmp(all_drink[i].brand, drink) == 0) {
>>>>>>> 21b86c0812b79a6f059ad077f71499eb55ad30d3
			return all_drink[i].no;
		}
	}
	return -1;
}
