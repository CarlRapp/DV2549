#version 450

layout(triangles, equal_spacing, ccw) in;

in vec3 tcPosition[];
in vec2 tcTex[];
in vec3 tcNormal[];

//in mat4 tcModelM[];

out vec3 tePosition;
out vec2 teTex;
out vec3 teNormal;

//uniform mat4 gProj;
//uniform mat4 gView;

uniform mat4 gPV;

uniform sampler2D gTexHeight;
uniform sampler2D gTexNormal;

void main()
{
    vec3 p0 = gl_TessCoord.x * tcPosition[0];
    vec3 p1 = gl_TessCoord.y * tcPosition[1];
    vec3 p2 = gl_TessCoord.z * tcPosition[2];
    vec3 pos = p0 + p1 + p2;

    vec3 n0 = gl_TessCoord.x * tcNormal[0];
    vec3 n1 = gl_TessCoord.y * tcNormal[1];
    vec3 n2 = gl_TessCoord.z * tcNormal[2];

    vec2 tc0 = gl_TessCoord.x * tcTex[0];
    vec2 tc1 = gl_TessCoord.y * tcTex[1];
    vec2 tc2 = gl_TessCoord.z * tcTex[2];  
    teTex = tc0 + tc1 + tc2;

	float height = texture(gTexHeight, teTex).x;

	//rounding hack to not crack tessellation
	//if(teTex.x == 0 || teTex.x == 1 || teTex.y == 0 || teTex.y == 1)
	//{
	//	int rounded = int(height * 10);
	//	height = (float(rounded)/10);
	//}
	
	pos.y += (height)*4;

    gl_Position = gPV * vec4(pos, 1);

	teNormal    = texture(gTexNormal, teTex).xyz;
	
    tePosition  = pos;
}
