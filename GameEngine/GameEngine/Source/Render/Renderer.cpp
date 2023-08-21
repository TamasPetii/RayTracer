#include "Renderer.h"

Renderer::Renderer()
{
	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);

	mTextures.push_back(Texture2D::LoadTexture2D("Assets/Sun.jpg"));
	mTextures.push_back(Texture2D::LoadTexture2D("Assets/Earth.jpg"));
	mTextures.push_back(Texture2D::LoadTexture2D("Assets/Triangle.png"));

	mCamera = new Camera(1200, 800);
	mCanvas = new Canvas();
	mSceneFrameBuffer = new FrameBuffer(1200, 800, FrameBufferType::ColorBuffer);
	mRayTraceProgram = new Program(
		{
			Shader(GL_VERTEX_SHADER, "Source/Shader/Raytrace.vert"),
			Shader(GL_FRAGMENT_SHADER, "Source/Shader/Raytrace.frag")
		},
		{
			ShaderLayout(0, "vert_position")
		}
	);
	CreateSpheres();

}

void Renderer::CreateSpheres()
{
	mSpheres.push_back(new Sphere(glm::vec3(0), 1, glm::vec3(0, 0, 0), REFLECTIVE, 0));
	mSpheres.push_back(new Sphere(glm::vec3(cosf(2 * M_PI / 3 * 1) * 5, 1, sinf(2 * M_PI / 3 * 1) * 5), 1, glm::vec3(0, 0, 0), REFLECTIVE, 0));
	mSpheres.push_back(new Sphere(glm::vec3(cosf(2 * M_PI / 3 * 2) * 5, 1, sinf(2 * M_PI / 3 * 2) * 5), 1, glm::vec3(0, 0, 0), TEXTURED, 0));
	mSpheres.push_back(new Sphere(glm::vec3(cosf(2 * M_PI / 3 * 3) * 5, 1, sinf(2 * M_PI / 3 * 3) * 5), 1, glm::vec3(0, 0, 0), TEXTURED, 1));
	mSpheres.push_back(new Sphere(glm::vec3(0, -250, 0), 500, glm::vec3(0.3, 0.85, 0.9), DIFFUSE, 0));
	mSpheres.push_back(new Sphere(glm::vec3(0, -150, 0), 150, glm::vec3(0.75, 0.85, 0.9), DIFFUSE, 0));
	static bool restart = false;
	for (int i = 0; i < 244; i++)
	{
		do
		{
			float textureRandom = GetRandom();
			int textureId = textureRandom > 0.66 ? 0 : textureRandom > 0.33 ? 1 : 2;

			float typeRandom = GetRandom();
			bool textured = typeRandom > 0.85;
			bool diffuse = typeRandom > 0.05 && !textured;
			float randomRadius = 0.4 + GetRandom() * 0.15;
			glm::vec3 randomPosition = glm::vec3((GetRandom() - 0.5) * 20, randomRadius, (GetRandom() - 0.5) * 20);
			glm::vec3 randomColor = diffuse ? glm::vec3(GetRandom(), GetRandom(), GetRandom()) : glm::vec3(0,0,0);
			MaterialType randomType = textured ? TEXTURED : diffuse ? DIFFUSE : REFLECTIVE;
			restart = Intersection(randomPosition, randomRadius);
			if (!restart) mSpheres.push_back(new Sphere(randomPosition, randomRadius, randomColor, randomType, textureId));
		}
		while (restart);
	}
	
}

Renderer::~Renderer()
{
	for (auto obj : mSpheres)
	{
		delete obj;
	}

	delete mCamera;
	delete mSceneProgram;
	delete mSceneFrameBuffer;
	Texture2D::ClearTextures();
}


void Renderer::Render()
{
	Renderer::PreRender();
	Renderer::RenderScene();
	Renderer::PostRender();
}

void Renderer::Update()
{
	static float lastTime = 0;
	float currentTime = glfwGetTime();
	float deltaTime = currentTime - lastTime;
	(*mSpheres.begin())->UpdatePosition(deltaTime);
	lastTime = currentTime;
}

void Renderer::PreRender()
{
	mSceneFrameBuffer->Clear();
	UploadSpheresToShader();

	
	mRayTraceProgram->Bind();
	mRayTraceProgram->SetUniformTexture("uTextures[0]", 0, mTextures[0]);
	mRayTraceProgram->SetUniformTexture("uTextures[1]", 1, mTextures[1]);
	mRayTraceProgram->SetUniformTexture("uTextures[2]", 2, mTextures[2]);
	mRayTraceProgram->UnBind();	
}

void Renderer::PostRender()
{
}

void Renderer::RenderScene()
{
	mSceneFrameBuffer->Bind();
	mRayTraceProgram->Bind();
	mRayTraceProgram->SetUniform("uViewProjMatrix", mCamera->GetViewProjMatrix());
	mRayTraceProgram->SetUniform("uCameraEye", mCamera->GetCameraEye());
	mRayTraceProgram->SetUniform("uPathDepth", 10);
	mRayTraceProgram->SetUniform("uLightDirection", glm::vec3(-1,-1,-1));
	mCanvas->Draw();
	mRayTraceProgram->UnBind();
	mSceneFrameBuffer->UnBind();
}

void Renderer::UploadSpheresToShader()
{
	int counter = 0;
	mRayTraceProgram->Bind();

	for (auto sphere : mSpheres) 
	{
		mRayTraceProgram->SetUniform("uSpheres[" + std::to_string(counter) + "].type", (int)sphere->material.type + 100 * sphere->material.textureId);
		mRayTraceProgram->SetUniform("uSpheres[" + std::to_string(counter) + "].color", sphere->material.color);
		mRayTraceProgram->SetUniform("uSpheres[" + std::to_string(counter) + "].origin", sphere->origin);
		mRayTraceProgram->SetUniform("uSpheres[" + std::to_string(counter) + "].radius", sphere->radius);
		counter++;
	}
	mRayTraceProgram->SetUniform("uSphereCount", counter);
	mRayTraceProgram->UnBind();
}

float Renderer::GetRandom()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

bool Renderer::Intersection(glm::vec3 origin, float radius)
{
	glm::vec3 originVec;
	float originLength;
	float radiusLength;
	for (auto obj : mSpheres)
	{
		originVec = obj->origin - origin;
		originLength = glm::dot(originVec, originVec);
		radiusLength = obj->radius + radius;
		if (originLength < radiusLength) return true;
	}
	return false;
}
