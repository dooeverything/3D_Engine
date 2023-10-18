#pragma once

#include <GL/glew.h>
#include <string>
#include <iostream>
#include <vector>
#include "Math.h"

#include "Light.h"
#include "Material.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

class Shader
{
public:
	Shader();
	Shader(const vector<string> paths);
	~Shader();

	bool processShader();
	
	GLuint getShaderId() { return m_shader_ID; };

	void setInt(const string& name, int value) const;
	void setFloat(const string& name, float value) const;
	void setVec2(const string& name, glm::vec2& vector) const;
	void setVec3(const string& name, glm::vec3& vector) const;
	void setVec3(const string& name, float x, float y, float z) const;
	void setMat4(const string& name, const glm::mat4& matrix) const;
	void setPVM(const glm::mat4& P, const glm::mat4& V, glm::mat4& m) const;
	void setLight(Light& light);
	void setMaterial(Material& material);

	void load();
	void unload();

private:
	bool compileShader(const string& fileName, GLenum shaderType, GLuint& outShader);
	bool isCompiled(GLuint& shader);

	GLuint m_shader_ID;
	vector<string> m_paths;
};