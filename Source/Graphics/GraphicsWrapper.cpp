#include <GLEW/glew.h>
#include "GraphicsWrapper.h"

using namespace Graphics;
GraphicsWrapper* GraphicsWrapper::m_instance = 0;

const int tileSize = 5;

float vertices[] = 
{
	tileSize,  0, 0,
	-tileSize, 0, 0,
	0, 0, -tileSize,

	tileSize,  0, 0,
	-tileSize, 0, 0,
	0, 0, tileSize,
};

float texCoords[] = 
{
	0, 0,
	1, 1,
	0, 1,

 	0, 0,
 	1, 1,
 	1, 0,
};

float normals[] = 
{
	0, 1, 0,
	0, 1, 0,
	0, 1, 0,

	0, 1, 0,
	0, 1, 0,
	0, 1, 0,
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
	for (size_t i = m_renderItems.size() -1 ; i > -1 ; i--)
	{
		for (size_t j = m_renderItems[i]->Instances.size()-1; j > -1; j--)
		{
			delete m_renderItems[i]->Instances[j];
		}

		delete m_renderItems[i];
		m_renderItems.pop_back();
	}

	delete m_shaderSTD;
	delete m_camera;

	SDL_GL_DeleteContext(m_context);
	SDL_Quit();

	m_instance=0;
}

//NOT USED ATM, FOR MODELS
void GraphicsWrapper::Render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shaderSTD->UseProgram();
	
	glm::mat4 vm = *m_camera->GetView();
	glm::mat4 pvm = *m_camera->GetProjection();

	m_shaderSTD->SetUniformV("gView", vm);
	m_shaderSTD->SetUniformV("gProj", pvm);

	m_shaderSTD->SetUniformV("innerTessLevel", 2048.0f);
	m_shaderSTD->SetUniformV("outerTessLevel", 2048.0f);

	GLint gSampler = glGetUniformLocation(m_shaderSTD->GetProgramHandle(), "gSampler");
	glUniform1i(gSampler, 0);

	for (int i = 0; i < m_renderItems.size();i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_renderItems[i]->TextureDiffuse);

		//Update matrix buffer//
		glBindBuffer(GL_ARRAY_BUFFER, m_renderItems[i]->MatrixBuffer);
		glm::mat4* matrices = (glm::mat4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		for (int j = 0; j < m_renderItems[i]->Instances.size(); j++)
		{
			matrices[j] = m_renderItems[i]->Instances[j]->ModelMatrix;
		}

		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(m_renderItems[i]->VAO);

		glDrawArraysInstanced(m_renderItems[i]->RenderParam,0, m_renderItems[i]->Vertices, (GLsizei)m_renderItems[i]->Instances.size());
	}

	SDL_GL_SwapWindow(m_window);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error rendering %d\n", error);
		system("pause");
	}
}

void GraphicsWrapper::RenderTerrain()
{
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	m_shaderSTD->UseProgram();

	for (int i = 0; i < m_terrainPatches.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_terrainPatches[i]->TextureDiffuse); 

		glm::mat4 vm = *m_camera->GetView() * m_terrainPatches[i]->ModelMatrix;
		glm::mat4 pvm = *m_camera->GetProjection() * vm;

		m_shaderSTD->SetUniformV("gVM", vm);
		m_shaderSTD->SetUniformV("gPVM", pvm);

		m_shaderSTD->SetUniformV("innerTessLevel", 256.0f);
		m_shaderSTD->SetUniformV("outerTessLevel", 256.0f);

		//GLint gSampler = glGetUniformLocation(m_shaderSTD->GetProgramHandle(), "gSampler");
		//glUniform1i(gSampler, 0);
		glBindVertexArray(m_terrainVAO);

		glDrawArrays(GL_PATCHES, 0, 6);
	}

	glBindVertexArray(0);

	SDL_GL_SwapWindow(m_window);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error rendering %d\n", error);
		//system("pause");
	}
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
	glDepthRange(0.0, 1.0);
	glEnable(GL_DEPTH_TEST);

	//CAMERA
	m_camera = new GLCamera(45.0f, m_width, m_height, 0.1f, 100.0f);
	m_camera->SetPosition(glm::vec3(0, 3, 0));
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

//NOT USED ATM
void Graphics::GraphicsWrapper::LoadModel()
{
	RenderItem* newItem = new RenderItem;

	GLuint vbo[4];
	glGenBuffers(4, vbo);

	//vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//texcoords
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

	//normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

	//model matrix
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &newItem->VAO);
	glBindVertexArray(newItem->VAO);

	GLint pos = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_position");
	GLint tex = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_tex");
	GLint norm = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_normal");
	GLint matrix = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_modelM");

	glEnableVertexAttribArray(pos);
	glEnableVertexAttribArray(tex);
	glEnableVertexAttribArray(norm);
	glEnableVertexAttribArray(matrix);      
	glEnableVertexAttribArray(matrix + 1);    
	glEnableVertexAttribArray(matrix + 2);    
	glEnableVertexAttribArray(matrix + 3);    

	//vertices
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//texcoords
	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, 0, 0);

	//normals
	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//model matrix
	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	glVertexAttribPointer(matrix, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 0));
	glVertexAttribDivisor(matrix, 1);
	glVertexAttribPointer(matrix + 1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 1));
	glVertexAttribDivisor(matrix + 1, 1);
	glVertexAttribPointer(matrix + 2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 2));
	glVertexAttribDivisor(matrix + 2, 1);
	glVertexAttribPointer(matrix + 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 3));
	glVertexAttribDivisor(matrix + 3, 1);

	//newItem->TextureDiffuse = LoadTextureRAW("../../../Content/height.raw");
	newItem->RenderParam = GL_PATCHES;
	newItem->MatrixBuffer = vbo[3];
	newItem->Vertices = sizeof(vertices) / sizeof(float);

	m_renderItems.push_back(newItem);
	RenderInstance* ins = new RenderInstance;
	ins->ModelMatrix = glm::mat4(1.0f);
	m_renderItems.back()->Instances.push_back(ins);

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
// 	m_shaderSTD->AddShader("../../../Content/Shaders/vsStandard.glsl", GL_VERTEX_SHADER);
// 	m_shaderSTD->AddShader("../../../Content/Shaders/fsStandard.glsl", GL_FRAGMENT_SHADER);
	m_shaderSTD->AddShader("../../../Content/Shaders/tess/tess_vs.glsl", GL_VERTEX_SHADER);
	m_shaderSTD->AddShader("../../../Content/Shaders/tess/tess_tri_tcs.glsl", GL_TESS_CONTROL_SHADER);
	m_shaderSTD->AddShader("../../../Content/Shaders/tess/tess_tri_tes.glsl", GL_TESS_EVALUATION_SHADER);
	m_shaderSTD->AddShader("../../../Content/Shaders/tess/tess_geo.glsl", GL_GEOMETRY_SHADER);
	m_shaderSTD->AddShader("../../../Content/Shaders/tess/tess_frag.glsl", GL_FRAGMENT_SHADER);
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


GLuint Graphics::GraphicsWrapper::LoadTexturePatch(const char * _filename, unsigned int _x, unsigned int _y, short _colorSlots, GLint _fileFormat)
{
	GLuint texture;
	GLubyte * data;
	FILE * file;
	// open texture data
 	fopen_s(&file,_filename, "rb");
 	if (file == NULL) 
	{
		printf("error reading file\n");
		return 0;
	}

	data = (GLubyte*)malloc(m_level.ChunkSize * m_level.ChunkSize * _colorSlots);
	if (_fileFormat == FORMAT_PAK)
	{
		long location = ((m_level.ChunkSize*m_level.ChunkSize*m_level.X*_y) + m_level.ChunkSize*m_level.ChunkSize*_x) * _colorSlots;

		fseek(file, location, SEEK_SET);
		fread(data, m_level.ChunkSize * m_level.ChunkSize * _colorSlots, 1, file);
	}

 	fclose(file);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, _colorSlots == 1 ? GL_RED : GL_RGB, m_level.ChunkSize, m_level.ChunkSize, 0, _colorSlots == 1 ? GL_RED : GL_RGB, GL_UNSIGNED_BYTE,data);

	free(data);

	return texture;

}

void Graphics::GraphicsWrapper::ConvertToPAK(const char * _filename, GLint _width, GLint _height, short _colorSlots)
{
	GLubyte * data;
	FILE * textureFile;
	FILE * pakFile;
	// open texture data
	fopen_s(&textureFile, _filename, "rb");
	fopen_s(&pakFile, "../../../Content/texture.pak", "ab");
	if (textureFile == NULL)
	{
		printf("error reading file\n");
		return;
	}
	if (pakFile == NULL)
	{
		printf("error reading file\n");
		return;
	}

	data = (GLubyte*)malloc(m_level.ChunkSize * _colorSlots);

	unsigned int x = _width / m_level.ChunkSize;
	unsigned int y = _height / m_level.ChunkSize;

	for (unsigned int k = 0; k < y;k++)
	{
		for (unsigned int i = 0; i < m_level.ChunkSize; i++)
		{
			for (unsigned int j = 0; j < m_level.ChunkSize; j++)
			{
				long offset = (i*m_level.ChunkSize + _width*j + m_level.ChunkSize*m_level.ChunkSize*k)*_colorSlots;

				fseek(textureFile, offset, SEEK_SET);
				fread(data, m_level.ChunkSize * _colorSlots, 1, textureFile);
				fwrite(data, 1, m_level.ChunkSize * _colorSlots, pakFile);
			}
		}
		printf("%d\n",k);
	}

	fclose(textureFile);
	fclose(pakFile);

	free(data);
}


void Graphics::GraphicsWrapper::LoadTerrainPatch()
{
	//Load patch structure once
	if(m_terrainPatches.empty())
	{
		glGenBuffers(3, m_terrainVBO);

		//vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		//texcoords
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

		//normals
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[2]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

		glGenVertexArrays(1, &m_terrainVAO);
		glBindVertexArray(m_terrainVAO);

		GLint pos = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_position");
		GLint tex = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_tex");
		GLint norm = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_normal");

		glEnableVertexAttribArray(pos);
		glEnableVertexAttribArray(tex);
		glEnableVertexAttribArray(norm);

		//vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[0]);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);

		//texcoords
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[1]);
		glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, 0, 0);

		//normals
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[2]);
		glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	//Add individual patch data like different heightmap
	for (int i = -4; i < 4; i++)
	{
		for (int j = -4; j < 4; j++)
		{
			TerrainPatch* newItem = new TerrainPatch;

			//ConvertToPAK("../../../Content/height.raw", m_level.Width, m_level.Height, 1);

			newItem->TextureDiffuse = LoadTexturePatch("../../../Content/texture.pak", j + m_level.X/2, i+m_level.Y/2, 1, FORMAT_PAK);
			newItem->ModelMatrix = glm::translate(glm::vec3(j*tileSize, 0, i*tileSize));

			m_terrainPatches.push_back(newItem);
		}
	}

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error loading model %d\n", error);
	}
}