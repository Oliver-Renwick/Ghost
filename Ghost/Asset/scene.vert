#version 450

layout( location = 0 ) in vec3 inPos;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;
layout( location = 3 ) in vec3 inTangent;
layout( location = 4 ) in vec4 inColor;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;
layout (location = 4) out vec4 outShadowCoord;
layout (location = 5) out vec2 outUV;

layout(push_constant) uniform PushConsts {
    uint table_offset;
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
    outColor = inColor.xyz;
    outNormal = inNormal;


    gl_Position = ubo.proj * ubo.view * primitive.model * vec4(inPos, 1.0);
   
   vec4 pos = primitive.model * vec4(inPos, 1.0f);
   outNormal = mat3(primitive.model) * inNormal;
   outLightVec = normalize(ubo.lightPos.xyz - inPos);
   outViewVec = -pos.xyz;
   outUV = inUV;

    outShadowCoord = (biasMat * ubo.lightSpace * primitive.model) * vec4(inPos, 1.0f);
}