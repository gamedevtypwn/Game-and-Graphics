#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <DirectXMath.h>
//#include "Application.h"
using namespace DirectX;

class Camera
{
private:
	XMFLOAT4 _eye;
	XMFLOAT4 _at;
	XMFLOAT4 _up;

	FLOAT _windowWidth;
	FLOAT _windowHeight;
	FLOAT _nearDepth;
	FLOAT _farDepth;

	XMFLOAT4X4 _view;
	XMFLOAT4X4 _projection;

public:
	Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up, FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
	~Camera();

	void CalculateViewProjection();

	XMFLOAT4X4 GetView() const { return _view; }
	XMFLOAT4X4 GetProjection() const { return _projection; }

	XMFLOAT4X4 GetViewProjection() const;

	XMFLOAT4 GetEye() const { return _eye; }
	XMFLOAT4 GetAt() const { return _at; }
	XMFLOAT4 GetUp() const { return _up; }

	void SetEye(XMFLOAT4 eye) { _eye = eye; }
	void SetAt(XMFLOAT4 at) { _at = at; }
	void SetUp(XMFLOAT4 up) { _up = up; }

	void Reshape(FLOAT windowWidth, FLOAT windowHeight, FLOAT nearDepth, FLOAT farDepth);
};

