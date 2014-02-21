#include "FileReaderWriter.h"
#include <cstdio>
#include <cstdlib>

bool FileReaderWriter::ReadShader(const char* filename, ShaderBinaryData* &inData)
{
	FILE* file = nullptr;
	fopen_s(&file, filename, "rb"); //rb is to open the file as binary
	if (file != NULL)
	{
		inData = (ShaderBinaryData *)malloc(sizeof(ShaderBinaryData));
		fseek(file, 0, SEEK_END);
		inData->size = ftell(file);
		fseek(file, 0, SEEK_SET);

		inData->shaderByteData = (unsigned char *)malloc(inData->size);

		int numBytesRead = 0;
		while (numBytesRead != inData->size)
		{
			fread(&inData->shaderByteData[numBytesRead++], 1, inData->size, file);
		}

		fclose(file);

		return true;
	}
	return false;
}