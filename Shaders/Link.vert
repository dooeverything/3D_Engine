#version 330 core
layout (location = 0) in vec3 in_pos;
layout (location = 1) in vec3 in_normal;
layout (location = 2) in vec3 in_tangent;
layout (location = 3) in vec2 in_texCoord;
layout (location = 4) in ivec4 in_boneIDs;
layout (location = 5) in vec4 in_weights;

const int max_bones = 200;
const int max_bone_weights = 4;

out vec3 frag_pos;
out vec3 frag_norm;
out vec2 frag_texCoord;
flat out ivec4 boneIDs;
out vec4 weights;
out vec3 pose_pos;

uniform mat4 bone_matrices[max_bones];

uniform bool animation;
uniform mat4 adjust; // Transformation matrix that transforms the vertex from origin to local space with correct position of model
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	//vec4 startPosition = nodeTransform * vec4(in_Pos, 1.0f);

	if(animation)
	{
		vec3 pose_normal = vec3(0.0f);

		mat4 bone_transform = mat4(0.0f);

		for(int i=0; i<max_bone_weights; ++i)
		{
			if(in_boneIDs[i] == -1) continue;
			
			bone_transform += bone_matrices[in_boneIDs[i]] * in_weights[i];
		}

		vec4 pose_position = bone_transform * vec4(in_pos, 1.0);

		pose_pos = vec3(pose_position.xyz);
		frag_pos = vec3(model * pose_position);
		frag_norm = mat3(transpose(inverse(model))) * in_normal; 

		gl_Position = projection * view * model * pose_position;
	}
	else
	{
		frag_pos = vec3(model * vec4(in_pos, 1.0));
		frag_norm = mat3(transpose(inverse(model))) * in_normal;
		
		gl_Position = projection * view * model  * vec4(in_pos, 1.0);
	}

	frag_texCoord = in_texCoord;
	weights = in_weights;
	boneIDs = in_boneIDs;
}