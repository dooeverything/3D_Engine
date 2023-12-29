#include "Shader.h"

#include <fstream>
#include <sstream>

#include "Light.h"
#include "Material.h"
#include "Mesh.h"

Shader::Shader(
	const GLuint& id, 
	const string& name, 
	const vector<string> paths) :
	m_shader_ID(id), m_name(name), m_paths(paths) {}

Shader::~Shader() {}

void Shader::load()
{
	//cout << "Shader SetActive " << m_shader_ID << " from " << m_paths.at(0) << endl;
	glUseProgram(m_shader_ID);
}

void Shader::setInt(const string& name, int value) const
{
	glUniform1i(glGetUniformLocation(m_shader_ID, name.c_str()), value);
}

void Shader::setFloat(const string& name, float value) const
{
	glUniform1f(glGetUniformLocation(m_shader_ID, name.c_str()), value);
}

void Shader::setVec2(const string& name, const glm::vec2& vector) const
{
	glUniform2f(glGetUniformLocation(m_shader_ID, name.c_str()), vector.x, vector.y);
}

void Shader::setVec3(const string& name, const glm::vec3& vector) const
{
	glUniform3fv(glGetUniformLocation(m_shader_ID, name.c_str()), 1, &vector[0]);
}

void Shader::setVec3(const string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(m_shader_ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const string& name, const glm::mat4& matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_shader_ID, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
}

void Shader::setPVM(const glm::mat4& P, const glm::mat4& V, const glm::mat4& m) const
{
	setMat4("projection", P);
	setMat4("view", V);
	setMat4("model", m);
}

void Shader::setLight(const Light& light)
{
	glm::vec3 dir = light.getDir();
	glm::vec3 amb = light.getAmb();
	glm::vec3 diff = light.getDiff();
	glm::vec3 spec = light.getSpec();
	setVec3("light.direction", dir);
	setVec3("light.ambient", amb);
	setVec3("light.diffuse", diff);
	setVec3("light.specular", spec);
}

void Shader::setMaterial(const Material& material)
{
	setVec3("mat.color", material.getBaseColor());
	setFloat("mat.metallic", material.getMetallic());
	setFloat("mat.roughness", material.getRoughness());
	setFloat("mat.ao", material.getAO());
}

void Shader::unload()
{
	glDeleteProgram(m_shader_ID);
}