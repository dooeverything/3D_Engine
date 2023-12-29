#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/norm.hpp>

#include <string>
#include <iostream>
#include <vector>
#include <Math.h>

class Material;
class Texture;
class Light;

using namespace std;

class Shader
{
public:
	Shader(const GLuint& id, const string& name, const vector<string> paths);
	~Shader();

	GLuint getShaderId() { return m_shader_ID; };

	void setInt(const string& name, int value) const;
	void setFloat(const string& name, float value) const;
	void setVec2(const string& name, const glm::vec2& vector) const;
	void setVec3(const string& name, const glm::vec3& vector) const;
	void setVec3(const string& name, float x, float y, float z) const;
	void setMat4(const string& name, const glm::mat4& matrix) const;
	void setPVM(const glm::mat4& P, const glm::mat4& V, const glm::mat4& m) const;
	void setLight(const Light& light);
	void setMaterial(const Material& material);

	void load();
	void unload();

private:
	GLuint m_shader_ID;
	string m_name;
	vector<string> m_paths;
};

#endif // !SHADER_H