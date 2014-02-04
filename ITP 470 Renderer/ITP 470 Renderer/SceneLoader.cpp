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

void LoadMaterials(ID3D11Device* d3dDevice, int inIndex, ObjectInitializeData &inObject, const aiScene *inScene)
{
	aiString texPath;
	aiReturn hasTex = inScene->mMaterials[inIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
	if (hasTex == AI_SUCCESS)
	{
		wchar_t texPathW[100];
		MultiByteToWideChar(CP_ACP, 0, texPath.C_Str(), -1, texPathW, 100);

		ID3D11Resource *texture0;
		HRESULT hr = CreateDDSTextureFromFile(d3dDevice, texPathW, &texture0, &inObject.texture0View);
		if (FAILED(hr))
		{
			texture0->Release();
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

			d3dDevice->CreateSamplerState(&textureSamplerDesc, &inObject.textureSampler);

			D3D11_TEXTURE2D_DESC textureDesc;
			reinterpret_cast<ID3D11Texture2D*>(texture0)->GetDesc(&textureDesc);
			texture0->Release();
		}
	}
}

void BuildShaders(ID3D11Device* d3dDevice, ObjectInitializeData &inObject)
{
	shaderData *vertexShaderData, *pixelShaderData;
	vertexShaderData = FileReaderWriter::ReadShader("phongVS.cso");
	pixelShaderData = FileReaderWriter::ReadShader("phongPS.cso");

	d3dDevice->CreateVertexShader(vertexShaderData->shaderByteData, vertexShaderData->size, nullptr, &inObject.vertexShader);
	d3dDevice->CreatePixelShader(pixelShaderData->shaderByteData, pixelShaderData->size, nullptr, &inObject.pixelShader);

	D3D11_INPUT_ELEMENT_DESC vertex1Desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	d3dDevice->CreateInputLayout(vertex1Desc, 3, vertexShaderData->shaderByteData, vertexShaderData->size, &inObject.inputLayout);

	//declare constant buffer description
	D3D11_BUFFER_DESC perObjectConstantBufferDesc;
	perObjectConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectConstantBufferDesc.ByteWidth = sizeof(perObjectCBStruct);
	perObjectConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectConstantBufferDesc.MiscFlags = 0;
	perObjectConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perObjectConstantBufferDesc, NULL, &inObject.perObjectConstantBuffer);

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
		newVertex.LoadAiVector3D(newVertex.Tex0, inMesh.mTextureCoords[0][i]);

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


bool ProcessMesh(ID3D11Device *ind3dDevice, const aiMesh &inMesh, ObjectInitializeData &inObject)
{
	std::vector<Vertex>tempVertexList;
	std::vector<UINT>tempIndexList;

	LoadVertices(inMesh, tempVertexList);
	LoadIndices(inMesh, tempIndexList);

	inObject.numIndices = tempIndexList.size();

	inObject.mVertexBufferStride = sizeof(Vertex);

	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.ByteWidth = sizeof(Vertex)* tempVertexList.size();
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;


	D3D11_SUBRESOURCE_DATA vertexInitData;
	vertexInitData.pSysMem = &tempVertexList[0];

	ind3dDevice->CreateBuffer(&vertexBufferDesc, &vertexInitData, &(inObject.vertexBuffer));

	D3D11_BUFFER_DESC indicesBufferDesc;
	indicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indicesBufferDesc.ByteWidth = sizeof(UINT)* tempIndexList.size();
	indicesBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indicesBufferDesc.CPUAccessFlags = 0;
	indicesBufferDesc.MiscFlags = 0;
	indicesBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexInitData;
	indexInitData.pSysMem = &tempIndexList[0];

	ind3dDevice->CreateBuffer(&indicesBufferDesc, &indexInitData, &inObject.indexBuffer);

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
		ObjectInitializeData objectData;
		bool successfulLoad = true;
		aiNode* currentNode = scene->mRootNode->mChildren[i];
		ProcessMesh(d3dDevice, *scene->mMeshes[currentNode->mMeshes[0]], objectData);
		BuildShaders(d3dDevice, objectData);
		LoadMaterials(d3dDevice, scene->mMeshes[currentNode->mMeshes[0]]->mMaterialIndex, objectData, scene);
		mDrawableObjects.push_back(new DrawableObject(objectData.vertexBuffer, objectData.indexBuffer, objectData.numIndices, objectData.perObjectConstantBuffer, objectData.inputLayout,
			objectData.mVertexBufferStride, objectData.vertexShader, objectData.pixelShader, objectData.textureSampler, objectData.texture0View));
	}
	return true;
}
