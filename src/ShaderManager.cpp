#include "ShaderManager.h"
#include "Shader.h"
#include <fstream>
#include <sstream>

unordered_map<string, shared_ptr <Shader>> ShaderManager::shader_cache;

void ShaderManager::createShader(const string& name, const vector<string>& paths)
{
	shared_ptr<Shader> shader = compileShader(name, paths);
	if (shader == nullptr)
	{
		cout << "Failed to compile shader: " << name << endl;
		assert(0);
	}


	if (shader_cache.find(name) == shader_cache.end())
		shader_cache[name] = shader;

	if (name == "Point") cout << shader_cache[name]->getShaderId() << endl;
}

shared_ptr<Shader> ShaderManager::compileShader(const string& name, const vector<string>& paths)
{
	cout << "Load shader file: " << name << endl;

	GLuint id = glCreateProgram();

	vector<GLuint> shaders(paths.size(), 0);

	for (int i = 0; i < paths.size(); ++i)
	{
		if (!readShader(paths.at(i), types[i], shaders[i]))
			assert(0);
	}

	for (int i = 0; i < paths.size(); ++i)
	{
		glAttachShader(id, shaders[i]);
	}

	glLinkProgram(id);

	GLint success;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		memset(infoLog, 0, 512);
		glGetProgramInfoLog(id, 512, nullptr, infoLog);
		printf("Compile Failed: %s", infoLog);
		return nullptr;
	}

	for (int i = 0; i < paths.size(); ++i)
	{
		glDeleteShader(shaders[i]);
	}

	return make_shared<Shader>(id, name, paths);
}

bool ShaderManager::readShader(const string& filePath, GLenum shaderType, GLuint& outShader)
{
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

bool ShaderManager::isCompiled(const GLuint& shader)
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

shared_ptr<Shader> ShaderManager::getShader(const string& name)
{
	if (shader_cache.find(name) == shader_cache.end())
	{
		cout << "No Shader Exists: " << name << endl;
		assert(0);
	}

	return shader_cache[name];
}
