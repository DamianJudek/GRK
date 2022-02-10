#version 410 core
out vec4 FragColor;

in vec3 TexCoords;
in vec4 pop;

uniform samplerCube skybox;

void main()
{   
	FragColor = texture(skybox, TexCoords);
	// Fog parameters, could make them uniforms and pass them into the fragment shader
	float fog_maxdist = 500.0;
	float fog_mindist = 0.5;
	vec4  fog_colour = vec4(0.03125, 0.1328125, 0.5390625, 0.9);
	
	// Calculate fog
	float dist = length(pop.xyz);
	float fog_factor = (fog_maxdist - dist) / (fog_maxdist - fog_mindist);
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	FragColor = mix(fog_colour, FragColor, fog_factor);
}