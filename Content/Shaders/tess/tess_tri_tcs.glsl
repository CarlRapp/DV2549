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

uniform vec3 gEyePos;

float GetTessLevel(float Distance0, float Distance1)
{
    float AvgDistance = (Distance0 + Distance1) / 2.0;

    if (AvgDistance <= 10.0) 
	{
        return 64.0;
    }
    else if (AvgDistance <= 20.0) 
	{
        return 32.0;
    }
	else if (AvgDistance <= 40.0) 
	{
        return 16.0;
    }
	else if (AvgDistance <= 60.0) 
	{
        return 8.0;
    }
    else 
	{
        return 4.0;
    }
}

void main()
{

    float inTess  = innerTessLevel;
    float outTess = outerTessLevel;

    tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
    tcNormal[gl_InvocationID]   = vNormal[gl_InvocationID];
    tcTex[gl_InvocationID] = vTex[gl_InvocationID];

	float EyeToVertexDistance0 = distance(gEyePos, vPosition[0]);
    float EyeToVertexDistance1 = distance(gEyePos, vPosition[1]);
    float EyeToVertexDistance2 = distance(gEyePos, vPosition[2]);

    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance1, EyeToVertexDistance2);
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance2, EyeToVertexDistance0);
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance0, EyeToVertexDistance1);
    gl_TessLevelInner[0] = gl_TessLevelOuter[0];
	gl_TessLevelInner[1] = gl_TessLevelOuter[1];
}
