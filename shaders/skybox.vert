#version 410 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;
out vec4 pop;

uniform mat4 projectionView;

void main()
{
    TexCoords = aPos;
    gl_Position = projectionView * vec4(aPos, 1.0);
    pop = gl_Position;
}  