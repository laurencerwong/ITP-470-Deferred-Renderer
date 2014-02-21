#pragma once

struct ShaderBinaryData
{
	unsigned char *shaderByteData;
	unsigned int size;

	~ShaderBinaryData() { delete[] shaderByteData; }
};

namespace FileReaderWriter
{
	bool ReadShader(const char* filename, ShaderBinaryData* &inData);
}

