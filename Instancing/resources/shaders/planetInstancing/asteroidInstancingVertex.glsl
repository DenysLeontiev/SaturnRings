#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float uTime;

vec3 rotateAroundAxis(vec3 v, vec3 axis, float angle);

void main()
{
	TexCoords = aTexCoords;

	float id = float(gl_InstanceID);
	vec3 tumbleAxis = normalize(vec3(
        sin(id * 12.9898),
        cos(id * 78.233),
        sin(id * 45.164)));

	float tumbleSpeed = 0.5 + fract(sin(id * 93.123) * 43758.5453) * 2.0;
    float tumbleAngle = uTime * tumbleSpeed;

    vec3 localPos = rotateAroundAxis(aPos, tumbleAxis, tumbleAngle);

    gl_Position = projection * view * model * aInstanceMatrix * vec4(localPos, 1.0f);
}

vec3 rotateAroundAxis(vec3 v, vec3 axis, float angle) 
{
    axis = normalize(axis);
    return v * cos(angle) + cross(axis, v) * sin(angle) + axis * dot(axis, v) * (1.0 - cos(angle));
}