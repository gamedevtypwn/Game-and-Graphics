#include "GameObject.h"



GameObject::GameObject(void)
{
}

GameObject::~GameObject(void)
{
}

void GameObject::Initialise(MeshData meshData)
{
	_meshData = meshData;

	XMStoreFloat4x4(&_world, XMMatrixIdentity());
	//XMStoreFloat4x4(&_scale, XMMatrixIdentity());
	//XMStoreFloat4x4(&_rotate, XMMatrixIdentity());
	//XMStoreFloat4x4(&_translate, XMMatrixIdentity());

	cout << "Number:" << bla << endl;

	//srand(time(NULL));
	xDir = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	zDir = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

	//need to normalize it
	float length = sqrt(xDir * xDir + zDir + zDir);
	xDir /= length;
	zDir /= length;

	//we have a direction, now we need a radius to get the final position
	float radius = static_cast <float> (rand()) / static_cast <float> (RAND_MAX/6.0f);
	xDir *= radius;
	zDir *= radius;
}

// I believe these three methods are to set the scale, rot, and trans for the initial creation of the object, not for updating with.
void GameObject::SetScale(float x, float y, float z)
{
	//XMStoreFloat4x4(transformations.push_back, XMMatrixScaling(x, y, z));
	transformations.push_back((XMMatrixScaling(x, y, z)));
}

void GameObject::SetRotation(float x, float y, float z)
{
	//XMStoreFloat4x4(transformations.push_back, XMMatrixRotationX(x) * XMMatrixRotationY(y) * XMMatrixRotationZ(z));
	transformations.push_back((XMMatrixRotationX(x) * XMMatrixRotationY(y) * XMMatrixRotationZ(z)));
}

void GameObject::SetTranslation(float x, float y, float z)
{
	//XMStoreFloat4x4(transformations.push_back, XMMatrixTranslation(x, y, z));
	transformations.push_back(XMMatrixTranslation(x, y, z));
}

void GameObject::UpdateWorld()
{
	XMFLOAT4X4 _newWorld;
	XMStoreFloat4x4(&_newWorld, XMMatrixIdentity());
	XMMATRIX newWorld = XMLoadFloat4x4(&_newWorld);
	//XMStoreFloat4x4(newWorld, XMMatrixIdentity());
	for (auto T : transformations) //assuming your vector is called "transformations"
	{
		newWorld *= T;
	}
	XMStoreFloat4x4(&_world, newWorld);
	//set the final world matrix equal to whatever is stored in the newWorld matrix
	//_world = newWorld;
	transformations.clear();
	//XMMATRIX scale = XMLoadFloat4x4(&_scale);
	//XMMATRIX rotate = XMLoadFloat4x4(&_rotate);
	//XMMATRIX translate = XMLoadFloat4x4(&_translate);

	//XMStoreFloat4x4(&_world, scale * rotate * translate);
}

void GameObject::Update(float elapsedTime)
{
	// TODO: Add GameObject logic 
}

float GameObject::GetXDir()
{
	return xDir;
}

float GameObject::GetZDir()
{
	return zDir;
}



void GameObject::Draw(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pImmediateContext)
{
	// NOTE: We are assuming that the constant buffers and all other draw setup has already taken place

	// Set vertex and index buffers
	pImmediateContext->IASetVertexBuffers(0, 1, &_meshData.VertexBuffer, &_meshData.VBStride, &_meshData.VBOffset);
	pImmediateContext->IASetIndexBuffer(_meshData.IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	pImmediateContext->DrawIndexed(_meshData.IndexCount, 0, 0);
}