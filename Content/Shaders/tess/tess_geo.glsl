//#version 410 core

//layout(triangles, invocations = 1) in;
//layout(line_strip, max_vertices = 4) out;


//void main(void)
//{
//	gl_Position = gl_in[0].gl_Position;
//	EmitVertex();	
//	gl_Position = gl_in[1].gl_Position;
//	EmitVertex();
//	gl_Position = gl_in[2].gl_Position;
//	EmitVertex();
	
//	EndPrimitive();
//}

#version 450

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

//in vec3 tePosition[3];
in vec2 teTex[3];
in vec3 teNormal[3];
in vec3 tePosition[3];
out vec3 facetNormal;
out vec2 tex;
out vec3 facetPosition;

void main() 
{
	facetNormal = teNormal[0];
    //facetNormal = normalize(cross(gl_in[0].gl_Position.xyz,gl_in[1].gl_Position.xyz)*teNormal[0]);
    tex = teTex[0];
    gl_Position = gl_in[0].gl_Position; 
	facetPosition = tePosition[0];
	EmitVertex();

	facetNormal = teNormal[1];
    //facetNormal = normalize(cross(gl_in[1].gl_Position.xyz,gl_in[2].gl_Position.xyz)*teNormal[1]);
    tex = teTex[1];
    gl_Position = gl_in[1].gl_Position; 
	facetPosition = tePosition[1];
	EmitVertex();

	facetNormal = teNormal[2];
	//facetNormal = normalize(cross(gl_in[2].gl_Position.xyz,gl_in[0].gl_Position.xyz)*teNormal[2]);
    tex = teTex[2];
    gl_Position = gl_in[2].gl_Position; 
	facetPosition = tePosition[2];
	EmitVertex();

    EndPrimitive();
}
