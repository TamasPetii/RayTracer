#pragma once
#define _USE_MATH_DEFINES
#include "../Engine/Engine.h"
#include <cmath>


class Canvas
{
public:
	Canvas();
	~Canvas();
	void Draw();
	void CreateVertices();
	void CreateIndices();
private:
	void UploadBuffers();
	std::vector<glm::vec3> mVertices;
	std::vector<unsigned int> mIndices;
	VertexArrayObject* mVao;
	DataBuffer<glm::vec3>* mVbo;
	IndexBufferObject* mIbo;
	unsigned int mIndexCount;
	unsigned int mVertexCount;
};

