#version 450

layout (set = 1, binding = 0) uniform samplerCube samplerCubeMap;

layout(location = 0) in vec3 UVW;

layout(location = 0) out vec4 outFrag;


void main()
{
	vec3 uvPos = UVW;
	uvPos = vec3(uvPos.x, uvPos.y * -1.0, uvPos.z);
	outFrag = texture(samplerCubeMap, uvPos);
}
