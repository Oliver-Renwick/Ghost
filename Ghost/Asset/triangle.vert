#version 450

layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;
layout( location = 3 ) in vec3 inTangent;
layout( location = 4 ) in vec4 inColor;

layout( location = 0 ) out vec4 outCol;
layout(location = 1) out vec3 fragPos;
layout(location = 2) out vec3 aNormal;
layout(location = 3) out vec3 LightPos;
layout(location = 4) out vec3 viewPos;
layout(location = 5) out vec4 outShadowCoord;

layout(push_constant) uniform PushConsts {
	mat4 model;
} primitive;

layout (binding = 0) uniform UniformBufferObject
{
    mat4 proj;
    mat4 view;
    mat4 lightSpace;
    vec4 lightPos;
    vec3 viewPos;
    float zNear;
    float zFar;
}ubo;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
    gl_Position = ubo.proj * ubo.view * primitive.model * vec4(inPos, 1.0);
    fragPos = vec3(primitive.model * vec4(inPos, 1.0));
    aNormal = mat3(transpose(inverse(primitive.model))) * inNormal;
    outCol = inColor;
    LightPos = vec3(ubo.lightPos);
    viewPos = ubo.viewPos;

    outShadowCoord = (biasMat * ubo.lightSpace * primitive.model) * vec4(inPos, 1.0f);
}