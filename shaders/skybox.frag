#version 410 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

float near = 0.1f;
float far = 70.0f;

float linearizeDepth(float depth)
{
	return (2.0 * near * far) / (far + near - (depth * 2.0 - 1.0) * (far - near));
}

float logisticDepth(float depth, float steepness = 0.5f, float offset = 3.0f)
{
	float zVal = linearizeDepth(depth);
	return (1 / (1 + exp(-steepness * (zVal - offset))));
}


void main()
{   
	float depth = logisticDepth(gl_FragCoord.z);
   // FragColor = texture(skybox, TexCoords) * (1.0f - depth) + vec4(depth * vec3(5.0f/255.0f, 35.0f/255.0f, 95.0f/255.0f), 1.0f);
	FragColor = texture(skybox, TexCoords);
}