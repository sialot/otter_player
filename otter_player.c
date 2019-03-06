
#include "otter_player.h"
#include "byte_list.h"

int main()
{
	fileRead("C:\\1.ts"); 
	system("pause");
	return 0;
}
int fileRead(char *filePath) {
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