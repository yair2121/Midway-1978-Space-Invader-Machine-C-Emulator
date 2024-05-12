#include <stdio.h>
#include<stdlib.h>

int main(int argc, char** argv) {
	FILE* codeFp;
	int err = fopen_s(&codeFp, argv[1], "rb");
	if (err != NULL) {
		printf("Not able to open the file.");
		exit(1); // What to return here?
	}

	fseek(codeFp, 0L, SEEK_END);
	size_t fileSize = ftell(codeFp);
	fseek(codeFp, 0L, SEEK_SET);

	unsigned char* buffer = (unsigned char*)malloc(fileSize);
	size_t size = fread(buffer, 1, fileSize, codeFp); // TODO: Validate that return value equal fileSize
	fclose(codeFp);

	DisassembleCode(size, buffer);


	free(buffer);
	return 0;
}