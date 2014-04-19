#include <DirectXMath.h>
#include "assimp\include\vector3.h"

using DirectX::XMFLOAT3;
using DirectX::XMFLOAT2;

struct VertexF3F3F3F3F2
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 Bitangent;
	XMFLOAT2 Tex0;

	inline void LoadAiVector3D(XMFLOAT3& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT3(inRHSVector.x, inRHSVector.y, inRHSVector.z);
	}
	inline void LoadAiVector3D(XMFLOAT2& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT2(inRHSVector.x, inRHSVector.y);
	}
};

struct VertexF3F2
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex0;

	inline void LoadAiVector3D(XMFLOAT3& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT3(inRHSVector.x, inRHSVector.y, inRHSVector.z);
	}
	inline void LoadAiVector3D(XMFLOAT2& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT2(inRHSVector.x, inRHSVector.y);
	}
};
struct VertexF3F3
{
	XMFLOAT3 Pos;
	XMFLOAT3 Tex0;

	inline void LoadAiVector3D(XMFLOAT3& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT3(inRHSVector.x, inRHSVector.y, inRHSVector.z);
	}
};
struct VertexF3
{
	XMFLOAT3 Pos;

	inline void LoadAiVector3D(XMFLOAT3& inLHSVector, const aiVector3D& inRHSVector)
	{
		inLHSVector = XMFLOAT3(inRHSVector.x, inRHSVector.y, inRHSVector.z);
	}
};
