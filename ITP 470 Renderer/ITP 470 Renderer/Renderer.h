#include "FrankLunaCode\d3dApp.h"
#include "DrawableObject.h"
#include "Camera.h"

struct perFrameCBStruct
{
	XMMATRIX mProj;
	XMMATRIX mView;
};

struct perFrameCBPSStruct
{
	//XMVECTOR gLightDir;
	//XMVECTOR gLightColor;
	XMVECTOR gAmbientColor;
};

class Renderer : public D3DApp
{
public:
	Renderer(HINSTANCE hInstance);
	~Renderer();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	void DeclareShaderConstants(ID3D11Device* d3dDevice);
	virtual void OnMouseMoveRaw(WPARAM btnState, long x, long y) override;

	DrawableObject *box;
	Camera *camera;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	ID3D11InputLayout *mInputLayout;
	ID3D11VertexShader* colorVS;
	ID3D11PixelShader* colorPS;
	ID3D11Buffer* perFrameVSConstantBuffer;
	ID3D11Buffer* perFramePSConstantBuffer;
};
