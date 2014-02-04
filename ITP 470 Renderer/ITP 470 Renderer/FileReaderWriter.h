#pragma once

struct shaderData
{
	unsigned char *shaderByteData;
	unsigned int size;

	~shaderData() { delete[] shaderByteData; }
};

namespace FileReaderWriter
{
	shaderData* ReadShader(const char* filename);
}

