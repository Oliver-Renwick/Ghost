#version 450

layout (location = 0) in vec3 inPos;

layout(binding = 0) uniform UBO
{
	mat4 depthMVP;
}ubo;

layout(push_constant) uniform PushConsts {
	uint materialID;
	mat4 model;
} primitive;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main()
{
	gl_Position = ubo.depthMVP * primitive.model * vec4(inPos, 1.0f);
}