#version 410 core

uniform sampler2D textureSampler;
uniform vec3 lightDir;

in vec3 interpNormal;
in vec2 interpTexCoord;
in vec4 pop;

void main()
{
	// Fog parameters, could make them uniforms and pass them into the fragment shader
	float fog_maxdist = 50.0;
	float fog_mindist = 0.5;
	vec4  fog_colour = vec4(0.015625, 0.06640625, 0.2734375, 0.9);
	
	// Calculate fog
	float dist = length(pop.xyz);
	float fog_factor = (fog_maxdist - dist) / (fog_maxdist - fog_mindist);
	fog_factor = clamp(fog_factor, 0.0, 1.0);
	
	//outputColor = mix(fog_colour, shadedColor, fog_factor);

	vec2 modifiedTexCoord = vec2(interpTexCoord.x, interpTexCoord.y); // Poprawka dla tekstur Ziemi, ktore bez tego wyswietlaja sie 'do gory nogami'
	vec3 color = texture2D(textureSampler, modifiedTexCoord).rgb;
	vec3 normal = normalize(interpNormal);
	float diffuse = max(dot(normal, -lightDir), 0.0);
	gl_FragColor = vec4(color * diffuse, 1.0);
	gl_FragColor = mix(fog_colour, gl_FragColor, fog_factor);
}
