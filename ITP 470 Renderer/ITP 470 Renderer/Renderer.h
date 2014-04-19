#include "FrankLunaCode\d3dApp.h"
#include "SceneLoader.h"
#include "Camera.h"
#include "LightManager.h"
#include "ShaderManager.h"
#include "TexturedQuad.h"
#include "ShadowMap.h"
#include "GBuffer.h"
#include "LightAccumulationBuffer.h"
#include "CubeMap.h"

struct perFrameCBStruct
{
	XMMATRIX mProj;
	XMMATRIX mView;
	XMVECTOR gFarFrustrumCorners[4];
};

struct perFrameDeferredPSStruct
{
	XMMATRIX gShadowTransform;
	XMVECTOR gAmbientColor;
	XMVECTOR gCamPos;
};
struct perFrameCombinationPSStruct
{
	XMVECTOR gAmbientColor;
};

struct perFrameDeferredFillStruct
{
	XMVECTOR gFarPlane;
};

struct perFrameCBPSStruct
{
	DirectionalLight gDirLight;
	PointLight gPointLight[4];
	XMVECTOR gAmbientColor;
	XMVECTOR gCamPos;
};

struct perFramePointLightShadowPSStruct
{
	PointLight gPointLight;
	XMVECTOR gColor;
};

typedef enum ViewMode
{
	VIEW_MODE_SHADOW_DEPTH,
	VIEW_MODE_DEPTH,
	VIEW_MODE_FULL,
	VIEW_MODE_DIFFUSE,
	VIEW_MODE_NORMAL,
	VIEW_MODE_SPECULAR,
	VIEW_MODE_LIGHT_ACCUM
} ViewMode;

class Renderer : public D3DApp
{
public:
	Renderer(HINSTANCE hInstance);
	~Renderer();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void BuildShadowTransform(const DirectionalLight &inDirectionalLight);
	void BuildShadowTransform(const PointLight &inPointLight, int inFaceNum);
	void DrawScene();
	void DrawSceneToShadowMap(ShadowMap* inShadowMap);
	void DrawDeferred();
	void DrawPhong();
	void DrawDepth();
	void DrawOmniDepth(int inIndex);
	void DrawDepthForPoint(PointLight &inLight);
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

	int mCurrentCubeMap;
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
	ID3D11Buffer* perFramePSDeferredBuffer;
	ID3D11Buffer* perFramePSDeferredFillBuffer;
	ID3D11Buffer* perFramePSCombinationBuffer;
	ID3D11Buffer* perFramePSPointBuffer;
	ID3D11DepthStencilState* mAlwaysLessDepthStencilState;
	ID3D11DepthStencilState* mDefaultDepthStencilState;
	UINT* mDefaultDepthStencilStateRef;
	ID3D11RasterizerState *mNoShadowAcneState;
	ID3D11RasterizerState *mNoCulling;


	ShadowMap *shadowMap;
	GBuffer *gBuffer;
	CubeMap *cubeShadowMap;
	LightAccumulationBuffer *laBuffer;
	TexturedQuad *texturedQuad;
	TexturedQuad *deferredRenderTarget;
};
