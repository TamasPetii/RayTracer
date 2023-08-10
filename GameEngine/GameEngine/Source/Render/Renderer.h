#pragma once
#define _USE_MATH_DEFINES
#include "../Engine/Engine.h"
#include "Canvas.h"
#include "GameObjects/Lights/DirectionLight.h"
#include "GameObjects/Lights/PointLight.h"
#include "Sphere.h"
#include <unordered_set>
#include <stdlib.h>
#include <cmath>

class Renderer
{
public:
	Renderer();
	~Renderer();

	void Render();
	void Update();
	inline FrameBuffer* GetSceneFrameBuffer() const { return mSceneFrameBuffer; }
	inline Camera* GetCamera() const { return mCamera; }
private:
	void PreRender();
	void PostRender();
	void RenderScene();
	void CreateSpheres();
	void UploadSpheresToShader();
	float GetRandom();
	bool Intersection(glm::vec3 origin, float radius);

	Program* mRayTraceProgram;
	Program* mSceneProgram;
	FrameBuffer* mSceneFrameBuffer;
	Camera* mCamera;
	Canvas* mCanvas;
	DirectionLight* mDirectionLight;
	std::unordered_set<Sphere*> mSpheres;
};