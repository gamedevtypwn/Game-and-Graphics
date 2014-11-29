#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "resource.h"
#include "GameObject.h"


#define ASTEROID_COUNT 100

using namespace DirectX;

struct SimpleVertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
};

struct ConstantBuffer

{

	XMMATRIX mWorld;
	XMMATRIX mView;
	XMMATRIX mProjection;

	XMFLOAT4 diffuseMaterial;
	XMFLOAT4 diffuseLight;

	XMFLOAT4 gAmbientMtrl;
	XMFLOAT4 gAmbientLight;

	XMFLOAT4 gSpecularMtrl;
	XMFLOAT4 gSpecularLight;
	float gSpecularPower;

	XMFLOAT3 gEyePosW;
	XMFLOAT3 lightVecW;

};

class Application
{
private:
	HINSTANCE               _hInst;
	HWND                    _hWnd;
	D3D_DRIVER_TYPE         _driverType;
	D3D_FEATURE_LEVEL       _featureLevel;
	ID3D11Device*           _pd3dDevice;
	ID3D11DeviceContext*    _pImmediateContext;
	IDXGISwapChain*         _pSwapChain;
	ID3D11RenderTargetView* _pRenderTargetView;
	ID3D11VertexShader*     _pVertexShader;
	ID3D11PixelShader*      _pPixelShader;
	ID3D11InputLayout*      _pVertexLayout;
	ID3D11Buffer*           _pVertexBuffer;
	ID3D11Buffer*           _pIndexBuffer;
	ID3D11Buffer*           _pVertexBufferPlane;
	ID3D11Buffer*           _pIndexBufferPlane;
	ID3D11Buffer*           _pConstantBuffer;
	// Declaration of the interface object which we'll use to set the render state
	ID3D11RasterizerState*  _wireFrame;
	// Store the Depth/Stencil view
	ID3D11DepthStencilView* _depthStencilView;
	// Store the Depth/Stencil buffer
	ID3D11Texture2D* _depthStencilBuffer;



	// Create Object instances
	//Object* _pSun, _pWorld1, _pWorld2, _pMoon1, _pMoon2;
	GameObject _sun, _planet1, _planet2, _moon1, _moon2, _asteroid;
	GameObject asteroidBelt[ASTEROID_COUNT];
	GameObject _plane;
	MeshData _meshData;


	// Sun's world matrix
	XMFLOAT4X4              _sunWorld;
	XMFLOAT4X4              _planet1World;
	XMFLOAT4X4              _planet2World;
	XMFLOAT4X4              _moon1World;
	XMFLOAT4X4              _moon2World;
	XMFLOAT4X4              _view;
	XMFLOAT4X4              _projection;

	// Plane's world matrix
	XMFLOAT4X4              _planeWorld;

	// Camera positions
	XMVECTOR Eye;
	XMVECTOR At;
	XMVECTOR Up;

	// Lighting variables
	XMFLOAT3 lightDir;
	XMFLOAT4 ambient;
	XMFLOAT4 diffuse;




private:
	HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
	HRESULT InitDevice();
	void Cleanup();
	HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	HRESULT InitShadersAndInputLayout();
	HRESULT InitVertexBuffer();
	HRESULT InitIndexBuffer();
	void Input();

	UINT _WindowHeight;
	UINT _WindowWidth;

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;

	float upDown = 6.5f;
	float leftRight = 0.0f;
	float forwardBack = 0.0f;
	int sleepTime = 16;
	bool WFMode = false;
	bool switchScene = false;



public:
	Application();
	~Application();

	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

	void Update();
	void Draw();
};

