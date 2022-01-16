#version 410 core

//uniform vec3 objectColor;
uniform vec3 lightDir;
//uniform vec3 lightPos;
uniform vec3 cameraPos;
uniform sampler2D color_texture;
uniform sampler2D specular_texture;
float fog_maxdist = 100.0;
float fog_mindist = 0.1;
vec4  fog_colour = vec4(0.4, 0.4, 0.4, 1.0);

in vec3 interpNormal;
in vec3 fragPos;
in vec2 uvCoord;

void main()
{
	//float falloff = pow(length(cameraPos-fragPos),2)*0.01;
	vec3 color = texture(color_texture,uvCoord).rgb;
	vec3 spec = texture(specular_texture,uvCoord).rgb;
	//vec3 lightDir = normalize(lightPos-fragPos);
	vec3 V = normalize(cameraPos-fragPos);
	vec3 normal = normalize(interpNormal);
	vec3 R = reflect(-normalize(lightDir),normal);
	
	
	float dist = length(fragPos.xyz - cameraPos.xyz );
	float fog_factor = (fog_maxdist - dist)/(fog_maxdist - fog_mindist);
	fog_factor = clamp(fog_factor, 0.0, 1.0);

	float specular = pow(max(0,dot(R,V)),10);
	float diffuse = max(0,dot(normal,normalize(lightDir)));
	gl_FragColor = vec4(mix(color,color*diffuse+spec*specular,0.7), 1.0);
	gl_FragColor = mix(fog_colour, gl_FragColor, fog_factor);
	//gl_FragColor = vec4(mix(vec3(0.1,0.1,0.1),mix(color,color*diffuse+spec*specular,0.7),min(1,1/falloff)), 1.0);
}
