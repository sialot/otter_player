
#include "otter_player.h"
#include "byte_list.h"

int main()
{

	/*test_add();
	test_add();
	test_add();
	test_add();
	test_add();
	printf("len %d\n", pat_loop_data_buffer->len);
	printf("free %d\n", pat_loop_data_buffer->free);
	for (int i = 0; i < pat_loop_data_buffer->len; i++) {
		printf("pBytes[%d]:%d\n", i, pat_loop_data_buffer->pBytes[i]);
	}*/
	//test_add();

	fileRead("C:\\1.ts"); 
	system("pause");
	return 0;
}

void test_add() {
	BYTE_LIST *pat_loop_data_buffer = byte_list_create(1);

	unsigned char data[5];
	for (int j = 0; j < 5; j++) {
		data[j] = j;
	}

	unsigned char *pLoopData = data;

	byte_list_add_list(pat_loop_data_buffer, pLoopData, 5);


	printf("len %d\n", pat_loop_data_buffer->len);
	printf("free %d\n", pat_loop_data_buffer->free);
	for (int i = 0; i < pat_loop_data_buffer->len; i++) {
		printf("pBytes[%d]:%d\n", i, pat_loop_data_buffer->pBytes[i]);
	}
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