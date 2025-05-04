#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (binding = 1) uniform sampler2D shadowMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inViewVec;
layout (location = 3) in vec3 inLightVec;
layout (location = 4) in vec4 inShadowCoord;
layout (location = 5) in vec2 in_uv;

layout(set = 2, binding = 0) uniform texture2D Textures[];
layout(set = 3, binding = 0) uniform sampler ImmutableSampler;

layout(push_constant) uniform PushConsts {
	 // Offset 0 is used by vertex.
    uint table_offset;
} registers;

layout(location = 0) out vec4 outColor;

#define ambient 0.1


float textureProj(vec4 shadowCoord, vec2 off)
{
    float shadow = 1.0f;
    if(shadowCoord.z > -1.0f && shadowCoord.z < 1.0f)
    {
        float dist = texture(shadowMap, shadowCoord.st + off ).r;
        if(shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = ambient;
        }
    }
    return shadow;
}


float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}


void main() {


    float shadow =  filterPCF(inShadowCoord / inShadowCoord.w);
    
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = normalize(-reflect(L, N));

	vec4 col =  texture(sampler2D(Textures[registers.table_offset], ImmutableSampler), in_uv);

	vec3 diffuse = max(dot(N, L), ambient) * col.xyz;

    outColor = vec4(diffuse * shadow , 1.0 * col.w);
}