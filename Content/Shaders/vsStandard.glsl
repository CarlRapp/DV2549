#version 450

in vec3 vs_position;
in vec2 vs_tex;
out vec2 texCoord;

uniform mat4 gPVM;

void main()
{
	texCoord = vs_tex;
    gl_Position = gPVM * vec4(vs_position, 1.0);
}