#include "Shader.h"
#include "Mesh.h"
#include <fstream>
#include <sstream>

Shader::Shader() : m_shader_ID(0), m_paths({})
{}

Shader::Shader(const vector<string> paths) 
	: m_shader_ID(0),  m_paths(paths)
{}

Shader::~Shader() 
{}

bool Shader::processShader()
{
	cout << "Load shader file: " << endl;

	GLuint vert_shader = 0;
	GLuint frag_shader = 0;
	GLuint geom_shader = 0;

	if (!compileShader(m_paths.at(0), GL_VERTEX_SHADER, vert_shader) || 
		!compileShader(m_paths.at(1), GL_FRAGMENT_SHADER, frag_shader))
		assert(0);

	if (m_paths.size() > 2)
	{
		compileShader(m_paths.at(2), GL_GEOMETRY_SHADER, geom_shader);
	}

	m_shader_ID = glCreateProgram();

	glAttachShader(m_shader_ID, vert_shader);
	glAttachShader(m_shader_ID, frag_shader);

	if (m_paths.size() > 2)
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

	if (m_paths.size() > 2)
		glDeleteShader(geom_shader);

	return true;
}

bool Shader::compileShader(const string& filePath, GLenum shaderType, GLuint& outShader)
{
	printf("  -Compile %s \n", filePath.c_str());

	ifstream shader_file(filePath);
	if (shader_file.is_open())
	{
		stringstream sstream;
		sstream << shader_file.rdbuf();

		string contents = sstream.str();
		const char* contents_char = contents.c_str();

		outShader = glCreateShader(shaderType);
		glShaderSource(outShader, 1, &contents_char, nullptr);
		glCompileShader(outShader);

		if (!isCompiled(outShader))
		{
			printf("Failed to compile shader %s \n", filePath.c_str());
			return false;
		}

		shader_file.close();
	}
	else
	{
		printf("Shader file not found! %s \n", filePath.c_str());
		return false;
	}

	return true;
}

bool Shader::isCompiled(GLuint& shader)
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

void Shader::setVec3(const string& name, glm::vec3& vector) const
{
	glUniform3fv(glGetUniformLocation(m_shader_ID, name.c_str()), 1, &vector[0]);
}

void Shader::setVec3(const string& name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(m_shader_ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const string& name, glm::mat4& matrix) const
{
	glUniformMatrix4fv(glGetUniformLocation(m_shader_ID, name.c_str()), 1, GL_FALSE, value_ptr(matrix));
}

void Shader::setPVM(glm::mat4& p, glm::mat4& v, glm::mat4& m) const
{
	setMat4("projection", p);
	setMat4("view", v);
	setMat4("model", m);
}

void Shader::setLight(Light& light)
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

void Shader::setMaterial(Material& material)
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