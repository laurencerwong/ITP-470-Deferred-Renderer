#include "FileReaderWriter.h"
#include <cstdio>
#include <cstdlib>

shaderData* FileReaderWriter::ReadShader(const char* filename)
{
	FILE* file;
	fopen_s(&file, filename, "rb"); //rb is to open the file as binary
	if (file != NULL)
	{
		shaderData *data = (shaderData *)malloc(sizeof(shaderData));
		fseek(file, 0, SEEK_END);
		data->size = ftell(file);
		fseek(file, 0, SEEK_SET);

		data->shaderByteData = (unsigned char *)malloc(data->size);

		int numBytesRead = 0;
		while (numBytesRead != data->size)
		{
			fread(&data->shaderByteData[numBytesRead++], 1, data->size, file);
		}

		fclose(file);

		return data;
	}
	return nullptr;
}