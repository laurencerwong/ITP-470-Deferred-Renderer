#include "FrankLunaCode\d3dApp.h"
#include "SceneLoader.h"
#include "Camera.h"
#include "LightManager.h"

struct perFrameCBStruct
{
	XMMATRIX mProj;
	XMMATRIX mView;
};

struct perFrameCBPSStruct
{
	DirectionalLight gDirLights;
	PointLight gPointLight1;
	PointLight gPointLight2;
	XMVECTOR gAmbientColor;
	XMVECTOR gCamPos;
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
	virtual void OnMouseMoveRaw(WPARAM btnState, RAWMOUSE &rawMouse) override;

	SceneLoader *loader;
	Camera *camera;
	LightManager *lightManager;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	ID3D11InputLayout *mInputLayout;
	ID3D11VertexShader* colorVS;
	ID3D11PixelShader* colorPS;
	ID3D11Buffer* perFrameVSConstantBuffer;
	ID3D11Buffer* perFramePSConstantBuffer;
};
