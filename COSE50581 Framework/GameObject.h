#pragma once

#include <d3d11_1.h>
#include <DirectXMath.h>
#include <iostream>
#include <vector>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <random>

using namespace DirectX;
using namespace std;

struct MeshData
{
	// This MeshData object will have 5 parameters.
	ID3D11Buffer * VertexBuffer;
	ID3D11Buffer * IndexBuffer;
	UINT VBStride;
	UINT VBOffset;
	UINT IndexCount;
};

class GameObject
{
private:
	MeshData _meshData; // We create a MeshData object

	XMFLOAT4X4 _world;

	float xDir;
	float zDir;
	//XMFLOAT4X4 _newWorld;

	mt19937 randomGenerator;
	uniform_real_distribution<float> bla{ 0.0f, 1.0f };




	//XMFLOAT4X4 _scale;
	//XMFLOAT4X4 _rotate;
	//XMFLOAT4X4 _translate;

	vector<XMMATRIX> transformations;

public:
	GameObject(void);
	~GameObject(void);

	XMFLOAT4X4 GetWorld() const { return _world; };

	void UpdateWorld();

	void SetScale(float x, float y, float z);
	void SetRotation(float x, float y, float z);
	void SetTranslation(float x, float y, float z);

	void Initialise(MeshData meshData);
	void Update(float elapsedTime);
	float GetXDir();
	float GetZDir();
	void Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext);
};

