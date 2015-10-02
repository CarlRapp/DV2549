#version 450

in vec2 texCoord; 
out vec4 outColor;

uniform sampler2D gTex; 

void main()
{
	outColor.xyz = texture2D(gTex, texCoord).xyz;
	outColor.w = 1;

	//outColor = vec4(1,1,1,1);
}