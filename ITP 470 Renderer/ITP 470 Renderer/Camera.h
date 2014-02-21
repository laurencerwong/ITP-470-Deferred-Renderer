#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class Camera
{
public:
	Camera();
	Camera(const Camera&);
	~Camera();

	void	Update(float dt);

	const XMFLOAT3		GetPosition()		const { return mPosition; }
	const XMFLOAT3		GetTarget()			const { return mTarget; }
	const XMFLOAT3		GetUp()				const { return mUp; }
	const XMMATRIX		GetViewMatrix()		const { return XMMatrixLookAtLH(XMLoadFloat3(&mPosition), XMLoadFloat3(&mTarget), XMLoadFloat3(&mUp)); }

	void	SetPosition(const XMFLOAT3& inPosition)		{ mPosition = inPosition; }
	void	SetTarget(const XMFLOAT3& inTarget)			{ mTarget = inTarget; }
	void	SetUp(const XMFLOAT3& inUp)					{ mUp = inUp; }
	void	SetView(const XMFLOAT4X4& inView)			{ mView = inView; }
	void	SetMouseCoords(float x, float y)			{ mMouseCoordinates = XMFLOAT2(x, y); }
	void	UpdateMouseWheel(USHORT inMouseWheelDelta);

private:

	void	DoOrbitalRotation(float dt);
	void	DoFirstPersonRotation(float dt);

	XMFLOAT2 mMouseCoordinates;

	XMFLOAT3 mPosition;
	XMFLOAT3 mTarget;
	XMFLOAT3 mUp;
	
	XMFLOAT3 mDirection;

	XMFLOAT4X4 mView;

};