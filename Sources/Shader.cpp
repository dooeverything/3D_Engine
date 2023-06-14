#include "Shader.h"
#include "Mesh.h"
#include <fstream>
#include <sstream>

Shader::Shader() : m_shader_ID(0) {}

Shader::~Shader() {}

bool Shader::loadShaderFile(const string& vert_path, const string& frag_path, const string& geom_path)
{
	//cout << "Load Shader File" << endl;

	GLuint vert_shader = 0;
	GLuint frag_shader = 0;
	GLuint geom_shader = 0;

	if (!CompileShader(vert_path, GL_VERTEX_SHADER, vert_shader) || 
		!CompileShader(frag_path, GL_FRAGMENT_SHADER, frag_shader))
		return false;

	m_shader_ID = glCreateProgram();

	glAttachShader(m_shader_ID, vert_shader);
	glAttachShader(m_shader_ID, frag_shader);

	if (geom_path != "")
		glAttachShader(m_shader_ID, geom_shader);

	glLinkProgram(m_shader_ID);

	GLint success;
	glGetProgramiv(m_shader_ID, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		memset(infoLog, 0, 512);
		glGetProgramInfoLog(m_shader_ID, 512, nullptr, infoLog);
		printf("Compile Failed: %s", infoLog);
		return false;
	}

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	if (geom_path != "")
		glDeleteShader(geom_shader);

	return true;
}

bool Shader::CompileShader(const string& filePath, GLenum shaderType, GLuint& outShader)
{
	//printf("Compile %s \n", filePath.c_str());

	ifstream shaderFile(filePath);
	if (shaderFile.is_open())
	{
		stringstream sstream;
		sstream << shaderFile.rdbuf();

		string contents = sstream.str();
		const char* contentsChar = contents.c_str();

		outShader = glCreateShader(shaderType);

		glShaderSource(outShader, 1, &contentsChar, nullptr);
		glCompileShader(outShader);

		if (!IsCompiled(outShader))
		{
			printf("Failed to compile shader %s \n", filePath.c_str());
			return false;
		}

		shaderFile.close();
	}
	else
	{
		printf("Shader file not found! %s \n", filePath.c_str());
		return false;
	}

	return true;
}

bool Shader::IsCompiled(GLuint& shader)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	
	if (status != GL_TRUE)
	{
		char buffer[512];
		memset(buffer, 0, 512);
		glGetShaderInfoLog(shader, 511, nullptr, buffer);
		printf("GLSL Complile Failed: %s", buffer);
		return false;
	}

	return true;
}

void Shader::load()
{
	//cout << "Shader SetActive " << m_shader_ID << endl;
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

void Shader::setVec3(const string& name, vec3 vector) const
{
	glUniform3fv(glGetUniformLocation(m_shader_ID, name.c_str()), 1, &vector[0]);
}

void Shader::setVec3(const string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(m_shader_ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const string& name, mat4& matrix) const
{
	//cout << "Set matrix " << name << ": ";
	//cout << matrix << endl;
	glUniformMatrix4fv(glGetUniformLocation(m_shader_ID, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
	//cout << endl;
}

void Shader::setPVM(glm::mat4& p, glm::mat4& v, glm::mat4& m) const
{
	setMat4("projection", p);
	setMat4("view", v);
	setMat4("model", m);
}

void Shader::setLight(Light& light)
{
	setVec3("dir_light.direction", light.getDir());
	setVec3("dir_light.ambient", light.getAmb());
	setVec3("dir_light.diffuse", light.getDiff());
	setVec3("dir_light.specular", light.getSpec());
}

void Shader::unload()
{
	glDeleteProgram(m_shader_ID);
}