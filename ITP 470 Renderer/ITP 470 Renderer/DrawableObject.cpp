#include "DrawableObject.h"
#include "FrankLunaCode\d3dUtil.h"
#include <d3dcompiler.h>
#include "assimp/include/Importer.hpp"
#include "assimp/include/postprocess.h"
#include "DDSTextureLoader/DDSTextureLoader.h"


DrawableObject::DrawableObject() :
mPosition(0.0f, 0.0f, 0.0f),
mRotationAmount(0.0f),
mScale(1.0f)
{
	mMeshData = new MeshData();
}

DrawableObject::~DrawableObject()
{
}

void DrawableObject::Update(float deltaTime)
{
	mRotationAmount += deltaTime;
	XMStoreFloat4(&mRotation, XMQuaternionRotationAxis(XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)), mRotationAmount));
}

void DrawableObject::UpdateVSConstantBuffer(ID3D11DeviceContext* d3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	d3dDeviceContext->Map(perObjectVSCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perObjectCBVSStruct *vertexShaderCB = (perObjectCBVSStruct*)mappedResource.pData;
	XMStoreFloat4x4(&vertexShaderCB->mWorld, XMMatrixScaling(mScale, mScale, mScale) * XMMatrixRotationQuaternion(XMLoadFloat4(&mRotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&mPosition)));
	d3dDeviceContext->Unmap(perObjectVSCB, 0);
	d3dDeviceContext->VSSetConstantBuffers(1, 1, &perObjectVSCB);
}

void DrawableObject::UpdatePSConstantBuffer(ID3D11DeviceContext *d3dDeviceContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	d3dDeviceContext->Map(perObjectPSCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perObjectCBPSStruct *pixelShaderCB = (perObjectCBPSStruct*)mappedResource.pData;
	pixelShaderCB->mMaterial = mMaterial;
	d3dDeviceContext->Unmap(perObjectPSCB, 0);
	d3dDeviceContext->PSSetConstantBuffers(1, 1, &perObjectPSCB);

}

void DrawableObject::UpdateSamplerState(ID3D11DeviceContext *d3dDeviceContext)
{
	d3dDeviceContext->PSSetSamplers(0, 1, &textureSampler);

}

void DrawableObject::Draw(ID3D11DeviceContext* d3dDeviceContext)
{

	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mMeshData->SetVertexAndIndexBuffers(d3dDeviceContext);

	for (auto &part : mParts)
	{
		d3dDeviceContext->PSSetShaderResources(0, 1, &std::get<0>(mTextures[part.mMaterialIndex]));
		d3dDeviceContext->PSSetShaderResources(1, 1, &std::get<1>(mTextures[part.mMaterialIndex]));
		d3dDeviceContext->DrawIndexed(part.mNumIndices, part.mIndexBufferStart, part.mVertexBufferStart);
	}

}

void DrawableObject::DrawWithoutTextures(ID3D11DeviceContext* d3dDeviceContext)
{
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	mMeshData->SetVertexAndIndexBuffers(d3dDeviceContext);

	for (auto &part : mParts)
	{
		d3dDeviceContext->DrawIndexed(part.mNumIndices, part.mIndexBufferStart, part.mVertexBufferStart);
	}
}

