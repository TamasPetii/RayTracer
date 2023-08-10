#include "Canvas.h"

Canvas::Canvas()
{
	mVao = new VertexArrayObject();
	mIbo = new IndexBufferObject();
	mVbo = new DataBuffer<glm::vec3>();
	CreateVertices();
	CreateIndices();
	UploadBuffers();
}

Canvas::~Canvas()
{
	delete mVao;
	delete mIbo;
	delete mVbo;
}

void Canvas::CreateVertices()
{
	mVertices = {
		glm::vec3(-1, 1, -1),
		glm::vec3(-1, -1, -1),
		glm::vec3(1, -1, -1),
		glm::vec3(1, 1, -1)
	};
}

void Canvas::CreateIndices()
{
	mIndices =
	{
		0, 1, 2,
		0, 2, 3
	};
}

void Canvas::UploadBuffers()
{
	mIndexCount = mIndices.size();
	mVertexCount = mVertices.size();

	mVao->Bind();
	mVbo->AttachData(mVertices, GL_STATIC_DRAW);
	mIbo->AttachData(mIndices, GL_STATIC_DRAW);

	mVao->LinkAttribute(*mVbo, 0, 3, GL_FLOAT, sizeof(glm::vec3), nullptr);

	mVao->UnBind();
	mVbo->UnBind();
	mIbo->UnBind();
}

void Canvas::Draw()
{
	mVao->Bind();
	glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_INT, nullptr);
	mVao->UnBind();
}