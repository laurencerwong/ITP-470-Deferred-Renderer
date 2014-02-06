#include "SceneLoader.h"
#include "assimp/include/scene.h"
#include "assimp/include/Importer.hpp"
#include "assimp/include/postprocess.h"
#include "DDSTextureLoader/DDSTextureLoader.h"
#include "assimp/include/mesh.h"
#include <sstream>
#include "FileReaderWriter.h"
#include "DrawableObject.h"

SceneLoader::SceneLoader()
{
}


SceneLoader::~SceneLoader()
{
}

void LoadMaterials(ID3D11Device* d3dDevice, int inIndex, DrawableObject &inObject, const aiScene *inScene)
{
	aiString texPath;
	aiReturn hasTex = inScene->mMaterials[inIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
	if (hasTex == AI_SUCCESS)
	{
		wchar_t texPathW[100];
		MultiByteToWideChar(CP_ACP, 0, texPath.C_Str(), -1, texPathW, 100);

		ID3D11Resource *texture0;
		ID3D11ShaderResourceView *newShaderResourceView;
		HRESULT hr = CreateDDSTextureFromFile(d3dDevice, texPathW, &texture0, &newShaderResourceView);
		if (FAILED(hr))
		{
		}
		else
		{
			D3D11_SAMPLER_DESC textureSamplerDesc;
			ZeroMemory(&textureSamplerDesc, sizeof(textureSamplerDesc));
			textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			textureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			textureSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

			ID3D11SamplerState *newSamplerState;
			d3dDevice->CreateSamplerState(&textureSamplerDesc, &newSamplerState);

			D3D11_TEXTURE2D_DESC textureDesc;
			reinterpret_cast<ID3D11Texture2D*>(texture0)->GetDesc(&textureDesc);
			texture0->Release();

			inObject.SetShaderResourceView(newShaderResourceView);
			inObject.SetSamplerState(newSamplerState);
		}
	}
	aiColor3D ambientColor, diffuseColor, specularColor;
	float shininess;
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_SHININESS, shininess);

	inObject.SetMaterial(ambientColor, diffuseColor, specularColor, shininess);
}

void BuildShaders(ID3D11Device* d3dDevice, DrawableObject &inObject)
{
	shaderData *vertexShaderData, *pixelShaderData;
	vertexShaderData = FileReaderWriter::ReadShader("phongVS.cso");
	pixelShaderData = FileReaderWriter::ReadShader("phongPS.cso");

	ID3D11VertexShader *newVertexShader;
	ID3D11PixelShader *newPixelShader;
	ID3D11InputLayout *newInputLayout;
	ID3D11Buffer *newConstantBuffer;
	d3dDevice->CreateVertexShader(vertexShaderData->shaderByteData, vertexShaderData->size, nullptr, &newVertexShader);
	d3dDevice->CreatePixelShader(pixelShaderData->shaderByteData, pixelShaderData->size, nullptr, &newPixelShader);

	D3D11_INPUT_ELEMENT_DESC vertex1Desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	d3dDevice->CreateInputLayout(vertex1Desc, 3, vertexShaderData->shaderByteData, vertexShaderData->size, &newInputLayout);

	//declare constant buffer description
	D3D11_BUFFER_DESC perObjectConstantBufferDesc;
	perObjectConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectConstantBufferDesc.ByteWidth = sizeof(perObjectCBStruct);
	perObjectConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectConstantBufferDesc.MiscFlags = 0;
	perObjectConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perObjectConstantBufferDesc, NULL, &newConstantBuffer);

	inObject.SetVertexShader(newVertexShader);
	inObject.SetPixelShader(newPixelShader);
	inObject.SetInputLayout(newInputLayout);
	inObject.SetConstantBuffer(newConstantBuffer);

	delete vertexShaderData;
	delete pixelShaderData;
}

void LoadVertices(const aiMesh &inMesh, std::vector<Vertex> &convertedVertices)
{
	for (unsigned int i = 0; i < inMesh.mNumVertices; ++i)
	{
		Vertex newVertex;
		newVertex.LoadAiVector3D(newVertex.Pos, inMesh.mVertices[i]);
		newVertex.LoadAiVector3D(newVertex.Normal, inMesh.mNormals[i]);
		if (inMesh.HasTextureCoords(0))
		{
			newVertex.LoadAiVector3D(newVertex.Tex0, inMesh.mTextureCoords[0][i]);
		}

		convertedVertices.push_back(newVertex);
	}
}

void LoadIndices(const aiMesh &inMesh, std::vector<UINT> &convertedIndices)
{
	for (unsigned int i = 0; i < inMesh.mNumFaces; ++i)
	{
		for (unsigned int j = 0; j < inMesh.mFaces[i].mNumIndices; ++j)
		{
			convertedIndices.push_back(inMesh.mFaces[i].mIndices[j]);
		}
	}
}


bool ProcessMesh(ID3D11Device *ind3dDevice, const aiMesh &inMesh, DrawableObject &inObject)
{
	std::vector<Vertex>tempVertexList;
	std::vector<UINT>tempIndexList;

	LoadVertices(inMesh, tempVertexList);
	LoadIndices(inMesh, tempIndexList);

	inObject.SetNumIndicies(tempIndexList.size());

	inObject.SetVertexBufferStride(sizeof(Vertex));

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.ByteWidth = sizeof(Vertex)* tempVertexList.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;


	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &tempVertexList[0];

	ID3D11Buffer *newVertexBuffer, *newIndexBuffer;

	ind3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &(newVertexBuffer));

	D3D11_BUFFER_DESC indicesBufferDesc;
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.ByteWidth = sizeof(UINT)* tempIndexList.size();
	indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indicesBufferDesc.CPUAccessFlags = 0;
	indicesBufferDesc.MiscFlags = 0;
	indicesBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = &tempIndexList[0];

	ind3dDevice->CreateBuffer(&indicesBufferDesc, &indexInitData, &newIndexBuffer);

	inObject.SetVertexBuffer(newVertexBuffer);
	inObject.SetIndexBuffer(newIndexBuffer);

	return true;
}

bool SceneLoader::LoadFile(const char* filename)
{
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(filename, 0);
	if (!scene)
	{
		std::stringstream oss;
		oss << "ERROR - File: " << filename << " not found." << std::endl;
		std::string debugMsg(oss.str());
		OutputDebugStringA(debugMsg.c_str());
		return false;
	}
	for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
	{
		DrawableObject *newObject = new DrawableObject();
		bool successfulLoad = true;
		aiNode* currentNode = scene->mRootNode->mChildren[i];
		ProcessMesh(d3dDevice, *scene->mMeshes[currentNode->mMeshes[0]], *newObject);
		BuildShaders(d3dDevice, *newObject);
		LoadMaterials(d3dDevice, scene->mMeshes[currentNode->mMeshes[0]]->mMaterialIndex, *newObject, scene);
		mDrawableObjects.push_back(newObject);
	}
	return true;
}
