#include "FrankLunaCode\d3dApp.h"
#include "Mesh.h"

class Renderer : public D3DApp
{
public:
	Renderer(HINSTANCE hInstance);
	~Renderer();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();
	void BuildVertexDescription(ID3D11Device* d3dDevice);
	void BuildEffect(ID3D11Device* d3dDevice);
	void BuildShaders(ID3D11Device* d3dDevice);

	Mesh *box;
	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;
	ID3DX11Effect *mEffect;
	ID3DX11EffectTechnique *mTechnique;
	ID3DX11EffectMatrixVariable *mWorldViewProj;
	ID3D11InputLayout *mInputLayout;
	ID3D11VertexShader* colorVS;
	ID3D11PixelShader* colorPS;
	ID3D11Buffer* constantBuffer;

};
