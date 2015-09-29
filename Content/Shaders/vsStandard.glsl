#version 450

in vec3 m_position;
in vec2 m_tex;
out vec2 texCoord;

uniform mat4 gPVM;

void main()
{
	texCoord = m_tex;
    gl_Position = gPVM * vec4(m_position, 1.0);
}