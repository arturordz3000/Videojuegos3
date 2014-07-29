#ifndef _CAMERA_CLASS_
#define _CAMERA_CLASS_

/**********

	Class: Camera

	Description: Clase que genera una matriz "view"
				 en primera persona.
				 Esta clase proporciona métodos
				 que simplifican la traslación y rotación
				 de una cámara en primera persona.

	Creation Date: 8 de Septiembre del 2013

	Author: Arturo de Jesús Rodríguez Arteaga

**********/
#define _XM_NO_INTRINSICS_

#include <xnamath.h>

class Camera
{
private:

	// Estructura que contiene los ejes de movimiento de la camara
	struct Movement
	{
		XMVECTOR xVelocity;
		XMVECTOR yVelocity;
		XMVECTOR zVelocity;
	};

	// Velocidad de rotacion sobre cada eje
	XMFLOAT3 rotationVelocity;

	// Velocidad de movimiento sobre cada eje
	Movement movementVelocity;

	// Posicion de la camara
	XMFLOAT3 position;

	/**
	* Traslada la camara sumandole el vector dado.
	*
	* PARAMETROS:
	*
	* vector: El vector a sumar
	*
	**/
	void TranslateCamera(XMVECTOR vector)
	{
		position.x += vector.x;
		position.y += vector.y;
		position.z += vector.z;
	}

	/**
	* Rota la camara alrededor del vector indicado.
	*
	* PARAMETROS: 
	*
	* axisVector: El vector sobre el cual girar la camara
	* angle: Angulo de rotacion
	*
	**/
	void RotateCamera(XMVECTOR axisVector, float angle)
	{
		// Obtengo el cuaternio de rotacion
		XMVECTOR rotationQuaternion = XMQuaternionRotationAxis(axisVector, angle);

		// Roto los ejes de movimiento de la camara
		RotateMovement(rotationQuaternion);
	}

	void RotateMovement(XMVECTOR q)
	{
		// Roto el eje de movimiento en X
		movementVelocity.xVelocity = XMVector3Rotate(movementVelocity.xVelocity, q);

		// Roto el eje de movimiento en Y
		movementVelocity.yVelocity = XMVector3Rotate(movementVelocity.yVelocity, q);

		// Roto el eje de movimiento en Z
		movementVelocity.zVelocity = XMVector3Rotate(movementVelocity.zVelocity, q);
	}
	
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;

public:

	Camera(XMFLOAT3 movementVelocity, XMFLOAT3 rotationVelocity, int width, int height)
	{
		// Inicializacion de la velocidad de movimiento y rotacion
		this->movementVelocity.xVelocity = XMLoadFloat3( &XMFLOAT3(movementVelocity.x, 0, 0) );
		this->movementVelocity.yVelocity = XMLoadFloat3( &XMFLOAT3(0, movementVelocity.y, 0) );
		this->movementVelocity.zVelocity = XMLoadFloat3( &XMFLOAT3(0, 0, movementVelocity.z) );
		this->rotationVelocity = rotationVelocity;

		// Posicion inicial de la camara
		XMFLOAT3 eye = XMFLOAT3(0.0f, 0.0f, -50.0f);
		XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 1.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

		position = eye;

		viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
		viewMatrix = XMMatrixTranspose( viewMatrix );

		projectionMatrix = XMMatrixPerspectiveFovLH( XM_PIDIV2, (FLOAT) width / (FLOAT) height, 0.01f, 1000.0f );
	}

	XMMATRIX GetProjectionMatrix() { return projectionMatrix; }

	void SetPositionY(float y)
	{
		this->position.y = y;
	}

	XMFLOAT3 GetPosition()
	{
		return this->position;
	}

	void MoveForward()
	{
		this->TranslateCamera(movementVelocity.zVelocity);
	}

	void MoveBackward()
	{
		this->TranslateCamera(-movementVelocity.zVelocity);
	}

	void MoveRight()
	{
		this->TranslateCamera(movementVelocity.xVelocity);
	}

	void MoveLeft()
	{
		this->TranslateCamera(-movementVelocity.xVelocity);
	}

	void MoveUp()
	{
		this->TranslateCamera(movementVelocity.yVelocity);
	}

	void MoveDown()
	{
		this->TranslateCamera(-movementVelocity.yVelocity);
	}

	void RotateOverXPositive()
	{
		this->RotateCamera(movementVelocity.xVelocity, rotationVelocity.x);
	}

	void RotateOverXNegative()
	{
		this->RotateCamera(movementVelocity.xVelocity, -rotationVelocity.x);
	}

	void RotateOverYPositive()
	{
		this->RotateCamera(XMLoadFloat3( &XMFLOAT3(0.0f, 1.0f, 0.0f) ), rotationVelocity.y);
	}

	void RotateOverYNegative()
	{
		this->RotateCamera(XMLoadFloat3( &XMFLOAT3(0.0f, 1.0f, 0.0f) ), -rotationVelocity.y);
	}

	void RotateOverZPositive(XMFLOAT3 vector)
	{
		this->RotateCamera(movementVelocity.zVelocity, rotationVelocity.z);
	}

	void RotateOverZNegative(XMFLOAT3 vector)
	{
		this->RotateCamera(movementVelocity.zVelocity, -rotationVelocity.z);
	}

	XMMATRIX GetViewMatrix()
	{
		XMVECTOR eye = XMLoadFloat3(&position);
		XMVECTOR target = eye + movementVelocity.zVelocity;
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

		viewMatrix = XMMatrixLookAtLH(eye, target, movementVelocity.yVelocity);
		viewMatrix = XMMatrixTranspose( viewMatrix );

		return viewMatrix;
	}
};

#endif