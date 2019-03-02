
#include "otter_player.h"

unsigned char * test;

int main()
{

	test = (unsigned char *)calloc(5, sizeof(unsigned char));
	test[0] = 0x1;
	test[1] = 0xff;
	test[2] = 0x33;

	printf("%#X\n", test[0]);
	printf("%#X\n", test[1]);
	printf("%#X\n", test[2]);
	printf("%#X\n", test[3]);

	unsigned char * bak = test;
	printf("bak%#X\n", bak[0]);
	printf("bak%#X\n", bak[1]);
	printf("bak%#X\n", bak[2]);
	printf("bak%#X\n", bak[3]);
	
	test = (unsigned char *)calloc(5, sizeof(unsigned char));
	printf("bak%#X\n", bak[0]);
	printf("bak%#X\n", bak[1]);
	printf("bak%#X\n", bak[2]);
	printf("bak%#X\n", bak[3]);
	printf("%#X\n", test[0]);
	printf("%#X\n", test[1]);
	printf("%#X\n", test[2]);
	printf("%#X\n", test[3]);

	fileRead("C:\\1.ts"); 
	   
	system("pause");
	return 0;
}

int fileRead(char *filePath)
{
	printf("%s\n", filePath);
	FILE *tsFile = NULL;

	if ((tsFile = fopen(filePath, "rb")) == NULL)
	{
		printf("file not exist!\n");
	}

	unsigned char pkt[188];

	while (fread(pkt, 188, 1, tsFile) == 1) 
	{
		receive_ts_packet(pkt);
	}
	   	 
	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}

	return 0;
}