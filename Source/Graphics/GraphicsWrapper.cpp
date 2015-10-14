#include <GLEW/glew.h>
#include "GraphicsWrapper.h"
#include <Memory/StackAllocator/StackAllocator_SingleBuffer.h>
#include <Memory/MemoryWrapper.h>
#include "TextRenderer.h"

using namespace Graphics;
static GraphicsWrapper* m_instance = nullptr;

GraphicsWrapper& GraphicsWrapper::GetInstance()
{
	if(m_instance == nullptr)
		m_instance = new GraphicsWrapper();

	return *m_instance;
}

GraphicsWrapper::GraphicsWrapper()
{
	m_level.X = m_level.Width / m_level.ChunkSize;
	m_level.Y = m_level.Height / m_level.ChunkSize;
		
	//compressionHandler = new Compression::CompressionHandler_zlib(); // Temp. instantiation until integration with ResourceManager.

	unsigned int width = m_level.PatchSize / m_level.TileSize;

	m_level.Vertices = (3 * 3 * 2 * width * width);
	m_level.TexCoords = (2 * 3 * 2 * width * width);
	m_level.Normals = (3 * 3 * 2 * width * width);

	m_level.TerrainVertices = (float*)malloc(m_level.Vertices*sizeof(float));
	m_level.TerrainTex = (float*)malloc(m_level.TexCoords*sizeof(float));
	m_level.TerrainNormals = (float*)malloc(m_level.Normals*sizeof(float));
	debugString = "DEBUG";
	TextRenderer::GetInstance().AddString(&debugString, glm::vec3(1, 1, 1), 2, 0, -400);
}

GraphicsWrapper::~GraphicsWrapper()
{
// 	for (size_t i = m_renderItems.size() -1 ; i > -1 ; i--)
// 	{
// 		for (size_t j = m_renderItems[i]->Instances.size()-1; j > -1; j--)
// 		{
// 			delete m_renderItems[i]->Instances[j];
// 		}
// 
// 		delete m_renderItems[i];
// 		m_renderItems.pop_back();
// 	}

	delete m_terrainShader;
	delete m_camera;

	for (int i = 0; i < m_terrainPatches.size(); i++)
	{
		glDeleteTextures(1, &m_terrainPatches[i]->TextureDiffuse);
		glDeleteTextures(1, &m_terrainPatches[i]->TextureNormal);
		glDeleteTextures(1, &m_terrainPatches[i]->TextureHeight);
	}

	SDL_GL_DeleteContext(m_context);
	SDL_Quit();
}


void GraphicsWrapper::RenderTerrain()
{
	SDL_LockMutex(gMutex);
	wglMakeCurrent(m_hDC, m_renderContext);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glViewport(0, 0, m_width, m_height);
 
	m_terrainShader->UseProgram();

	m_terrainShader->SetUniformV("gEyePos", m_camera->GetPosition());

	GLint availableMem;

	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &availableMem);

	int active = 0;

	std::vector<int> faultySlots;

	for (int i = 0; i < m_terrainPatches.size(); i++)
	{
		if (m_terrainPatches[i]->IsActive)
		{
			GLuint tex = glGetUniformLocation(m_terrainShader->GetProgramHandle(), "gTexHeight");
			glUniform1i(tex, 0);

			tex = glGetUniformLocation(m_terrainShader->GetProgramHandle(), "gTexNormal");
			glUniform1i(tex, 1);

			tex = glGetUniformLocation(m_terrainShader->GetProgramHandle(), "gTexDiffuse");
			glUniform1i(tex, 2);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_terrainPatches[i]->TextureHeight);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, m_terrainPatches[i]->TextureNormal);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, m_terrainPatches[i]->TextureDiffuse);

			glm::mat4 vm = *m_camera->GetView() * m_terrainPatches[i]->ModelMatrix;
			glm::mat4 pvm = *m_camera->GetProjection() * vm;

			m_terrainShader->SetUniformV("gM", m_terrainPatches[i]->ModelMatrix);
			m_terrainShader->SetUniformV("gPV", *m_camera->GetProjection() * *m_camera->GetView());
			m_terrainShader->SetUniformV("gPVM", pvm);

			m_terrainShader->SetUniformV("gFog", m_fog);

			glBindVertexArray(m_terrainVAO);

			glDrawArrays(GL_PATCHES, 0, m_level.Vertices);

			active++;
		}
		else
			faultySlots.push_back(i);
	}

	if (active == m_terrainPatches.size())
	{
		debugString = "ALL " + std::to_string(active) + " PATCHES RENDERED";
	}
	else
	{
		debugString = "FAULTY PATCH ID: ";
		for (int i = 0; i < faultySlots.size(); i++)
			debugString += std::to_string(faultySlots[i]) + ",";
	}
	debugString += "\nVIDEO MEM AVAILABLE: " + std::to_string(availableMem/(1024)) + "MB";

	
	
	glBindVertexArray(0);
	glUseProgram(0);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error rendering terrain %d\n", error);
		//system("pause");
	}

	TextRenderer::GetInstance().RenderText(m_width,m_height);

	SDL_GL_SwapWindow(m_window);
	wglMakeCurrent(NULL, NULL);
	SDL_UnlockMutex(gMutex);

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

	m_renderContext = wglGetCurrentContext();

	SDL_GL_SetSwapInterval(1);

	m_SDLStarted = true;
}

HDC Graphics::GraphicsWrapper::GetHDC()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	if (SDL_GetWindowWMInfo(m_window,&wmInfo) < 0)	//returns (-1) on error
	{
		std::cout << "error in SDL_GetWMInfo";
	}
	HWND hWnd = wmInfo.info.win.window;

	m_hDC = GetDC(hWnd); 

	return m_hDC;
}

void Graphics::GraphicsWrapper::InitializeGLEW()
{
	wglMakeCurrent(m_hDC, m_renderContext);

	//GLEW
	glewExperimental = GL_TRUE;
	glewInit();
	glClearColor(0.0f, 0.0f, 0.1f, 0.0f);
	glViewport(0, 0, m_width, m_height);
	glDepthRange(0.0, 900.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);

	//CAMERA
	m_camera = new GLCamera(45.0f, m_width, m_height, 0.1f, 900.0f);
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
	printf("\nGL Vendor : %s\n", vendor);
	printf("GL Renderer : %s\n", renderer);
	printf("GL Version (string) : %s\n", version);
	printf("GL Version (integer) : %d.%d\n", major, minor);
	printf("GLSL Version : %s\n", glslVersion);

	GLint textureUnits;

	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &textureUnits);
	printf("Max bound textures : %d\n\n", textureUnits);
}



void Graphics::GraphicsWrapper::InitializeShaders()
{
	m_terrainShader = new ShaderHandler();

	m_terrainShader->CreateShaderProgram();
	m_terrainShader->AddShader("../../../Content/Shaders/tess/tess_vs.glsl", GL_VERTEX_SHADER);
	m_terrainShader->AddShader("../../../Content/Shaders/tess/tess_tri_tcs.glsl", GL_TESS_CONTROL_SHADER);
	m_terrainShader->AddShader("../../../Content/Shaders/tess/tess_tri_tes.glsl", GL_TESS_EVALUATION_SHADER);
	m_terrainShader->AddShader("../../../Content/Shaders/tess/tess_geo.glsl", GL_GEOMETRY_SHADER);
	m_terrainShader->AddShader("../../../Content/Shaders/tess/tess_frag.glsl", GL_FRAGMENT_SHADER);
	m_terrainShader->LinkShaderProgram();

	TextRenderer::GetInstance().LoadText(m_width,m_height);

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

void Graphics::GraphicsWrapper::ReloadTerrainPatches(std::vector<TerrainPatch*> newPatches)
{
	SDL_LockMutex(gMutex);
	m_terrainPatches.clear();

	for (int n = 0; n < newPatches.size(); ++n)
	{
		m_terrainPatches.push_back(newPatches[n]);
	}	
	SDL_UnlockMutex(gMutex);
}

void Graphics::GraphicsWrapper::LoadSingleTexturePatch(int _tileX, int _tileY, TerrainPatch* _memLocation, HDC* _hdc, HGLRC* _hglrc)
{
	unsigned int X = _tileX + m_level.X / 2;
	unsigned int Y = _tileY + m_level.Y / 2;

	Memory::StackAllocator_SingleBuffer* tempStack = (Memory::StackAllocator_SingleBuffer*)Memory::MemoryWrapper::GetInstance()->GetGlobalStack();
	size_t memoryTop = tempStack->GetTop();

	TextureRAM texHeight = PushTextureToRAM("../../../Content/height.pak", Y, X, 1);
	TextureRAM texNormal = PushTextureToRAM("../../../Content/norm.pak", Y, X, 3);
	TextureRAM texDiffuse = PushTextureToRAM("../../../Content/diffuse.pak", Y, X, 3);

	SDL_LockMutex(gMutex);
	wglMakeCurrent(*_hdc, *_hglrc);
	_memLocation->TextureHeight = PushTextureToGL(texHeight.ColorSlots, texHeight.Data);
 	_memLocation->TextureNormal = PushTextureToGL(texNormal.ColorSlots, texNormal.Data);
 	_memLocation->TextureDiffuse = PushTextureToGL(texDiffuse.ColorSlots, texDiffuse.Data);

	_memLocation->ModelMatrix = glm::translate(glm::vec3(_tileX*m_level.PatchSize, 0, _tileY*m_level.PatchSize));
	_memLocation->IsActive = true;

	glFlush();
	wglMakeCurrent(NULL, NULL);
	SDL_UnlockMutex(gMutex);

	tempStack->FreeTo(memoryTop);
}

void Graphics::GraphicsWrapper::DeleteSingleTexturePatch(TerrainPatch* memLocation)
{
	SDL_LockMutex(gMutex);
	glDeleteTextures(1, &memLocation->TextureDiffuse);
	glDeleteTextures(1, &memLocation->TextureNormal);
	glDeleteTextures(1, &memLocation->TextureHeight);
	memLocation->IsActive = false;
	SDL_UnlockMutex(gMutex);
}

Graphics::TextureRAM Graphics::GraphicsWrapper::PushTextureToRAM(const char * _filename, unsigned int _x, unsigned int _y, short _colorSlots)
{
	TextureRAM texRAM;
	texRAM.ColorSlots = _colorSlots;

	FILE * file;
 	fopen_s(&file,_filename, "rb");
 	if (file == NULL) 
	{
		printf("missing file %s\n", _filename);
		texRAM.Data = nullptr;
		return texRAM;
	}

	texRAM.Data = (GLubyte*)(Memory::StackAllocator_SingleBuffer*)Memory::MemoryWrapper::GetInstance()->GetGlobalStack()->Reserve(m_level.ChunkSize * m_level.ChunkSize * _colorSlots);

	long location = (m_level.ChunkSize*m_level.ChunkSize)*(m_level.X*_x + _y) * _colorSlots;

	fseek(file, location, SEEK_SET);
	fread(texRAM.Data, m_level.ChunkSize * m_level.ChunkSize * _colorSlots, 1, file);
 	fclose(file);

	return texRAM;
}


GLuint Graphics::GraphicsWrapper::PushTextureToGL(short _colorSlots, GLubyte * data)
{
	GLuint texture;

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, _colorSlots == 1 ? GL_RED : GL_RGB, m_level.ChunkSize, m_level.ChunkSize, 0, _colorSlots == 1 ? GL_RED : GL_RGB, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error loading texture patch %d\n", error);
	}

	return texture;
}

void Graphics::GraphicsWrapper::ConvertToPAK(const char * _filename, GLint _width, GLint _height, short _colorSlots)
{
	Memory::StackAllocator_SingleBuffer* tempStack = (Memory::StackAllocator_SingleBuffer*)Memory::MemoryWrapper::GetInstance()->GetGlobalStack();

	//PackageReaderWriter *pakRW = new PackageReaderWriter(compressionHandler); // compressionHandler);

	// Test #1: Create a package:

	//std::vector<std::string> filePaths;
	//filePaths.push_back("../../../Content/diffuse.raw");
	//filePaths.push_back("../../../Content/norm.raw");
	//filePaths.push_back("../../../Content/height.raw");
	//pakRW->createPackageFromFiles("../../../Content/texturePak_lz4.pak", filePaths);

	// Test #2: Load files within package:

	//char *loadedDiffuseData = new char[150000000]; //[145686528];
	//pakRW->loadPackageData("../../../Content/texturePak_lz4.pak", loadedDiffuseData, 0, 0);

	//char *loadedNormData = new char[150000000]; //[145686528];
	//pakRW->loadPackageData("../../../Content/texturePak_lz4.pak", loadedNormData, 1, 1);

	//char *loadedHeightData = new char[50000000]; //[145686528];
	//pakRW->loadPackageData("../../../Content/texturePak_lz4.pak", loadedHeightData, 2, 2);

	//delete pakRW;
	
	// *************************************

	GLubyte * data;
	FILE * textureFile;
	FILE * pakFile;
	// open texture data
	fopen_s(&textureFile, _filename, "rb");

	std::string outPut;
	outPut = _filename;

	outPut.erase(outPut.end()-3, outPut.end());

	outPut += "pak";

	fopen_s(&pakFile, outPut.c_str(), "r");

	if (pakFile != NULL)
	{
		printf("%s OK\n", outPut.c_str());
		fclose(pakFile);
		return;
	}

	printf("%s NOT FOUND, try conversion from .raw to .pak\n", outPut.c_str());

	fopen_s(&pakFile, outPut.c_str(), "ab");
	if (textureFile == NULL)
	{
		printf("%s is missing, maybe unpack .rar?\n", _filename);
		return;
	}
	if (pakFile == NULL)
	{
		printf("unknown error converting file\n");
		return;
	}
	
	//data = (GLubyte*)malloc(m_level.ChunkSize * _colorSlots);
	size_t tempTop = tempStack->GetTop();
	data = (GLubyte*)tempStack->Reserve(m_level.ChunkSize * _colorSlots);//(GLubyte*)malloc(m_level.ChunkSize * m_level.ChunkSize * _colorSlots);
	unsigned int x = _width / m_level.ChunkSize;
	unsigned int y = _height / m_level.ChunkSize;

	data = (GLubyte*)malloc(m_level.ChunkSize * _colorSlots); // *x * y);

	printf("Converting...\n");
	//Attempts to read 1xChunksize at a time, and write to pak file
	
	unsigned long bytesHandled = 0;
	unsigned long elementsRead = 0;
	for (unsigned int k = 0; k < y;k++)
	{
		for (unsigned int i = 0; i < x; i++)
		{
			for (unsigned int j = 0; j < m_level.ChunkSize; j++)
			{
				long x_ = i*m_level.ChunkSize;
				long y_ = j*m_level.Width + k*m_level.ChunkSize*m_level.Width;

				long offset = (x_ + y_)*_colorSlots;

				fseek(textureFile, offset, SEEK_SET);
				fread(data, m_level.ChunkSize * _colorSlots, 1, textureFile);
				bytesHandled += elementsRead * m_level.ChunkSize * _colorSlots;
				//compressionHandler->compress_memoryToFile(data,  m_level.ChunkSize * _colorSlots, pakFile);
				fwrite(data, 1, m_level.ChunkSize * _colorSlots, pakFile);	//		bytesToCompress += m_level.ChunkSize * _colorSlots; //	
			}
		}
		printf("%d / %d\r",k,y);
	}

	//compressionHandler->compress_memoryToFile(data, bytesHandled, pakFile);

	fclose(textureFile);
	fclose(pakFile);

	//free(data);
	tempStack->FreeTo(tempTop);
}

unsigned int Graphics::GraphicsWrapper::AddString(std::string* _text, glm::vec3 _color, float _scale, float _x, float _y)
{
	return TextRenderer::GetInstance().AddString(_text, _color, _scale,  _x,  _y);
}

void Graphics::GraphicsWrapper::LoadTerrainPatch()
{
	//Load patch structure once
	if(m_terrainPatches.empty())
	{
		int i = 0;
		int t = 0;
		unsigned int width = m_level.PatchSize / m_level.TileSize;

		for (unsigned int row = 0; row < width; row++)
		{
			for (unsigned int col = 0; col < width; col++)
			{
				float offsetX = (float)((row)*m_level.TileSize);
				float offsetZ = (float)((col)*m_level.TileSize);

				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = 0 + offsetX; 
				m_level.TerrainNormals[i] = 1;
				m_level.TerrainVertices[i++] = 0; 
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = 0 + offsetZ; 

				m_level.TerrainTex[t++] = float(offsetX) / (m_level.PatchSize);
				m_level.TerrainTex[t++] = float(offsetZ) / (m_level.PatchSize);
				
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = 0 + offsetX; 
				m_level.TerrainNormals[i] = 1;
				m_level.TerrainVertices[i++] = 0; 
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = m_level.TileSize + offsetZ;

				m_level.TerrainTex[t++] = float(offsetX) / (m_level.PatchSize );
				m_level.TerrainTex[t++] = float(m_level.TileSize+offsetZ) / (m_level.PatchSize );
				
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = m_level.TileSize + offsetX;
				m_level.TerrainNormals[i] = 1;
				m_level.TerrainVertices[i++] = 0; 
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = 0 + offsetZ;
				
				m_level.TerrainTex[t++] = float(m_level.TileSize + offsetX) / (m_level.PatchSize );
				m_level.TerrainTex[t++] = float(offsetZ) / (m_level.PatchSize);
				
				

				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = 0 + offsetX;
				m_level.TerrainNormals[i] = 1;
				m_level.TerrainVertices[i++] = 0;
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = m_level.TileSize + offsetZ;

				m_level.TerrainTex[t++] = float( offsetX) / (m_level.PatchSize );
				m_level.TerrainTex[t++] = float(m_level.TileSize + offsetZ) / (m_level.PatchSize );
				
				
				
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = m_level.TileSize + offsetX;
				m_level.TerrainNormals[i] = 1;
				m_level.TerrainVertices[i++] = 0;
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = 0 + offsetZ;

				m_level.TerrainTex[t++] = float(m_level.TileSize + offsetX) / (m_level.PatchSize );
				m_level.TerrainTex[t++] = float(offsetZ) / (m_level.PatchSize );
				
				
				
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = m_level.TileSize + offsetX;
				m_level.TerrainNormals[i] = 1;
				m_level.TerrainVertices[i++] = 0;
				m_level.TerrainNormals[i] = 0;
				m_level.TerrainVertices[i++] = m_level.TileSize + offsetZ;

				m_level.TerrainTex[t++] = float(m_level.TileSize + offsetX) / (m_level.PatchSize );
				m_level.TerrainTex[t++] = float(m_level.TileSize + offsetZ) / (m_level.PatchSize );

			}
		}

		glGenBuffers(3, m_terrainVBO);

		//vertices
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[0]);
		glBufferData(GL_ARRAY_BUFFER, m_level.Vertices*sizeof(float),m_level.TerrainVertices, GL_STATIC_DRAW);

		//texcoords
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[1]);
		glBufferData(GL_ARRAY_BUFFER, m_level.TexCoords*sizeof(float), m_level.TerrainTex, GL_STATIC_DRAW);

		//normals
		glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO[2]);
		glBufferData(GL_ARRAY_BUFFER, m_level.Normals*sizeof(float), m_level.TerrainNormals, GL_STATIC_DRAW);

		glGenVertexArrays(1, &m_terrainVAO);
		glBindVertexArray(m_terrainVAO);

		GLint pos = glGetAttribLocation(m_terrainShader->GetProgramHandle(), "in_position");
		GLint tex = glGetAttribLocation(m_terrainShader->GetProgramHandle(), "in_tex");
		GLint norm = glGetAttribLocation(m_terrainShader->GetProgramHandle(), "in_normal");

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

	ConvertToPAK("../../../Content/diffuse.raw", m_level.Width, m_level.Height, 3);
	ConvertToPAK("../../../Content/norm.raw", m_level.Width, m_level.Height, 3);
	ConvertToPAK("../../../Content/height.raw", m_level.Width, m_level.Height, 1);
	
	int x = m_level.X / 2;
	int y = m_level.Y / 2;
	/*
	// 	//Add individual patch data, like different heightmap
	for (int i = -x; i < x; i++)
	{
		for (int j = -y; j < y; j++)
		{
			TerrainPatch* newItem = new TerrainPatch;

			//newItem->TextureDiffuse = LoadTextureRAW("../../../Content/test.raw", 512, 512, 3);
			newItem->TextureHeight = LoadTexturePatch("../../../Content/height.pak", j + m_level.Y / 2, i + m_level.X / 2, 1);
			newItem->TextureNormal = LoadTexturePatch("../../../Content/norm.pak", j + m_level.Y / 2, i + m_level.X / 2, 3);
			newItem->TextureDiffuse = LoadTexturePatch("../../../Content/diffuse.pak", j + m_level.Y / 2, i + m_level.X / 2, 3);
			newItem->ModelMatrix = glm::translate(glm::vec3(i*m_level.PatchSize, 0, j*m_level.PatchSize));

			m_terrainPatches.push_back(newItem);
		}
	}
	

	Memory::MemoryWrapper mem = Memory::MemoryWrapper::GetInstance();
	mem->GetPoolManager()->CreatePool(m_level.X*m_level.Y,sizeof(TerrainPatch));

	m_mapStatus.resize(m_level.Y);
	for (int i = 0; i < m_mapStatus.size(); i++)
	{
		m_mapStatus[i].resize(m_level.X);
		for (int j = 0; j < m_mapStatus[i].size(); j++)
		{
			m_mapStatus[i][j] = 0;
		}
	}
	*/
	wglMakeCurrent(m_hDC, m_renderContext);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error loading model %d\n", error);
	}
}

// GLuint LoadTextureRAW(const char * _filename, unsigned int _width, unsigned int _height, short _colorSlots)
// {
// 	GLuint texture;
// 	GLubyte * data;
// 	FILE * file;
// 	// open texture data
// 	fopen_s(&file, _filename, "rb");
// 	if (file == NULL)
// 	{
// 		printf("error reading file\n");
// 		return 0;
// 	}
// 
// 	data = (GLubyte*)malloc(_width * _height * _colorSlots);
// 
// 	fread(data, _width * _height * _colorSlots, 1, file);
// 
// 	fclose(file);
// 
// 	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
// 
// 	glGenTextures(1, &texture);
// 	glBindTexture(GL_TEXTURE_2D, texture);
// 
// 
// 	glTexImage2D(GL_TEXTURE_2D, 0, _colorSlots == 1 ? GL_RED : GL_RGB, _width, _height, 0, _colorSlots == 1 ? GL_RED : GL_RGB, GL_UNSIGNED_BYTE, data);
// 
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
// 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
// 
// 	free(data);
// 
// 	return texture;
// 
// }

//NOT USED ATM, FOR MODELS
void GraphicsWrapper::Render()
{
	// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// 	m_terrainShader->UseProgram();
	// 	
	// 	glm::mat4 vm = *m_camera->GetView();
	// 	glm::mat4 pvm = *m_camera->GetProjection();
	// 
	// 	m_terrainShader->SetUniformV("gView", vm);
	// 	m_terrainShader->SetUniformV("gProj", pvm);
	// 
	// 	GLint gSampler = glGetUniformLocation(m_terrainShader->GetProgramHandle(), "gSampler");
	// 	glUniform1i(gSampler, 0);
	// 
	// 	for (int i = 0; i < m_renderItems.size();i++)
	// 	{
	// 		glActiveTexture(GL_TEXTURE0);
	// 		glBindTexture(GL_TEXTURE_2D, m_renderItems[i]->TextureDiffuse);
	// 
	// 		//Update matrix buffer//
	// 		glBindBuffer(GL_ARRAY_BUFFER, m_renderItems[i]->MatrixBuffer);
	// 		glm::mat4* matrices = (glm::mat4*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	// 		for (int j = 0; j < m_renderItems[i]->Instances.size(); j++)
	// 		{
	// 			matrices[j] = m_renderItems[i]->Instances[j]->ModelMatrix;
	// 		}
	// 
	// 		glUnmapBuffer(GL_ARRAY_BUFFER);
	// 		glBindBuffer(GL_ARRAY_BUFFER, 0);
	// 
	// 		glBindVertexArray(m_renderItems[i]->VAO);
	// 
	// 		glDrawArraysInstanced(m_renderItems[i]->RenderParam,0, m_renderItems[i]->Vertices, (GLsizei)m_renderItems[i]->Instances.size());
	// 	}
	// 
	// 	SDL_GL_SwapWindow(m_window);
	// 
	// 	GLenum error = glGetError();
	// 	if (error != GL_NO_ERROR)
	// 	{
	// 		printf("Error rendering %d\n", error);
	// 		system("pause");
	// 	}
}
//NOT USED ATM
void Graphics::GraphicsWrapper::LoadModel()
{
	// 	RenderItem* newItem = new RenderItem;
	// 
	// 	GLuint vbo[4];
	// 	glGenBuffers(4, vbo);
	// 
	// 	//vertices
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	// 
	// 	//texcoords
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);
	// 
	// 	//normals
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	// 	glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);
	// 
	// 	//model matrix
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	// 	glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), NULL, GL_DYNAMIC_DRAW);
	// 
	// 	glGenVertexArrays(1, &newItem->VAO);
	// 	glBindVertexArray(newItem->VAO);
	// 
	// 	GLint pos = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_position");
	// 	GLint tex = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_tex");
	// 	GLint norm = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_normal");
	// 	GLint matrix = glGetAttribLocation(m_shaderSTD->GetProgramHandle(), "in_modelM");
	// 
	// 	glEnableVertexAttribArray(pos);
	// 	glEnableVertexAttribArray(tex);
	// 	glEnableVertexAttribArray(norm);
	// 	glEnableVertexAttribArray(matrix);      
	// 	glEnableVertexAttribArray(matrix + 1);    
	// 	glEnableVertexAttribArray(matrix + 2);    
	// 	glEnableVertexAttribArray(matrix + 3);    
	// 
	// 	//vertices
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	// 	glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// 
	// 	//texcoords
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	// 	glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, 0, 0);
	// 
	// 	//normals
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
	// 	glVertexAttribPointer(norm, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// 
	// 	//model matrix
	// 	glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
	// 	glVertexAttribPointer(matrix, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 0));
	// 	glVertexAttribDivisor(matrix, 1);
	// 	glVertexAttribPointer(matrix + 1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 1));
	// 	glVertexAttribDivisor(matrix + 1, 1);
	// 	glVertexAttribPointer(matrix + 2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 2));
	// 	glVertexAttribDivisor(matrix + 2, 1);
	// 	glVertexAttribPointer(matrix + 3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * 3));
	// 	glVertexAttribDivisor(matrix + 3, 1);
	// 
	// 	//newItem->TextureDiffuse = LoadTextureRAW("../../../Content/height.raw");
	// 	newItem->RenderParam = GL_PATCHES;
	// 	newItem->MatrixBuffer = vbo[3];
	// 	newItem->Vertices = sizeof(vertices) / sizeof(float);
	// 
	// 	m_renderItems.push_back(newItem);
	// 	RenderInstance* ins = new RenderInstance;
	// 	ins->ModelMatrix = glm::mat4(1.0f);
	// 	m_renderItems.back()->Instances.push_back(ins);
	// 
	// 	GLenum error = glGetError();
	// 	if (error != GL_NO_ERROR)
	// 	{
	// 		printf("Error loading model %d\n", error);
	// 	}
}
