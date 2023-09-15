#include "Renderer.h"

Renderer::Renderer()
{
	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearColor(0.2f, 0.3f, 0.6f, 1.f);

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
	
	int blockIndex, blockSize;
	blockIndex = glGetUniformBlockIndex(mRayTraceProgram->GetProgramId(), "Spheres");
	glGetActiveUniformBlockiv(mRayTraceProgram->GetProgramId(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

	const GLchar* names[] = { "type", "color", "origin", "radius" };
	GLuint indices[4];
	glGetUniformIndices(mRayTraceProgram->GetProgramId(), 4, names, indices);

	GLint offset[4];
	glGetActiveUniformsiv(mRayTraceProgram->GetProgramId(), 4, indices, GL_UNIFORM_OFFSET, offset);
	GLint count[4];
	glGetActiveUniformsiv(mRayTraceProgram->GetProgramId(), 4, indices, GL_UNIFORM_SIZE, count);
	
	GLint size[4];
	size[0] = (offset[1] - offset[0]) / count[0];
	size[1] = (offset[2] - offset[1]) / count[1];
	size[2] = (offset[3] - offset[2]) / count[2];
	size[3] = (blockSize - offset[3]) / count[3];

	glGenBuffers(1, &uniformBufferId);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferId);
	glBufferData(GL_UNIFORM_BUFFER, blockSize, nullptr, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBufferId);
	
	int index = 0;
	for (auto sphere : mSpheres)
	{
		int type = sphere->material.type + sphere->material.textureId * 100;
		glBufferSubData(GL_UNIFORM_BUFFER, offset[0] + index * size[0], sizeof(int), &(type));
		glBufferSubData(GL_UNIFORM_BUFFER, offset[1] + index * size[1], sizeof(glm::vec3), &sphere->material.color);
		glBufferSubData(GL_UNIFORM_BUFFER, offset[2] + index * size[2], sizeof(glm::vec3), &sphere->origin);
		glBufferSubData(GL_UNIFORM_BUFFER, offset[3] + index * size[3], sizeof(float), &sphere->radius);
		index++;
	}

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	static std::vector<std::string> paths =
	{
		"Assets/Skybox/right.jpg",
		"Assets/Skybox/left.jpg",
		"Assets/Skybox/top.jpg",
		"Assets/Skybox/bottom.jpg",
		"Assets/Skybox/front.jpg",
		"Assets/Skybox/back.jpg"
	};

	skyboxTexture = ImageTexture::LoadTextureMap(paths);
}

void Renderer::CreateSpheres()
{

	mSpheres.push_back(new Sphere(glm::vec3(0), 1, glm::vec3(0, 0, 0), REFLECTIVE, 0));
	mSpheres.push_back(new Sphere(glm::vec3(cosf(2 * M_PI / 3 * 1) * 5, 1, sinf(2 * M_PI / 3 * 1) * 5), 1, glm::vec3(0, 0, 0), REFLECTIVE, 0));
	mSpheres.push_back(new Sphere(glm::vec3(cosf(2 * M_PI / 3 * 2) * 5, 1, sinf(2 * M_PI / 3 * 2) * 5), 1, glm::vec3(0, 0, 0), TEXTURED, 0));
	mSpheres.push_back(new Sphere(glm::vec3(cosf(2 * M_PI / 3 * 3) * 5, 1, sinf(2 * M_PI / 3 * 3) * 5), 1, glm::vec3(0, 0, 0), TEXTURED, 1));
	mSpheres.push_back(new Sphere(glm::vec3(0, -1500, 0), 1500, glm::vec3(0.75, 0.85, 0.9), DIFFUSE, 0));
	static bool restart = false;
	for (int i = 0; i < 145; i++)
	{
		do
		{
			float textureRandom = GetRandom();
			int textureId = textureRandom > 0.66 ? 0 : textureRandom > 0.33 ? 1 : 2;
			float typeRandom = GetRandom();
			bool textured = typeRandom > 0.85;
			bool diffuse = typeRandom > 0.05 && !textured;
			float randomRadius = 0.4 + GetRandom() * 0.3;
			glm::vec3 randomPosition = glm::vec3((GetRandom() - 0.5) * 15, randomRadius, (GetRandom() - 0.5) * 15);
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
	static int frameCount = 0;
	static float time = 0;
	
	float currentTime = glfwGetTime();
	float deltaTime = currentTime - lastTime;
	time += deltaTime;
	frameCount++;

	if (time >= 1)
	{
		std::cout << frameCount << std::endl;
		time = 0;
		frameCount = 0;
	}


	lastTime = currentTime;
}

void Renderer::PreRender()
{
	mSceneFrameBuffer->Clear();
	mRayTraceProgram->Bind();
	mRayTraceProgram->SetUniformTexture("uTextures[0]", 0, mTextures[0]);
	mRayTraceProgram->SetUniformTexture("uTextures[1]", 1, mTextures[1]);
	mRayTraceProgram->SetUniformTexture("uTextures[2]", 2, mTextures[2]);
	mRayTraceProgram->SetUniformTexture("u_SkyboxTexture", 3, skyboxTexture);
	mRayTraceProgram->SetUniform("scale", scale);
	mRayTraceProgram->UnBind();	
}

void Renderer::PostRender()
{
}

void Renderer::RenderScene()
{
	mSceneFrameBuffer->Bind();
	mRayTraceProgram->Bind();
	mRayTraceProgram->SetUniform("uSphereCount", (int)mSpheres.size());
	mRayTraceProgram->SetUniform("uViewProjMatrix", mCamera->GetViewProjMatrix());
	mRayTraceProgram->SetUniform("uCameraEye", mCamera->GetCameraEye());
	mRayTraceProgram->SetUniform("uPathDepth", 5);
	mRayTraceProgram->SetUniform("uLightDirection", glm::vec3(-0.4,-1,-0.2));
	mCanvas->Draw();
	mRayTraceProgram->UnBind();
	mSceneFrameBuffer->UnBind();
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
		originLength = glm::length(originVec);
		radiusLength = obj->radius + radius;
		if (originLength < radiusLength) return true;
	}
	return false;
}
