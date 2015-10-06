#ifndef TextRenderer_H_
#define TextRenderer_H_
#include <GLEW/glew.h>
#include <map>
#include "GLText.h"
#include <vector>
#include "GLShaderHandler.h"

struct TextureInfo
{
public:
	GLuint TexHandle;
	float X, Y, Width, Height;

	TextureInfo() 
	{
		TexHandle = 0;
		X = 0;
		Y = 0;
		Width = 0;
		Height = 0;
	};

	TextureInfo(GLuint texHandle, float _x, float _y, float _width, float _height)
	{
		TexHandle = texHandle;
		X = _x;
		Y = _y;
		Width = _width;
		Height = _height;
	}

};

struct RenderString
{
	glm::vec3 Color;

	float Scale;
	float X;
	float Y;

	unsigned int ID;

	std::string* Text;

	RenderString(std::string* _text, glm::vec3 _color, float _scale, float _x, float _y, unsigned int _ID)
	{
		Text = _text;
		Color = _color;
		Scale = _scale;
		X = _x;
		Y = _y;
		ID = _ID;
	}
	
};

class TextRenderer
{

private:
	std::map<std::string, GLuint> m_TextureMap;
	std::vector<GLbyte(*)[64]> m_letters64;
	TextRenderer();
	ShaderHandler* m_TextShader;
	std::vector<RenderString> m_RenderStrings;
	unsigned int m_currentStringID = 0;
public:

	~TextRenderer();
	static TextRenderer& GetInstance();

	GLuint m_2DVAO;
	GLuint m_2DVBO[3];
	TextureInfo m_TextInfo;

	void LoadText(unsigned int _w, unsigned int _h);
	void LoadLetters();
	
	void RenderText(unsigned int _w, unsigned int _h);

	unsigned int AddString(std::string* _text, glm::vec3 _color, float _scale, float _x, float _y);
	void RemoveString(unsigned int _ID);
};

#endif