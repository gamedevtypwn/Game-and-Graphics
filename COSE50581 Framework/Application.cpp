#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

Application::Application()
{
	_hInst = nullptr;
	_hWnd = nullptr;
	_driverType = D3D_DRIVER_TYPE_NULL;
	_featureLevel = D3D_FEATURE_LEVEL_11_0;
	_pd3dDevice = nullptr;
	_pImmediateContext = nullptr;
	_pSwapChain = nullptr;
	_pRenderTargetView = nullptr;
	_pVertexShader = nullptr;
	_pPixelShader = nullptr;
	_pVertexLayout = nullptr;
	_pVertexBuffer = nullptr;
	_pIndexBuffer = nullptr;
	_pConstantBuffer = nullptr;
}

Application::~Application()
{
	Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow)
{
	if (FAILED(InitWindow(hInstance, nCmdShow)))
	{
		return E_FAIL;
	}

	RECT rc;
	GetClientRect(_hWnd, &rc);
	_WindowWidth = rc.right - rc.left;
	_WindowHeight = rc.bottom - rc.top;

	if (FAILED(InitDevice()))
	{
		Cleanup();

		return E_FAIL;
	}

	// Initialise the mesh data for the first cube (The Sun), then initialise the first cube
	_meshData.VertexBuffer = _pVertexBuffer;
	_meshData.IndexBuffer = _pIndexBuffer;
	// VBStride is simply the amount of memory each element in the array will take up. So because each element is a SimpleVertex, we want each element to take up the memeory of one SimpleVertex object.
	_meshData.VBStride = sizeof(SimpleVertex);
	// The VBOffset is just the index of the vertex buffer array, so we want to start at the first element which is 0.
	_meshData.VBOffset = 0;
	_meshData.IndexCount = 36;

	_sun.Initialise(_meshData);
	_planet1.Initialise(_meshData);
	_planet2.Initialise(_meshData);
	_moon1.Initialise(_meshData);
	_moon2.Initialise(_meshData);

	for (int i = 0; i < ASTEROID_COUNT; i++)
	{
		asteroidBelt[i] = _asteroid;
		asteroidBelt[i].Initialise(_meshData);
	}

	srand(time(NULL));

	// Initialise mesh data for the plane
	_meshData.VertexBuffer = _pVertexBufferPlane;
	_meshData.IndexBuffer = _pIndexBufferPlane;

	_plane.Initialise(_meshData);

	// Initialise the lighting variables
	lightDir = XMFLOAT3(0.25f, 0.5f, -1.0f);
	ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);


	// Initialize the world matrix
	//XMStoreFloat4x4(&_sunWorld, XMMatrixIdentity());
	//XMStoreFloat4x4(&_planet1World, XMMatrixIdentity());
	//XMStoreFloat4x4(&_planet2World, XMMatrixIdentity());
	//XMStoreFloat4x4(&_moon1World, XMMatrixIdentity());
	//XMStoreFloat4x4(&_moon2World, XMMatrixIdentity());

	//for (int i = 0; i < ASTEROID_COUNT; i++)
	//{
	//	XMStoreFloat4x4(&_moon2World, XMMatrixIdentity());
	//}

	// Initialize the view matrix
	//XMVECTOR Eye = XMVectorSet(0.0f, 6.5f, -20.0f, 0.0f);
	//XMVECTOR At = XMVectorSet(0.0f, -3.0f, 0.0f, 0.0f);
	//XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);



	// Up and Down
	//Eye = XMVectorSet(0.0f, upDown, -100.0f, 0.0f);
	Eye = XMVectorSet(0.0f, 10.0f, -10.0f, 0.0f);
	// Left and Right
	//XMVECTOR At = XMVectorSet(leftRight, -9.0f, 0.0f, leftRight);
	At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	//XMVECTOR Up = XMVectorSet(0.0f, 20.0f, 0.0f, 0.0f);
	Up = XMVectorSet(10.0f, 0.0f, 0.0f, 0.0f);

	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

	// Initialize the projection matrix
	XMStoreFloat4x4(&_projection, XMMatrixPerspectiveFovLH(XM_PIDIV2, _WindowWidth / (FLOAT)_WindowHeight, 0.01f, 100.0f));

	return S_OK;
}

HRESULT Application::InitShadersAndInputLayout()
{
	HRESULT hr;

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	hr = CompileShaderFromFile(L"Lighting.fx", "VS", "vs_4_0", &pVSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

	// Compile the pixel shader
	ID3DBlob* pPSBlob = nullptr;
	hr = CompileShaderFromFile(L"Lighting.fx", "PS", "ps_4_0", &pPSBlob);

	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the pixel shader
	hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &_pPixelShader);
	pPSBlob->Release();

	if (FAILED(hr))
		return hr;

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};


	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
		pVSBlob->GetBufferSize(), &_pVertexLayout);
	pVSBlob->Release();

	if (FAILED(hr))
		return hr;

	// Set the input layout
	_pImmediateContext->IASetInputLayout(_pVertexLayout);

	return hr;
}

HRESULT Application::InitVertexBuffer()
{
	HRESULT hr;


	// Create vertex buffer for Cube 1
	SimpleVertex vertices[] =
	{	// Top Left - v0
		{ XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 1.0f, -1.0f) },
		// Top right - v1		 
		{ XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, -1.0f) },
		// Bottom left - v2		 
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f) },
		// Bottom right - v3	 
		{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f) },
		// Top left Z=1 - v4
		{ XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 1.0f, 1.0f) },
		// Top right Z=1 - v5				  
		{ XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) },
		// Bottom left Z=1 - v6				  
		{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f) },
		// Bottom right Z=1 - v7			  
		{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f) },
	};

	//SimpleVertex planeVertices[] =
	//{	// Top Left - v0
	//	{ XMFLOAT3(-1.0f, -0.9f, -1.0f), XMFLOAT3(-1.0f, -0.1f, -1.0f) },
	//	// Top right - v1		 
	//	{ XMFLOAT3(1.0f, -0.9f, -1.0f), XMFLOAT3(1.0f, -0.1f, -1.0f) },
	//	// Bottom left - v2		 
	//	{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, -1.0f, -1.0f) },
	//	// Bottom right - v3	 
	//	{ XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, -1.0f, -1.0f) },
	//	// Top left Z=1 - v4
	//	{ XMFLOAT3(-1.0f, -0.9f, 1.0f), XMFLOAT3(-1.0f, -0.9f, 1.0f) },
	//	// Top right Z=1 - v5				  
	//	{ XMFLOAT3(1.0f, -0.9f, 1.0f), XMFLOAT3(1.0f, -0.9f, 1.0f) },
	//	// Bottom left Z=1 - v6				  
	//	{ XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, -1.0f, 1.0f) },
	//	// Bottom right Z=1 - v7			  
	//	{ XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, -1.0f, 1.0f) },
	//};


	SimpleVertex planeVertices[] =
	{	// Top Left - v0
		{ XMFLOAT3(-100.0f, -99.0f, -100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Top right - v1		 
		{ XMFLOAT3(100.0f, -99.0f, -100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Bottom left - v2		 
		{ XMFLOAT3(-100.0f, -100.0f, -100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Bottom right - v3	 
		{ XMFLOAT3(100.0f, -100.0f, -100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Top left Z=1 - v4
		{ XMFLOAT3(-100.0f, -99.0f, 100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Top right Z=1 - v5				  
		{ XMFLOAT3(100.0f, -99.0f, 100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Bottom left Z=1 - v6				  
		{ XMFLOAT3(-100.0f, -100.0f, 100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
		// Bottom right Z=1 - v7			  
		{ XMFLOAT3(100.0f, -100.0f, 100.0f), XMFLOAT3(0.0f, 1.0f, 0.0f) },
	};


	//D3D11 buffer for Cube
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SimpleVertex) * 8;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;

	// Store the vertices for our cube
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = vertices;

	// Create the Vertex Buffer for the solar system
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

	if (FAILED(hr))
		return hr;

	// Store the vertices for our plane
	InitData.pSysMem = planeVertices;

	// Create the Vertex Buffer for the plane
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBufferPlane);

	// Cube check
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Application::InitIndexBuffer()
{
	HRESULT hr;

	// Create index buffer for cube
	WORD indices[] =
	{
		//Front
		0, 1, 2,
		2, 1, 3,
		//Left Side
		4, 0, 6,
		6, 0, 2,
		//Back
		5, 4, 6,
		5, 6, 7,
		//Right Side
		3, 1, 5,
		3, 5, 7,
		//Top
		4, 5, 1,
		4, 1, 0,
		//Bottom
		6, 3, 7,
		6, 2, 3


	};

	// Create index buffer for plane
	WORD planeIndices[] =
	{
		//Front
		0, 1, 2,
		2, 1, 3,
		//Left Side
		4, 0, 6,
		6, 0, 2,
		//Back
		5, 4, 6,
		5, 6, 7,
		//Right Side
		3, 1, 5,
		3, 5, 7,
		//Top
		4, 5, 1,
		4, 1, 0,
		//Bottom
		6, 3, 7,
		6, 2, 3


	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * 36;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = indices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

	if (FAILED(hr))
		return hr;

	InitData.pSysMem = planeIndices;
	hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBufferPlane);

	if (FAILED(hr))
		return hr;


	return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// Create window
	_hInst = hInstance;
	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	_hWnd = CreateWindow(L"TutorialWindowClass", L"DX11 Framework", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
		nullptr);
	if (!_hWnd)
		return E_FAIL;

	ShowWindow(_hWnd, nCmdShow);

	return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;
	hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

	if (FAILED(hr))
	{
		if (pErrorBlob != nullptr)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		if (pErrorBlob) pErrorBlob->Release();

		return hr;
	}

	if (pErrorBlob) pErrorBlob->Release();

	return S_OK;
}

HRESULT Application::InitDevice()
{
	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;

#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};

	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = _WindowWidth;
	sd.BufferDesc.Height = _WindowHeight;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = _hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
		if (SUCCEEDED(hr))
			break;
	}

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = nullptr;
	hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	if (FAILED(hr))
		return hr;

	// Define our depth/stencil buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = _WindowWidth;
	depthStencilDesc.Height = _WindowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	// End of depth/stencil definition

	// Create depth/stencil buffer:

	// The first parameter is the depth/stencil description, the second parameter is the state (we don't have one so we set it to nullptr, and the third is the returned depth/stencil buffer.
	_pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
	_pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);


	// Now we need to bind the depth to the OM stage of the pipeline

	hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
	pBackBuffer->Release();

	if (FAILED(hr))
		return hr;

	// Originally the third parameter was set to nullptr because we didn't have a depth/stencil view. But now we have one!
	_pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

	// This struct object tells the program the type of rasterizer state we want to create.
	D3D11_RASTERIZER_DESC wfdesc;
	// This makes sure the memory is cleared
	ZeroMemory(&wfdesc, sizeof(D3D11_RASTERIZER_DESC));
	// D3D11_FILL_WIREFRAME for wireframe rendering, or D3D11_FILL_SOLID for solid rendering
	wfdesc.FillMode = D3D11_FILL_WIREFRAME;
	// Disables culling, which means we can see the backs of the cubes as they spin.
	wfdesc.CullMode = D3D11_CULL_NONE;
	// Creates the new render state. The render state will be bound to the RS (RenderState)
	// stage of the pipeline, so we create the render state with the ID3D11Device::CreateRasterizerState()
	// method. The first parameter is the description of our render state, and the second is a pointer
	// to a ID3D11RasterizerState object which will hold our new render state.
	hr = _pd3dDevice->CreateRasterizerState(&wfdesc, &_wireFrame);

	// Sometimes different object need different render states, don't set the render state at start of program,
	// set it before you render each object. Default render state = nullptr.
	_pImmediateContext->RSSetState(_wireFrame);


	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)_WindowWidth;
	vp.Height = (FLOAT)_WindowHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	_pImmediateContext->RSSetViewports(1, &vp);

	InitShadersAndInputLayout();

	InitVertexBuffer();

	// Set vertex buffer
	//_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);

	InitIndexBuffer();

	// Set index buffer
	//_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	// Set primitive topology
	_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Application::Input()
{
	if (GetAsyncKeyState(0x57))
	{
		++upDown;
		//Eye = XMVectorSet(0.0f, up * elapsed, -10.0f, 0.0f);
		Sleep(sleepTime);
	}

	// Down
	if (GetAsyncKeyState(0x53))
	{
		--upDown;
		//Eye = XMVectorSet(0.0f, up * elapsed, -10.0f, 0.0f);
		Sleep(sleepTime);
	}


	//Left
	if (GetAsyncKeyState(0x41))
	{
		--leftRight;
		//Eye = XMVectorSet(0.0f, up * elapsed, -10.0f, 0.0f);
		Sleep(sleepTime);
	}

	//Right
	if (GetAsyncKeyState(0x44))
	{
		++leftRight;
		//Eye = XMVectorSet(0.0f, up * elapsed, -10.0f, 0.0f);
		Sleep(sleepTime);
	}

	//Back
	if (GetAsyncKeyState(0x28))
	{
		++forwardBack;
		//Eye = XMVectorSet(0.0f, up * elapsed, -10.0f, 0.0f);
		Sleep(sleepTime);
	}

	//Forward
	if (GetAsyncKeyState(0x26))
	{
		--forwardBack;
		//Eye = XMVectorSet(0.0f, up * elapsed, -10.0f, 0.0f);
		Sleep(sleepTime);
	}

	if (GetAsyncKeyState(VK_RETURN))
	{
		WFMode = !WFMode;
		Sleep(sleepTime);
	}

	if (GetAsyncKeyState(0x4B))
	{
		switchScene = true;
		Sleep(sleepTime);
	}

	if (GetAsyncKeyState(0x4C))
	{
		switchScene = false;
		Sleep(sleepTime);
	}
}

void Application::Cleanup()
{
	if (_pImmediateContext) _pImmediateContext->ClearState();

	if (_pConstantBuffer) _pConstantBuffer->Release();
	if (_pVertexBuffer) _pVertexBuffer->Release();
	if (_pIndexBuffer) _pIndexBuffer->Release();
	if (_pVertexLayout) _pVertexLayout->Release();
	if (_pVertexShader) _pVertexShader->Release();
	if (_pPixelShader) _pPixelShader->Release();
	if (_pRenderTargetView) _pRenderTargetView->Release();
	if (_pSwapChain) _pSwapChain->Release();
	if (_pImmediateContext) _pImmediateContext->Release();
	if (_pd3dDevice) _pd3dDevice->Release();
	if (_depthStencilView) _depthStencilView->Release();
	if (_depthStencilBuffer) _depthStencilBuffer->Release();
	if (_wireFrame) _wireFrame->Release();
}

void Application::Update()

{
	// Update our time
	static float t = 0.0f;
	static float elapsed = t;
	static float oldT;

	if (_driverType == D3D_DRIVER_TYPE_REFERENCE)
	{
		t += (float)XM_PI * 0.0125f;
	}
	else
	{
		static DWORD dwTimeStart = 0;
		DWORD dwTimeCur = GetTickCount();

		if (dwTimeStart == 0)
			dwTimeStart = dwTimeCur;
		oldT = t;
		t = (dwTimeCur - dwTimeStart) / 1000.0f;
		elapsed = t - oldT;
	}

	//
	// Animate the cubes
	//

	// Cube 1 transformation (The Sun)
	//XMStoreFloat4x4(&_sunWorld, XMMatrixScaling(0.75f, 0.75f, 0.75f) * XMMatrixRotationY(t));

	// Cube 1 GameObject transformation (The Sun)
	_sun.SetScale(0.75f, 0.75f, 0.75f);
	_sun.SetRotation(0.0f, t, 0.0f);
	_sun.SetTranslation(0.0f, 10.0f, 0.0f);
	_sun.UpdateWorld();

	// Cube 2 transformation (Planet 1 - left)
	//XMStoreFloat4x4(&_planet1World, XMMatrixScaling(0.5f, 0.5f, 0.5f) *  XMMatrixRotationY(-t) * XMMatrixTranslation(-3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	_planet1.SetScale((0.5f), (0.5f), (0.5f));
	_planet1.SetRotation(0.0f, (-t), 0.0f);
	_planet1.SetTranslation((-3.00f), 10.0f, 0.0f);
	_planet1.SetRotation(0.0f, (-t), 0.0f);
	_planet1.UpdateWorld();

	// Cube 3 transformation (Planet 2 - right)
	//XMStoreFloat4x4(&_planet2World, XMMatrixScaling(0.5f, 0.5f, 0.5f) *  XMMatrixRotationY(-t) * XMMatrixTranslation(3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	_planet2.SetScale((0.5f), (0.5f), (0.5f));
	_planet2.SetRotation(0.0f, (-t), 0.0f);
	_planet2.SetTranslation((3.00f), 10.0f, 0.0f);
	_planet2.SetRotation(0.0f, (-t), 0.0f);
	_planet2.UpdateWorld();

	// Cube 4 transformation (Moon 1 - left)
	//XMStoreFloat4x4(&_moon1World, XMMatrixRotationY(-t) * XMMatrixTranslation(-5.00f, 0.0f, 0.0f) * XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixRotationY(-t * 3) * XMMatrixTranslation(-3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	_moon1.SetRotation(0.0f, (-t), 0.0f);
	_moon1.SetTranslation((-5.00f), 0.0f, 0.0f);
	_moon1.SetScale(0.25f, 0.25f, 0.25f);
	_moon1.SetRotation(0.0f, (-t * 3), 0.0f);
	_moon1.SetTranslation((-3.00f), 10.0f, 0.0f);
	_moon1.SetRotation(0.0f, (-t), 0.0f);
	_moon1.UpdateWorld();

	// Cube 5 transformation (Moon 2 - right)
	//XMStoreFloat4x4(&_moon2World, XMMatrixRotationY(-t) * XMMatrixTranslation(5.00f, 0.0f, 0.0f) * XMMatrixScaling(0.25f, 0.25f, 0.25f) * XMMatrixRotationY(-t * 3) * XMMatrixTranslation(3.00f, 0.0f, 0.0f) * XMMatrixRotationY(-t));
	_moon2.SetRotation(0.0f, (-t), 0.0f);
	_moon2.SetTranslation(5.00f, 0.0f, 0.0f);
	_moon2.SetScale(0.25f, 0.25f, 0.25f);
	_moon2.SetRotation(0.0f, (-t * 3), 0.0f);
	_moon2.SetTranslation(3.00f, 10.0f, 0.0f);
	_moon2.SetRotation(0.0f, (-t), 0.0f);
	_moon2.UpdateWorld();


	for (int i = 0; i < ASTEROID_COUNT; i++)
	{
		//srand(time(NULL));
		//float xDir = rand() % (2 + 1);
		//float zDir = rand() % (2 + 1);

		////need to normalize it
		//float length = sqrt(xDir * xDir + zDir + zDir);
		//xDir /= length;
		//zDir /= length;

		////we have a direction, now we need a radius to get the final position
		//float radius = rand() % (3 + 1);
		//xDir *= radius;
		//zDir *= radius;

		float xDir = asteroidBelt[i].GetXDir();
		float zDir = asteroidBelt[i].GetZDir();

		//asteroidBelt[i].SetRotation(0.0f, (-t), 0.0f);
		//asteroidBelt[i].SetTranslation(6.0f, 0.0f, 0.0f);
		////asteroidBelt[i].SetTranslation(3.0f, 0.0f, 0.0f);
		//asteroidBelt[i].SetScale(0.01f, 0.01f, 0.01f);
		//asteroidBelt[i].SetRotation(0.0f, (-t * 3), 0.0f);
		////asteroidBelt[i].SetTranslation(xDir, 0.0f, zDir);
		//asteroidBelt[i].SetTranslation(xDir, 0.0f, zDir);
		////asteroidBelt[i].SetTranslation(3.0f, 0.0f, 0.0f);
		//asteroidBelt[i].SetRotation(0.0f, (-t), 0.0f);
		//asteroidBelt[i].UpdateWorld();

		//asteroidBelt[i].SetRotation(0.0f, (-t), 0.0f);
		//asteroidBelt[i].SetTranslation(6.0f, 0.0f, 0.0f);
		//asteroidBelt[i].SetTranslation(3.0f, 0.0f, 0.0f);
		asteroidBelt[i].SetScale(0.01f, 0.01f, 0.01f);
		//asteroidBelt[i].SetRotation(0.0f, (-t * 3), 0.0f);
		////asteroidBelt[i].SetTranslation(xDir, 0.0f, zDir);
		asteroidBelt[i].SetTranslation(xDir, 0.0f, zDir);
		////asteroidBelt[i].SetTranslation(3.0f, 0.0f, 0.0f);
		//asteroidBelt[i].SetRotation(0.0f, (-t), 0.0f);
		asteroidBelt[i].UpdateWorld();
	}


	Input();

	//Eye = XMVectorSet(0.0f, upDown, -60.0f, 0.0f);
	//Eye = XMVectorSet(0.0f, 0.0f, -60.0f, 0.0f);
	// Moves right and left, up and down on the camera's own axis
	At = XMVectorSet(leftRight, upDown, forwardBack, 0.0f);
	Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	XMStoreFloat4x4(&_view, XMMatrixLookAtLH(Eye, At, Up));

}

void Application::Draw()
{
	//
	// Clear the back buffer
	//
	float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; // red,green,blue,alpha
	_pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
	/*Now we need to clear the depth/stencil view every frame, like we do with
	the above RenderTargetView.
	The first value is the depth/stencil view we want to clear, the second is
	an enumerated type, ordered together, specifying the part of the depth
	/stencil view we want to clear, and the fourth parameter is the value we
	want to clear the depth to. We set this to 1.0f, since 1.0f is the
	largest depth value anything can have. This makes sure everything is drawn
	on the screen. If it were 0.0f, nothing would be drawn since all the depth
	values of the pixel fragments would be between 0.0f and 1.0f. The last
	parameter is the value we set the stencil to. We set it to 0 since we're
	not using it.*/

	_pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	if (WFMode)
	{
		_pImmediateContext->RSSetState(_wireFrame);
	}
	else
	{
		_pImmediateContext->RSSetState(nullptr);
	}

	// Declare and initialise the WVP matrices
	XMMATRIX world;
	XMMATRIX view = XMLoadFloat4x4(&_view);
	XMMATRIX projection = XMLoadFloat4x4(&_projection);

	//
	// Update variables
	//

	// Setup the constant buffer
	ConstantBuffer cb;
	cb.mWorld;
	cb.mView = XMMatrixTranspose(view);
	cb.mProjection = XMMatrixTranspose(projection);
	cb.diffuseMaterial = XMFLOAT4(0.25f, 0.5f, 1.0f, 1.0f);
	cb.diffuseLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.lightVecW = XMFLOAT3(0.0f, 0.0f, -1.0f);
	cb.gAmbientLight = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	cb.gAmbientMtrl = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	cb.gSpecularMtrl = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	cb.gSpecularLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	cb.gSpecularPower = 10.0f;
	XMStoreFloat3(&cb.gEyePosW, Eye);


	// Setup our Vertex Shader and our Pixel Shader
	_pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
	_pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
	_pImmediateContext->PSSetShader(_pPixelShader, nullptr, 0);
	_pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);

	if (switchScene)
	{
		_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBuffer, &stride, &offset);
		_pImmediateContext->IASetIndexBuffer(_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

		// Load the first world (Sun) matrix to CPU
		// (From GameObject object) Load the first world (Sun) matrix to CPU
		//world = XMLoadFloat4x4(&_sunWorld);
		world = XMLoadFloat4x4(&_sun.GetWorld());
		// Prime the first world matrix for passing to GPU
		cb.mWorld = XMMatrixTranspose(world);
		// Pass the first world matrix to GPU
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		// Draw the first world matrix
		//_pImmediateContext->DrawIndexed(36, 0, 0);   
		_sun.Draw(_pd3dDevice, _pImmediateContext);

		for (int i = 0; i < ASTEROID_COUNT; i++)
		{
			world = XMLoadFloat4x4(&asteroidBelt[i].GetWorld());
			cb.mWorld = XMMatrixTranspose(world);
			_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
			//asteroidBelt[i].Draw(_pd3dDevice, _pImmediateContext);
		}


		// Load the fourth world (Moon 1) matrix to CPU
		//world = XMLoadFloat4x4(&_moon1World);
		world = XMLoadFloat4x4(&_moon1.GetWorld());
		// Prime the fourth world matrix for passing to GPU
		cb.mWorld = XMMatrixTranspose(world);
		// Pass the fourth world matrix to GPU
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		// Draw the fourth world matrix
		//_pImmediateContext->DrawIndexed(36, 0, 0);
		_moon1.Draw(_pd3dDevice, _pImmediateContext);

		// Load the fifth world (Moon 2) matrix to CPU
		//world = XMLoadFloat4x4(&_moon2World);
		world = XMLoadFloat4x4(&_moon2.GetWorld());
		// Prime the fifth world matrix for passing to GPU
		cb.mWorld = XMMatrixTranspose(world);
		// Pass the fifth world matrix to GPU
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		// Draw the fifth world matrix
		//_pImmediateContext->DrawIndexed(36, 0, 0);
		_moon2.Draw(_pd3dDevice, _pImmediateContext);



		_pImmediateContext->RSSetState(_wireFrame);
		// Load the second world (Planet 1) matrix to CPU
		//world = XMLoadFloat4x4(&_planet1World);
		world = XMLoadFloat4x4(&_planet1.GetWorld());
		// Prime the second world matrix for passing to GPU
		cb.mWorld = XMMatrixTranspose(world);
		// Pass the second world matrix to GPU
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		// Draw the second world matrix
		//_pImmediateContext->DrawIndexed(36, 0, 0);
		_planet1.Draw(_pd3dDevice, _pImmediateContext);

		//_pImmediateContext->RSSetState(_wireFrame);
		// Load the third world (Planet 2) matrix to CPU
		//world = XMLoadFloat4x4(&_planet2World);
		world = XMLoadFloat4x4(&_planet2.GetWorld());
		// Prime the third world matrix for passing to GPU
		cb.mWorld = XMMatrixTranspose(world);
		// Pass the third world matrix to GPU
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		// Draw the third world matrix
		//_pImmediateContext->DrawIndexed(36, 0, 0);
		_planet2.Draw(_pd3dDevice, _pImmediateContext);

	}
	else
	{
		_pImmediateContext->IASetVertexBuffers(0, 1, &_pVertexBufferPlane, &stride, &offset);
		_pImmediateContext->IASetIndexBuffer(_pIndexBufferPlane, DXGI_FORMAT_R16_UINT, 0);

		// Render the plane
		world = XMLoadFloat4x4(&_plane.GetWorld());
		// Prime the first world matrix for passing to GPU
		cb.mWorld = XMMatrixTranspose(world);
		// Pass the first world matrix to GPU
		_pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
		// Draw the first world matrix
		//_pImmediateContext->DrawIndexed(36, 0, 0);   
		_plane.Draw(_pd3dDevice, _pImmediateContext);
	}



	//
	// Present our back buffer to our front buffer
	//
	_pSwapChain->Present(0, 0);
}