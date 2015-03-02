#pragma once

#include <DirectXMath.h>

struct BoundingSphere
{
	BoundingSphere() : mCenter(0.0f, 0.0f, 0.0f), mRadius(0.0f) {};
	DirectX::XMFLOAT3 mCenter;
	float mRadius;
};