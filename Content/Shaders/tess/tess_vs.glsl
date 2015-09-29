//#version 450

//in vec3 m_position;
//in vec2 m_tex;
//out vec2 texCoord;

//uniform mat4 gPVM;

//void main()
//{
//	texCoord = m_tex;
//    gl_Position = gPVM * vec4(m_position, 1.0);
//}

#version 450

in vec3 in_position;
in vec2 in_tex;
in vec3 in_normal;
//in mat4 in_modelM;

out vec3 vPosition;
out vec2 vTex;
out vec3 vNormal;
//out mat4 vModelM;

uniform mat4 gM;

void main() 
{
    vPosition = (gM * vec4(in_position,1)).xyz;
    vTex = in_tex;
	vNormal = in_normal;
	//vModelM = in_modelM;
}
