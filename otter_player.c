
#include "otter_player.h"

int main()
{

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
		ireceive_ts_packet(pkt);
	}
	   
	test_icld();
	   	 
	if (fclose(tsFile))
	{
		printf("The file was not closed\n");
	}
}