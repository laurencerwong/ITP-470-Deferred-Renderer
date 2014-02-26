#include "FrankLunaCode\d3dApp.h"
#include "SceneLoader.h"
#include "Camera.h"
#include "LightManager.h"
#include "ShaderManager.h"
#include "TexturedQuad.h"
#include "ShadowMap.h"

struct perFrameCBStruct
{
	XMMATRIX mProj;
	XMMATRIX mView;
};

struct perFrameCBPSStruct
{
	DirectionalLight gDirLight;
	PointLight gPointLight[4];
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
	void DrawDepthStencil();
	void DeclareShaderConstants(ID3D11Device* d3dDevice);
	void CreateDepthStencilState(ID3D11Device* d3dDevice);
	void SetBackBufferRenderTarget();
	virtual void OnMouseMoveRaw(WPARAM btnState, RAWMOUSE &rawMouse) override;

	int mSkybox;

	bool mUpdateObjects;
	SceneLoader *loader;
	Camera *camera;
	LightManager *lightManager;
	ShaderManager *shaderManager;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	ID3D11InputLayout *mInputLayout;
	ID3D11VertexShader* colorVS;
	ID3D11PixelShader* colorPS;
	ID3D11Buffer* perFrameVSConstantBuffer;
	ID3D11Buffer* perFramePSConstantBuffer;
	ID3D11DepthStencilState* mNoDoubleBlendDSS;

	ShadowMap *shadowMap;
	TexturedQuad *texturedQuad;
};
