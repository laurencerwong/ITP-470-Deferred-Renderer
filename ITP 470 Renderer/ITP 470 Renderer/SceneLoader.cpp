#include "SceneLoader.h"
#include "assimp/include/scene.h"
#include "assimp/include/Importer.hpp"
#include "assimp/include/postprocess.h"
#include "DDSTextureLoader/DDSTextureLoader.h"
#include "assimp/include/mesh.h"
#include <sstream>
#include "FileReaderWriter.h"
#include "DrawableObject.h"
#include "Vertex.h"

SceneLoader::SceneLoader()
{
}


SceneLoader::~SceneLoader()
{
}

void LoadMaterials(ID3D11Device* d3dDevice, int inIndex, DrawableObject &inObject, const aiScene *inScene)
{
	aiString texPath, normTexPath;
	aiReturn hasTex = inScene->mMaterials[inIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texPath);
	if (hasTex == AI_SUCCESS)
	{
		wchar_t texPathW[150];

		//std::string texPathString = "dabrovic-sponza/01_STUB.dds";

		MultiByteToWideChar(CP_ACP, 0, texPath.C_Str(), -1, texPathW, 150);

		ID3D11Resource *texture, *textureNorm;
		ID3D11ShaderResourceView *newDiffuseShaderResourceView, *newNormalMapShaderResourceView;
		HRESULT hr = CreateDDSTextureFromFile(d3dDevice, texPathW, &texture, &newDiffuseShaderResourceView);
		if (FAILED(hr))
		{
			CreateDDSTextureFromFile(d3dDevice, L"texture_missing.dds", &texture, &newDiffuseShaderResourceView);
			std::stringstream oss;
			oss << "ERROR - Texture: " << texPathW << " not found." << std::endl;
			std::string debugMsg(oss.str());
			OutputDebugStringA(debugMsg.c_str());
		}
		std::string normTexPath = std::string(texPath.C_Str());
		normTexPath.resize(normTexPath.length() - 4); // remove the .dds
		normTexPath += "_normal.dds";

		//normTexPath = "dabrovic-sponza/01_STUB_normal.dds";

		MultiByteToWideChar(CP_ACP, 0, normTexPath.c_str(), -1, texPathW, 150);

		hr = CreateDDSTextureFromFile(d3dDevice, texPathW, &textureNorm, &newNormalMapShaderResourceView);
		if (FAILED(hr))
		{
			std::stringstream oss;
			oss << "ERROR - Normal Texture: " << texPathW << " not found." << std::endl;
			std::string debugMsg(oss.str());
			OutputDebugStringA(debugMsg.c_str());
			CreateDDSTextureFromFile(d3dDevice, L"texture_missing_normal.dds", &textureNorm, &newNormalMapShaderResourceView);
		}

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
		reinterpret_cast<ID3D11Texture2D*>(texture)->GetDesc(&textureDesc);
		texture->Release();
		reinterpret_cast<ID3D11Texture2D*>(textureNorm)->GetDesc(&textureDesc);
		textureNorm->Release();

		inObject.AddTexture(newDiffuseShaderResourceView, newNormalMapShaderResourceView);
		inObject.SetSamplerState(newSamplerState);
	}
	hasTex = inScene->mMaterials[inIndex]->GetTexture(aiTextureType_NORMALS, 0, &texPath);
	if (hasTex == AI_SUCCESS)
	{
		wchar_t texPathW[100];
		MultiByteToWideChar(CP_ACP, 0, texPath.C_Str(), -1, texPathW, 100);

		ID3D11Resource *texture0;
		ID3D11ShaderResourceView *newShaderResourceView;
		HRESULT hr = CreateDDSTextureFromFile(d3dDevice, texPathW, &texture0, &newShaderResourceView);
		if (FAILED(hr))
		{
			CreateDDSTextureFromFile(d3dDevice, L"texture_missing.dds", &texture0, &newShaderResourceView);
		}
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

		inObject.SetDiffuseResourceView(newShaderResourceView);
		inObject.SetSamplerState(newSamplerState);
	}
	aiColor3D ambientColor, diffuseColor, specularColor;
	float shininess;
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_COLOR_AMBIENT, ambientColor);
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
	inScene->mMaterials[inIndex]->Get(AI_MATKEY_SHININESS, shininess);

	inObject.SetMaterial(ambientColor, diffuseColor, specularColor, shininess);
}

void BuildShaders(ID3D11Device* d3dDevice, DrawableObject &inObject, ShaderManager* inShaderManager)
{
	ID3D11Buffer *newVSConstantBuffer, *newPSConstantBuffer;

	D3D11_INPUT_ELEMENT_DESC vertex1Desc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	inObject.SetVertexShader(inShaderManager->AddVertexShader("phongVS.cso", vertex1Desc, 5));
	inObject.SetPixelShader(inShaderManager->AddPixelShader("phongPS.cso"));

	//declare VS constant buffer description
	D3D11_BUFFER_DESC perObjectConstantBufferDesc;
	perObjectConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectConstantBufferDesc.ByteWidth = sizeof(perObjectCBVSStruct);
	perObjectConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectConstantBufferDesc.MiscFlags = 0;
	perObjectConstantBufferDesc.StructureByteStride = 0;
	d3dDevice->CreateBuffer(&perObjectConstantBufferDesc, NULL, &newVSConstantBuffer);


	//declare PS constant buffer description
	perObjectConstantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	perObjectConstantBufferDesc.ByteWidth = sizeof(perObjectCBPSStruct);
	perObjectConstantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	perObjectConstantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	perObjectConstantBufferDesc.MiscFlags = 0;
	perObjectConstantBufferDesc.StructureByteStride = 0;

	d3dDevice->CreateBuffer(&perObjectConstantBufferDesc, NULL, &newPSConstantBuffer);

	inObject.SetVSConstantBuffer(newVSConstantBuffer);
	inObject.SetPSConstantBuffer(newPSConstantBuffer);
}

void LoadVertices(const aiMesh &inMesh, std::vector<Vertex> &convertedVertices)
{
	for (unsigned int i = 0; i < inMesh.mNumVertices; ++i)
	{
		Vertex newVertex;
		newVertex.LoadAiVector3D(newVertex.Pos, inMesh.mVertices[i]);
		if (inMesh.HasNormals())
		{
			newVertex.LoadAiVector3D(newVertex.Normal, inMesh.mNormals[i]);
		}
		if (inMesh.HasTextureCoords(0))
		{
			newVertex.LoadAiVector3D(newVertex.Tex0, inMesh.mTextureCoords[0][i]);
			newVertex.Tex0.y = 1.0 - newVertex.Tex0.y;
		}
		if (inMesh.HasTangentsAndBitangents())
		{
			newVertex.LoadAiVector3D(newVertex.Tangent, inMesh.mTangents[i]);
			//newVertex.Tangent.z = -newVertex.Tangent.z;
			newVertex.LoadAiVector3D(newVertex.Bitangent, inMesh.mBitangents[i]);
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


bool ProcessMesh(ID3D11Device *ind3dDevice, const aiMesh &inMesh, DrawableObject &inObject, std::vector<Vertex> &inVertexList, std::vector<UINT> &inIndexList, unsigned int inMaterialIndex)
{

	int inPrevIndexListSize = inIndexList.size();
	int inPrevVertexListSize = inVertexList.size();

	LoadVertices(inMesh, inVertexList);
	LoadIndices(inMesh, inIndexList);

	inObject.AddPart(inIndexList.size() - inPrevIndexListSize, inPrevIndexListSize, inPrevVertexListSize, inMaterialIndex);

	return true;
}

int SceneLoader::LoadFile(const char* filename)
{
	Assimp::Importer importer;

	const aiScene *scene = importer.ReadFile(filename, 0);
	scene = importer.ApplyPostProcessing(aiProcess_CalcTangentSpace | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_JoinIdenticalVertices);
	if (!scene)
	{
		std::stringstream oss;
		oss << "ERROR - File: " << filename << " not found." << std::endl;
		std::string debugMsg(oss.str());
		OutputDebugStringA(debugMsg.c_str());
		return false;
	}
	DrawableObject *newObject = new DrawableObject(mShaderManager);
	std::vector<Vertex> vertexList;
	std::vector<UINT> indexList;
	std::stringstream oss;
	for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; ++i)
	{
		bool successfulLoad = true;
		aiNode* currentNode = scene->mRootNode->mChildren[i];
		BuildShaders(d3dDevice, *newObject, mShaderManager);
		for (unsigned int j = 0; j < currentNode->mNumMeshes; ++j)
		{
			ProcessMesh(d3dDevice, *scene->mMeshes[currentNode->mMeshes[j]], *newObject, vertexList, indexList, scene->mMeshes[currentNode->mMeshes[j]]->mMaterialIndex - 1);
			//LoadMaterials(d3dDevice, scene->mMeshes[currentNode->mMeshes[j]]->mMaterialIndex, *newObject, scene);
			oss << "MatIndex = " << scene->mMeshes[currentNode->mMeshes[j]]->mMaterialIndex << "\n";
		}
	}
	std::string debugMsg(oss.str());
	OutputDebugStringA(debugMsg.c_str());
	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		LoadMaterials(d3dDevice, i, *newObject, scene);
	}
	newObject->GetMeshData()->Initialize(d3dDevice, vertexList, indexList);
	mDrawableObjects.push_back(newObject);

	return mDrawableObjects.size() - 1;
}
