#include <GLEW/glew.h>
#include "GraphicsWrapper.h"


using namespace Graphics;
GraphicsWrapper* GraphicsWrapper::m_instance = 0;

float vertices[] = {
	0.0f,  0.5f, 1.0f,
	0.5f, -0.5f, 1.0f,
	-0.5f, -0.5f, 1.0f
};

GraphicsWrapper& GraphicsWrapper::GetInstance()
{
	if (!m_instance)
		m_instance = new GraphicsWrapper();
	return *m_instance;
}

GraphicsWrapper::GraphicsWrapper()
{

}
GraphicsWrapper::~GraphicsWrapper()
{
	delete m_shaderSTD;
	delete m_camera;

	SDL_GL_DeleteContext(m_context);
	SDL_Quit();

	m_instance=0;
}

void GraphicsWrapper::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shaderSTD->UseProgram();

	CameraToShader();

	glDrawArrays(GL_TRIANGLES, 0, 3);
	SDL_GL_SwapWindow(m_window);
}

void Graphics::GraphicsWrapper::InitializeSDL(unsigned int _width, unsigned int _height)
{
	m_width = _width;
	m_height = _height;

	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	m_window = SDL_CreateWindow("OpenGL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_OPENGL);

	m_context = SDL_GL_CreateContext(m_window);

}

void Graphics::GraphicsWrapper::InitializeGLEW()
{
	//GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glViewport(0, 0, m_width, m_height);

	//CAMERA
	m_camera = new GLCamera(45.0f, m_width, m_height, 0.1f, 100.0f);
	m_camera->SetPosition(glm::vec3(0, 0, 10));
	m_camera->SetLookAt(glm::vec3(0, 0, 0));
	m_camera->SetViewPort(0, 0, m_width, m_height);

	//ERROR
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error initializing Glew %d\n", error);
	}

	//PRINT
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("GL Vendor : %s\n", vendor);
	printf("GL Renderer : %s\n", renderer);
	printf("GL Version (string) : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glslVersion);
}

void Graphics::GraphicsWrapper::LoadModel()
{
	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLint pos = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "m_position");

	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(pos);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error loading model %d\n", error);
	}
}

void Graphics::GraphicsWrapper::InitializeShaders()
{
	m_shaderSTD = new ShaderHandler();

	m_shaderSTD->CreateShaderProgram();
	m_shaderSTD->AddShader("../../../Content/Shaders/vsStandard.glsl", GL_VERTEX_SHADER);
	m_shaderSTD->AddShader("../../../Content/Shaders/fsStandard.glsl", GL_FRAGMENT_SHADER);
	m_shaderSTD->LinkShaderProgram();

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error initializing shaders %d\n", error);
	}
}

//CAMERA MOVEMENT
void Graphics::GraphicsWrapper::MoveCameraForward(float _val)	{m_camera->Move(_val);}
void Graphics::GraphicsWrapper::MoveCameraStrafe(float _val)	{m_camera->Strafe(_val);}
void Graphics::GraphicsWrapper::LookCameraX(float _val)			{m_camera->Yaw(_val);}
void Graphics::GraphicsWrapper::LookCameraY(float _val)			{m_camera->Pitch(_val);}

void Graphics::GraphicsWrapper::CameraToShader()
{
	glm::mat4* temp1 = m_camera->GetProjection();

	GLint projection = glGetUniformLocation(m_shaderSTD->GetProgramHandle(), "m_matProj");
	glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(*temp1));

	temp1 = m_camera->GetView();

	GLint view = glGetUniformLocation(m_shaderSTD->GetProgramHandle(), "m_matView");
	glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(*temp1));
}