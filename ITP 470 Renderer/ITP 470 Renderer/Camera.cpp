#include "Camera.h"
#include <iostream>
#include <sstream>

Camera::Camera()
{
	mPosition = XMFLOAT3(0.0f, 0.0f, 25.0f);
	mTarget = XMFLOAT3(0.0f, 0.0f, -1.0f);
	mUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMStoreFloat3(&mDirection, XMVectorSubtract(XMLoadFloat3(&mTarget), XMLoadFloat3(&mPosition)));
	mMouseCoordinates = XMFLOAT2(0, 0);
}

void Camera::Update(float dt)
{
	DoFirstPersonRotation(dt);
}

void Camera::DoFirstPersonRotation(float dt)
{
	XMMATRIX transformMatrix;
	XMVECTOR forwardVec, leftVec, rotation;

	//scale down m
	float deltaX = mMouseCoordinates.x / 1000.0f;
	float deltaY = mMouseCoordinates.y / 1000.0f;

	rotation = XMQuaternionRotationAxis(XMLoadFloat3(&mUp), 6.28f * deltaX);

	XMStoreFloat3(&mDirection, XMVector3Rotate(XMLoadFloat3(&mDirection), rotation));

	XMStoreFloat3(&mDirection, XMVector3Rotate(XMLoadFloat3(&mDirection), XMQuaternionRotationAxis(XMVector3Cross(XMLoadFloat3(&mUp), XMLoadFloat3(&mDirection)), 6.28f * deltaY)));

	//XMStoreFloat3(&mUp, XMVector3Rotate(XMLoadFloat3(&mUp), XMQuaternionRotationAxis(XMVector3Cross(XMLoadFloat3(&mUp), XMLoadFloat3(&mDirection)), 6.28f * deltaY)));

	XMStoreFloat3(&mTarget, XMLoadFloat3(&mDirection) + XMLoadFloat3(&mPosition));
	mMouseCoordinates = XMFLOAT2(0, 0);
}

void Camera::DoOrbitalRotation(float dt)
{
	XMMATRIX transformMatrix;
	XMVECTOR forwardVec, leftVec, rotation;

	//scale down m
	float deltaX = mMouseCoordinates.x / 1000.0f;
	float deltaY = mMouseCoordinates.y / 1000.0f;

	rotation = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f), 6.28f * deltaX);
	XMStoreFloat3(&mUp, XMVector3Rotate(XMLoadFloat3(&mUp), rotation));
	XMStoreFloat3(&mPosition, XMVector3Rotate(XMLoadFloat3(&mPosition), rotation));

	forwardVec = XMVectorSubtract(XMLoadFloat3(&mTarget), XMLoadFloat3(&mPosition));
	forwardVec = XMVector3Normalize(forwardVec);

	leftVec = XMVector3Cross(XMLoadFloat3(&mUp), forwardVec);
	transformMatrix = XMMatrixRotationQuaternion(XMQuaternionRotationAxis(leftVec, 6.28f * deltaY));

	XMStoreFloat3(&mUp, XMVector3Transform(XMLoadFloat3(&mUp), transformMatrix));
	XMStoreFloat3(&mPosition, XMVector3Transform(XMLoadFloat3(&mPosition), transformMatrix));
	mMouseCoordinates = XMFLOAT2(0, 0);
}

void Camera::UpdateMouseWheel(USHORT inMouseWheelDelta)
{
	XMVECTOR forwardVec;
	float signedInMouseDelta = (float)((short)inMouseWheelDelta) / 120.0;
	forwardVec = XMVectorSubtract(XMLoadFloat3(&mTarget), XMLoadFloat3(&mPosition));
	XMVECTOR lengthFromTarget = XMVector3Length(forwardVec);
	forwardVec /= lengthFromTarget;
	XMVECTOR lengthFromTargetDelta = XMVectorReplicate(signedInMouseDelta);
	lengthFromTarget += lengthFromTargetDelta;
	if (XMVector3Greater(lengthFromTarget, XMVectorZero()))
	{
		forwardVec *= lengthFromTarget;
	}
	else
	{
		lengthFromTarget -= lengthFromTargetDelta;
	}
	XMStoreFloat3(&mPosition, XMLoadFloat3(&mTarget) - forwardVec);
}