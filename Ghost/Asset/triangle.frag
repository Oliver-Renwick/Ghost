#version 450

layout (binding = 1) uniform sampler2D shadowMap;

layout(location = 0) in vec4 inCol;
layout(location = 1) in vec3 fragPos;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec3 lightPos;
layout(location = 4) in vec3 viewPos;
layout(location = 5) in vec4 inShadowCoord;

layout(location = 0) out vec4 outColor;

#define amb 0.1


float textureProj(vec4 shadowCoord, vec2 off)
{
    float shadow = 1.0f;
    if(shadowCoord.z > -1.0f && shadowCoord.z < 1.0f)
    {
        float dist = texture(shadowMap, shadowCoord.st + off ).r;
        if(shadowCoord.w > 0.0 && dist < shadowCoord.z)
        {
            shadow = amb;
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
    
    //initialization
    vec3 objectColor = inCol.xyz;
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
    float ambientStrength = 0.1f;
    vec3 ambient =  ambientStrength  * lightColor;

    //Diffuse
    vec3 lightDir = normalize(lightPos - fragPos);
    vec3 norm = normalize(aNormal);

    float diff = max(dot(lightDir, norm), 0.0);
    vec3 diffuse = diff * lightColor;


    //Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 refectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, refectDir), 0.0), 32);
    vec3 specular = spec * specularStrength * lightColor;


    vec3 result = objectColor * (ambient + diffuse + specular);

    outColor = vec4(result * shadow, 1.0);
}