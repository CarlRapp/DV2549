#include "TextRenderer.h"

TextRenderer& TextRenderer::GetInstance()
{
	static TextRenderer* m_instance = new TextRenderer();
	return *m_instance;
}

TextRenderer::TextRenderer()
{

}

TextRenderer::~TextRenderer()
{

}

void TextRenderer::LoadText(unsigned int _w, unsigned int _h)
{
	m_TextShader = new ShaderHandler();

	m_TextShader->CreateShaderProgram();
	m_TextShader->AddShader("../../../Content/Shaders/text_vertex.glsl", GL_VERTEX_SHADER);
	m_TextShader->AddShader("../../../Content/Shaders/text_fragment.glsl", GL_FRAGMENT_SHADER);
	m_TextShader->LinkShaderProgram();

	LoadLetters();

	float w = (8.f * (Text::LETTERS-1)) / (_w);
	float h = 8.0f / (_h);

	// Load texture file and convert to openGL format
	GLbyte imgData[64 * Text::LETTERS];
	int k = 0;
	int x = 0;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < m_letters64.size(); j++)
		{
			for (int g = 0; g < 8; g++)
			{
				imgData[k] = (*m_letters64[j])[i * 8 + g];//[j];
				k++;
				x++;
 			}
 		}
	}

	glActiveTexture(GL_TEXTURE3);
	glGenTextures(1, &m_TextInfo.TexHandle);
	glBindTexture(GL_TEXTURE_2D, m_TextInfo.TexHandle);

	// Copy file to OpenGL
	//glActiveTexture(GL_TEXTURE0); error = glGetError();
	glGenTextures(1, &m_TextInfo.TexHandle);
	glBindTexture(GL_TEXTURE_2D, m_TextInfo.TexHandle); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//gluBuild2DMipmaps(GL_TEXTURE_2D, channels, width, height, GL_RGBA, GL_UNSIGNED_BYTE, imgData);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,(GLsizei)(8 * m_letters64.size()), 8, 0, GL_RED, GL_BYTE, imgData); 

	m_TextInfo.Height = h;
	m_TextInfo.Width = w;
	m_TextInfo.X = 0;
	m_TextInfo.Y = 0;

	//buffra datan till 2D shader
	float positionData[] = {
		-1.0, -1.0,
		-1.0, 1.0,
		1.0, -1.0,
		-1.0, 1.0,
		1.0, 1.0,
		1.0, -1.0 };

	float texCoordData[] = {
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f };

	int nrOfPoints = 12;

	GLuint m_2DVBO[2];
	glGenBuffers(2, m_2DVBO);

	// "Bind" (switch focus to) first buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_2DVBO[0]);
	glBufferData(GL_ARRAY_BUFFER, nrOfPoints * sizeof(float), positionData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, m_2DVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, nrOfPoints * sizeof(float), texCoordData, GL_STATIC_DRAW);

	// create 1 VAO
	glGenVertexArrays(1, &m_2DVAO);
	glBindVertexArray(m_2DVAO);

	// enable "vertex attribute arrays"
	glEnableVertexAttribArray(0); // position
	glEnableVertexAttribArray(1); // texCoord

								  // map index 0 to position buffer
	glBindBuffer(GL_ARRAY_BUFFER, m_2DVBO[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

	glBindBuffer(GL_ARRAY_BUFFER, m_2DVBO[1]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLubyte *)NULL);

	

	glBindVertexArray(0); // disable VAO
	glUseProgram(0); // disable shader program
	glDeleteBuffers(2, m_2DVBO);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error loading text %d\n", error);
	}
}

void TextRenderer::LoadLetters()
{
	m_letters64.push_back(&Text::_space);
	m_letters64.push_back(&Text::_exclamation);
	m_letters64.push_back(&Text::_quote);
	m_letters64.push_back(&Text::_number);
	m_letters64.push_back(&Text::_dollar);
	m_letters64.push_back(&Text::_percent);
	m_letters64.push_back(&Text::_ampersand);
	m_letters64.push_back(&Text::_apostrophe);
	m_letters64.push_back(&Text::_leftbrace);
	m_letters64.push_back(&Text::_rightbrace);
	m_letters64.push_back(&Text::_asterisk);
	m_letters64.push_back(&Text::_plus);
	m_letters64.push_back(&Text::_comma);
	m_letters64.push_back(&Text::_minus);
	m_letters64.push_back(&Text::_dot);
	m_letters64.push_back(&Text::_slash);

	m_letters64.push_back(&Text::_0);
	m_letters64.push_back(&Text::_1);
	m_letters64.push_back(&Text::_2);
	m_letters64.push_back(&Text::_3);
	m_letters64.push_back(&Text::_4);
	m_letters64.push_back(&Text::_5);
	m_letters64.push_back(&Text::_6);
	m_letters64.push_back(&Text::_7);
	m_letters64.push_back(&Text::_8);
	m_letters64.push_back(&Text::_9);

	m_letters64.push_back(&Text::_colon);
	m_letters64.push_back(&Text::_semicolon);
	m_letters64.push_back(&Text::_lessthan);
	m_letters64.push_back(&Text::_equal);
	m_letters64.push_back(&Text::_morethan);
	m_letters64.push_back(&Text::_question);
	m_letters64.push_back(&Text::_at);

	m_letters64.push_back(&Text::A);
	m_letters64.push_back(&Text::B);
	m_letters64.push_back(&Text::C);
	m_letters64.push_back(&Text::D);
	m_letters64.push_back(&Text::E);
	m_letters64.push_back(&Text::F);
	m_letters64.push_back(&Text::G);
	m_letters64.push_back(&Text::H);
	m_letters64.push_back(&Text::I);
	m_letters64.push_back(&Text::J);
	m_letters64.push_back(&Text::K);
	m_letters64.push_back(&Text::L);
	m_letters64.push_back(&Text::M);
	m_letters64.push_back(&Text::N);
	m_letters64.push_back(&Text::O);
	m_letters64.push_back(&Text::P);
	m_letters64.push_back(&Text::Q);
	m_letters64.push_back(&Text::R);
	m_letters64.push_back(&Text::S);
	m_letters64.push_back(&Text::T);
	m_letters64.push_back(&Text::U);
	m_letters64.push_back(&Text::V);
	m_letters64.push_back(&Text::W);
	m_letters64.push_back(&Text::X);
	m_letters64.push_back(&Text::Y);
	m_letters64.push_back(&Text::Z);

	//    m_letters256.resize(m_letters64.size());
	//    
	//    for(int i=0; i < m_letters64.size();i++)
	//    {
	////        GLbyte* f;
	////        f = (GLbyte*)malloc(sizeof(GLbyte[256]));
	//
	//        m_letters256[i] = (GLbyte(*)[256])malloc(256);
	//    }


// 	GLbyte temp;
// 
// 	GLbyte shade = 'm';
// 
// 	for (int i = 0; i < m_letters64.size(); i++)
// 	{
// 		for (int j = 0; j < 64; j++)
// 		{
// 			temp = (*m_letters64[i])[j];
// 			if (temp == '~')
// 			{
// 				//                if(j-8 > 0)
// 				//                {
// 				//                    if((*m_letters[i])[j-8] != '~')
// 				//                    (*m_letters[i])[j-8] += 40;  
// 				//                }
// 
// 				if (j + 1 < 64)
// 				{
// 					if ((*m_letters64[i])[j + 1] == '~')
// 					{
// 						if (j + 8 < 64)
// 							if ((*m_letters64[i])[j - 8] == '~' && !((*m_letters64[i])[j - 1] == '~'))
// 								(*m_letters64[i])[j] -= 40;
// 					}
// 				}
// 			}
// 
// 			if (temp == 0)
// 			{
// 				int a = 0;
// 
// 				if (j - 8 > 0)
// 				{
// 					if ((*m_letters64[i])[j - 8] == '~')
// 						a++;
// 				}
// 
// 				if (j + 8 < 64)
// 				{
// 					if ((*m_letters64[i])[j + 8] == '~')
// 						a++;
// 				}
// 
// 				if (j - 1 > 0)
// 				{
// 					if ((*m_letters64[i])[j - 1] == '~')
// 						a++;
// 				}
// 
// 				if (j + 1 < 64)
// 				{
// 					if ((*m_letters64[i])[j + 1] == '~')
// 						a++;
// 				}
// 
// 				if (a > 0)
// 					(*m_letters64[i])[j] = ' '*a;
// 			}
// 		}
// 	}

}

void TextRenderer::RenderText(unsigned int _w, unsigned int _h)
{
	m_TextShader->UseProgram();
	glDisable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE3);
	GLuint tex = glGetUniformLocation(m_TextShader->GetProgramHandle(), "m_texture");
	glUniform1i(tex, 3);

	glBindVertexArray(m_2DVAO);
	glBindTexture(GL_TEXTURE_2D, m_TextInfo.TexHandle);

	for (int s = 0; s < m_RenderStrings.size();s++)
	{
		if (m_RenderStrings[s].Text == NULL)
			return;

		float width;
		float height;

		width = (8.0f / _w)*m_RenderStrings[s].Scale;
		height = (8.0f / _h)*m_RenderStrings[s].Scale;

		float r_ = m_RenderStrings[s].Color.x;
		float g_ = m_RenderStrings[s].Color.y;
		float b_ = m_RenderStrings[s].Color.z;

		GLint colorLocation = glGetUniformLocation(m_TextShader->GetProgramHandle(), "m_color");
		glUniform3f(colorLocation, r_, g_, b_);

		float x = m_RenderStrings[s].X;
		float y = m_RenderStrings[s].Y + _h - (height * _h);

		for (int i = 0; i < m_RenderStrings[s].Text->size(); i++)
		{
			if (m_RenderStrings[s].Text->at(i) == '\n')
			{
				y -= (height * _h);
				i++;
				x = 0;
			}

			x += (width);

			glViewport((GLint)(x*_w), (GLint)(y), (GLsizei)(width*_w), (GLsizei)(height*_h));
			GLint letterLocation = glGetUniformLocation(m_TextShader->GetProgramHandle(), "m_letter");

			glUniform1i(letterLocation, m_RenderStrings[s].Text->at(i) - 32); //Not entire alphabet available -32
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	glBindVertexArray(0);
	
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		printf("Error rendering text %d\n", error);
	}
}

//TODO Pool allocator?
unsigned int TextRenderer::AddString(std::string* _text, glm::vec3 _color, float _scale, float _x, float _y)
{
	m_RenderStrings.push_back(RenderString(_text,_color,_scale,_x,_y, m_currentStringID));

	//To upper
	for (int i = 0; (*m_RenderStrings.back().Text)[i] != 0; i++)
	{
		if ((*m_RenderStrings.back().Text)[i] <= 122 && (*m_RenderStrings.back().Text)[i] >= 97)
			(*m_RenderStrings.back().Text)[i] -= 32;
	}

	return m_currentStringID++;
}

void TextRenderer::RemoveString(unsigned int _ID)
{
	for (int i = 0; i < m_RenderStrings.size(); i++)
	{
		if (m_RenderStrings[i].ID = _ID)
		{
			m_RenderStrings.erase(m_RenderStrings.begin() + i);
			break;
		}
	}
}

