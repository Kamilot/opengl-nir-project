#version 330 core

const int MAX_JOINTS = 50;

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 boneIds;
layout (location = 4) in vec4 boneWeights;

out vec3 Normal;
out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 jointTransforms[MAX_JOINTS];

void main()
{
    vec4 totalLocalPos = vec4(0.0);
    vec4 totalNormal = vec4(0.0);

    for(int i = 0; i < 4; i++){
    		mat4 jointTransform = jointTransforms[boneIds[i]];
    		vec4 posePosition = jointTransform * vec4(aPos, 1.0);
    		totalLocalPos += posePosition * boneWeights[i];

    		vec4 worldNormal = jointTransform * vec4(aNormal, 0.0);
    		totalNormal += worldNormal * boneWeights[i];
    }

    // gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_Position = projection * view * model * totalLocalPos;
    vec4 tempPos = model * totalLocalPos;
    FragPos = tempPos.xyz / tempPos.w; // World position
    Normal = (model * totalNormal).xyz;
    TexCoords = aTexCoords;
}
