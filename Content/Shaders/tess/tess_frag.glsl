#version 450

in vec2 tex; 
in vec3 facetNormal;
in vec3 facetPosition;
out vec4 outColor;

uniform sampler2D gTexDiffuse;

uniform vec3 gEyePos;

uniform bool gFog;

vec3 LightDirection = normalize(vec3(1,-5.0,0.3));
vec3 LightDiffuse = vec3(0.9,0.9,0.9);
vec3 Lightspecular = vec3(0.1,0.1,0.1);
float Shininess = 2;

void main()
{
	float diffF = dot(-LightDirection,facetNormal);

	vec3 toEye = normalize(gEyePos- facetPosition);

	vec3 diff= vec3(0,0,0);
	vec3 spec = vec3(0,0,0);
	vec3 texColor = texture(gTexDiffuse, tex).xyz;

	if(diffF > 0.0f)
	{
		vec3 v = reflect(LightDirection,facetNormal);
		float specF = pow(max(dot(v, toEye),0.0),Shininess);

		diff = diffF*LightDiffuse;
		spec = specF*Lightspecular;//*texColor.z;

	}

	float d = distance(facetPosition,gEyePos);


	outColor.xyz = texColor * diff + spec;
	outColor.w = 1.0;

	if(gFog)
	{
		int fogStart = 360;

		if(d > fogStart)
			outColor.w = max(1-(d-fogStart)*0.01,0);
	}
}