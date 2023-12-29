#pragma once

#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <iostream>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>

using namespace std;

class Shader;

static const vector<GLenum> types = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
	GL_GEOMETRY_SHADER
};

class ShaderManager
{
public:
	static void createShader(const string& name, const vector<string>& paths);
	static shared_ptr<Shader> getShader(const string& name);

private:
	ShaderManager() = delete;

	static unordered_map<string, shared_ptr<Shader>> shader_cache;
	static shared_ptr<Shader> compileShader(const string& name, const vector<string>& paths);
	static bool readShader(const string& filePath, GLenum shaderType, GLuint& outShader);
	static bool isCompiled(const GLuint& shader);

};

#endif // !SHADERMANAGER_H