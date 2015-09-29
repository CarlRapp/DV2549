//#version 410 core

//layout(vertices = 3) out;

//void main(void)
//{
//	gl_TessLevelOuter[0] = 2.0;
//	gl_TessLevelOuter[1] = 4.0;
//	gl_TessLevelOuter[2] = 6.0;

//	gl_TessLevelInner[0] = 8.0;

//	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
//}

#version 450

layout(vertices = 3) out;

in vec3 vPosition[];
in vec2 vTex[];
in vec3 vNormal[];
//in mat4 vModelM[];

out vec3 tcPosition[];
out vec2 tcTex[];
out vec3 tcNormal[];
//out mat4 tcModelM[];

uniform float innerTessLevel;
uniform float outerTessLevel;

void main()
{

    float inTess  = innerTessLevel;
    float outTess = outerTessLevel;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcNormal[gl_InvocationID]   = vNormal[gl_InvocationID];
    tcTex[gl_InvocationID] = vTex[gl_InvocationID];
	//tcModelM[gl_InvocationID] = vModelM[gl_InvocationID];

    if(gl_InvocationID == 0) 
	{
        gl_TessLevelInner[0] = inTess;
        gl_TessLevelInner[1] = inTess;
        gl_TessLevelOuter[0] = outTess;
        gl_TessLevelOuter[1] = outTess;
        gl_TessLevelOuter[2] = outTess;
        gl_TessLevelOuter[3] = outTess;
    }
}
