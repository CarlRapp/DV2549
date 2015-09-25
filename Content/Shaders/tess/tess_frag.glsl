#version 450

in vec2 tex; 
in vec3 facetNormal;
//in vec3 facetPosition;
out vec4 outColor;

uniform sampler2D gSampler; 

vec3 LightPosition = vec3(1,5,1);
vec3 LightDiffuse = vec3(0.7,0.7,0.7);

void main()
{
	vec3 L = normalize(LightPosition - vec3(1,1,1));//facetPosition);   
	vec4 Idiff = vec4(LightDiffuse * max(dot(facetNormal,L), 0.0),1.0);  
	Idiff = clamp(Idiff, 0.0, 1.0); 

	outColor = texture2D(gSampler, tex)*2;

	outColor.y = outColor.x;
	outColor.z = outColor.x;
	//outColor = Idiff;

	//outColor += vec4(facetNormal,1);
}