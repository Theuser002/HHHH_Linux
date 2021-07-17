#include "inventory.h"

int *readInventoryInfo(int max_drink)
{
	// printf("MAX DRINK: %d\n", max_drink);
	int *countPerDrink = (int *)malloc(3 * max_drink * sizeof(int));
	if (countPerDrink == NULL)
	{
		throwMallocException();
	}
	int quantity;
	char machine[100], brand[100];
	FILE *f = fopen(INVENTORY_INFO_FILENAME, "r+");
	if (f == NULL)
	{
		printf("Error opening inventory info file!\n");
		for (int i = 0; i < 3 * max_drink; i++){
			countPerDrink[i] = 0;
		}
	}else{
		int i = 0;
		while (!feof(f))
		{
			fscanf(f, "%s\t%s\t%d\n", machine, brand, &quantity);
			// printf("TEST: %d: %s-%s-%d\n",i, machine, brand, quantity);
			countPerDrink[i] = quantity;
			i++;
		}
		fclose(f);
	}
	return countPerDrink;
}

void writeInventoryInfo(drink *all_drink, int max_drink, int *figures)
{
	char *client_str[] = {"VM1", "VM2", "VM3"};
	FILE *f = fopen(INVENTORY_INFO_FILENAME, "w");
	int i = 0;
	printf("Quantities of all drinks after bought: ");
	while (i < max_drink * 3)
	{
		// printf("Quantity: %d\n", figures[i]);
		int may = i / 3;
		int nhan = i % 3;
		fprintf(f, "%s\t%s\t%d\n", client_str[may], all_drink[nhan].brand, figures[i]);
		i++;
	}
	fclose(f);
}

// void writeInventoryInfo(client_info *clt, int* figures){
//   FILE *f = fopen(INVENTORY_INFO_FILENAME, "w");
//   int i = 0;

//   while(i < max_drink){
//     fprintf(f, "%s\t%d\n", no2brand(all_drink, max_drink, i), figures[i]);
//     i++;
//   }
//   fclose(f);
// }
