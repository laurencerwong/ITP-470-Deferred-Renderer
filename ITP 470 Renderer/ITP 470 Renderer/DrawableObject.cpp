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
}

DrawableObject::~DrawableObject()
{
}

void DrawableObject::Update(float deltaTime)
{
	mRotationAmount += deltaTime;
	XMStoreFloat4(&mRotation, XMQuaternionRotationAxis(XMLoadFloat3(&XMFLOAT3(0.0f, 1.0f, 0.0f)), mRotationAmount));
}

void DrawableObject::Draw(ID3D11DeviceContext* d3dDeviceContext)
{
	//TODO add member variables for the scale, rotate, translate and set them here
	UINT stride = mVertexBufferStride;
	UINT offset = 0;

	d3dDeviceContext->IASetInputLayout(inputLayout);
	d3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dDeviceContext->VSSetShader(vertexShader, 0, 0);
	d3dDeviceContext->PSSetShader(pixelShader, 0, 0);

	//set object vertex shader resources
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	d3dDeviceContext->Map(perObjectVSCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perObjectCBVSStruct *vertexShaderCB = (perObjectCBVSStruct*)mappedResource.pData;
	XMStoreFloat4x4(&vertexShaderCB->mWorld, XMMatrixScaling(mScale, mScale, mScale) * XMMatrixRotationQuaternion(XMLoadFloat4(&mRotation)) * XMMatrixTranslationFromVector(XMLoadFloat3(&mPosition)));
	d3dDeviceContext->Unmap(perObjectVSCB, 0);
	d3dDeviceContext->VSSetConstantBuffers(1, 1, &perObjectVSCB);

	//set object pixel shader resources
	d3dDeviceContext->Map(perObjectPSCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	perObjectCBPSStruct *pixelShaderCB = (perObjectCBPSStruct*)mappedResource.pData;
	pixelShaderCB->mMaterial = mMaterial;
	d3dDeviceContext->Unmap(perObjectPSCB, 0);
	d3dDeviceContext->PSSetConstantBuffers(1, 1, &perObjectPSCB);

	d3dDeviceContext->PSSetSamplers(0, 1, &textureSampler);

	d3dDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	d3dDeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	for (std::tuple<UINT, UINT, int, unsigned int> part : mParts)
	{
		d3dDeviceContext->PSSetShaderResources(0, 1, &std::get<0>(mTextures[std::get<3>(part)]));
		d3dDeviceContext->PSSetShaderResources(1, 1, &std::get<1>(mTextures[std::get<3>(part)]));
		d3dDeviceContext->DrawIndexed(std::get<0>(part), std::get<1>(part), std::get<2>(part));
	}

}

