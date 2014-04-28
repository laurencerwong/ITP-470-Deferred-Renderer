#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include "assimp/include/vector3.h"
#include "assimp/include/mesh.h"
#include <vector>
#include "ShaderManager.h"
#include "BoundingSphere.h"
#include "Vertex.h"
#include "MeshData.h"

using namespace DirectX;

class DrawableObject;

class SceneLoader
{
public:
	SceneLoader();
	SceneLoader(ID3D11Device *ind3dDevice, ShaderManager *inShaderManager) : d3dDevice(ind3dDevice), mShaderManager(inShaderManager) { };
	~SceneLoader();

	int LoadFile(const char* filename);
	std::vector<DrawableObject*> &GetDrawableObjects() { return mDrawableObjects; }
	DrawableObject* GetDrawableObject(int inDrawableObjectID) { return mDrawableObjects[inDrawableObjectID]; }
	BoundingSphere GetBoundingSphere() { return mSceneBoundingSphere; }
	int LoadModel(const char* filename, MeshData& outMesh);
	HRESULT LoadTexture(ID3D11Device *ind3dDevice, const char* filename, ID3D11Resource **outResource, ID3D11ShaderResourceView **outResourceView);
private:
	bool ProcessMesh(ID3D11Device *ind3dDevice, aiMesh &inMesh , DrawableObject &inObject, std::vector<VertexF3F3F3F3F2> &inVertexList, std::vector<UINT> &inIndexList, unsigned int inMaterialIndex);

	BoundingSphere mSceneBoundingSphere;
	ID3D11Device *d3dDevice;
	ShaderManager *mShaderManager;
	std::vector<DrawableObject*>mDrawableObjects;
};

