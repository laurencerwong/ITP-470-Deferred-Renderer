#include "FrankLunaCode\d3dApp.h"
#include "SceneLoader.h"
#include "Camera.h"
#include "LightManager.h"
#include "ShaderManager.h"
#include "TexturedQuad.h"
#include "ShadowMap.h"
#include "GBuffer.h"

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

typedef enum ViewMode
{
	VIEW_MODE_SHADOW_DEPTH,
	VIEW_MODE_DEPTH,
	VIEW_MODE_FULL,
	VIEW_MODE_DIFFUSE,
	VIEW_MODE_NORMAL,
	VIEW_MODE_SPECULAR
} ViewMode;

class Renderer : public D3DApp
{
public:
	Renderer(HINSTANCE hInstance);
	~Renderer();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void BuildShadowTransform();
	void DrawScene();
	void DrawSceneToShadowMap(ShadowMap* inShadowMap);
	void DrawPhong();
	void DrawDepth();
	void DrawDepthStencil();
	void FillGBuffer();
	void DeclareShaderConstants(ID3D11Device* d3dDevice);
	void CreateDepthStencilState(ID3D11Device* d3dDevice);
	void SetBackBufferRenderTarget();
	void SetPerObjectVSCB();
	void SetPerObjectPSCB();
	void UpdatePerFrameVSCB();
	void UpdatePerFrameVSCBShadowMap();
	void InitializeMiscShaders();
	virtual void OnMouseMoveRaw(WPARAM btnState, RAWMOUSE &rawMouse) override;
	virtual void OnKeyUp(WPARAM keyCode) override;
	virtual void OnKeyDown(WPARAM keyCode) override;

	int mSkybox;

	bool mUpdateObjects;
	bool mUpdateLights;
	ViewMode mCurrentViewMode;

	SceneLoader *loader;
	Camera *camera;
	LightManager *lightManager;
	ShaderManager *shaderManager;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	XMFLOAT4X4 mLightView;
	XMFLOAT4X4 mLightProj;
	XMFLOAT4X4 mShadowTransform;

	ID3D11InputLayout *mInputLayout;
	ID3D11VertexShader* colorVS;
	ID3D11PixelShader* colorPS;
	ID3D11Buffer* perFrameVSConstantBuffer;
	ID3D11Buffer* perFramePSConstantBuffer;
	ID3D11DepthStencilState* mNoDoubleBlendDSS;
	ID3D11RasterizerState *mNoShadowAcneState;


	ShadowMap *shadowMap;
	GBuffer *gBuffer;
	TexturedQuad *texturedQuad;
};
