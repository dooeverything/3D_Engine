#version 330 core
layout (location = 0) in vec3 in_Pos;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec3 in_Tangent;
layout (location = 3) in vec2 in_TexCoord;
layout (location = 4) in ivec4 in_BoneIDs;
layout (location = 5) in vec4 in_Weights;

const int max_bones = 200;
const int max_bone_weights = 4;

out vec3 fragPos;
out vec3 normal;
out vec2 texCoord;
flat out ivec4 boneIDs;
out vec4 weights;
out vec3 posePos;

uniform mat4 boneMatrices[max_bones];

uniform bool animation;
uniform mat4 adjust; // Transformation matrix that transforms the vertex from origin to local space with correct position of model
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	//vec4 startPosition = nodeTransform * vec4(in_Pos, 1.0f);

	if(animation)
	{
		vec3 poseNormal = vec3(0.0f);

		mat4 boneTransform = mat4(0.0f);

		for(int i=0; i<max_bone_weights; ++i)
		{
			if(in_BoneIDs[i] == -1) continue;
			
			boneTransform += boneMatrices[in_BoneIDs[i]] * in_Weights[i];
		}

		vec4 posePosition = boneTransform * vec4(in_Pos, 1.0);

		posePos = vec3(posePosition.xyz);
		fragPos = vec3(model * posePosition);
		normal = mat3(transpose(inverse(model))) * in_Normal; 

		gl_Position = projection * view * model * posePosition;
	}
	else
	{
		fragPos = vec3(model * vec4(in_Pos, 1.0));
		normal = mat3(transpose(inverse(model))) * in_Normal;
		
		gl_Position = projection * view * model  * vec4(in_Pos, 1.0);
	}

	texCoord = in_TexCoord;
	weights = in_Weights;
	boneIDs = in_BoneIDs;
}