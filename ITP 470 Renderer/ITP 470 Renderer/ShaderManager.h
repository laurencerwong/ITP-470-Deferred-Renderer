#pragma once
#include <d3d11.h>
#include <unordered_map>

class ShaderManager
{
public:
	ShaderManager();
	ShaderManager(ID3D11Device *inDevice, ID3D11DeviceContext *inDeviceContext) : md3dDevice(inDevice), md3dDeviceContext(inDeviceContext) {};
	~ShaderManager();

	ID3D11PixelShader* GetPixelShader(const std::string &inShaderName) { return mPixelShaders[inShaderName]; }
	ID3D11VertexShader* GetVertexShader(const std::string &inShaderName) { return mVertexShaders[inShaderName]; }

	void SetPixelShader(const std::string &inShaderName);
	void SetVertexShader(const std::string &inShaderName);

	std::string AddVertexShader(const std::string &inShaderFilename, D3D11_INPUT_ELEMENT_DESC* inInputLayoutDesc, int inSizeOfLayoutDesc);
	std::string AddPixelShader(const std::string &inShaderFilename);

private:

	struct VSWithLayout
	{
		ID3D11VertexShader* mVertexShader;
		ID3D11InputLayout* mInputLayout;
	};

	ID3D11Device *md3dDevice;
	ID3D11DeviceContext *md3dDeviceContext;

	std::string mCurrentPixelShader, mCurrentVertexShader;

	std::unordered_map<std::string, ID3D11PixelShader*>		mPixelShaders;
	std::unordered_map<std::string, ID3D11VertexShader*>	mVertexShaders;
	std::unordered_map<std::string, VSWithLayout>			mVSWithLayouts;
};

