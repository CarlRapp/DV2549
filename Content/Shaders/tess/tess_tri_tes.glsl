//#version 410 core

//layout(triangles, equal_spacing, ccw) in;

//void main()
//{	
//	gl_Position.xyzw =	gl_in[0].gl_Position.xyzw * gl_TessCoord.x +
//						gl_in[1].gl_Position.xyzw * gl_TessCoord.y +
//						gl_in[2].gl_Position.xyzw * gl_TessCoord.z;
//}

#version 450

layout(triangles, equal_spacing, ccw) in;

in vec3 tcPosition[];
in vec2 tcTex[];
in vec3 tcNormal[];

//in mat4 tcModelM[];

//out vec3 tePosition;
out vec2 teTex;
out vec3 teNormal;


//uniform mat4 gProj;
//uniform mat4 gView;

uniform mat4 gPVM;
uniform mat4 gVM;

uniform sampler2D gSampler;

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

	float height = texture(gSampler, teTex).x;
    pos.y += (height - 0.5)*1.5;

    gl_Position = gPVM * vec4(pos, 1);

	teNormal    = normalize(n0+n1+n2);
	
   // tePosition  = vec3(gVM * vec4(pos,1.0)).xyz;
}
