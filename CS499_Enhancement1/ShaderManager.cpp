
//Karina Washington November 2025 Update//

#include "ShaderManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ShaderManager::ShaderManager()
{
    m_programID = 0;
}

ShaderManager::~ShaderManager()
{
}

GLuint ShaderManager::LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string VertexShaderCode;
    std::ifstream vShaderFile(vertex_file_path);
    std::stringstream vShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    VertexShaderCode = vShaderStream.str();
    vShaderFile.close();

    std::string FragmentShaderCode;
    std::ifstream fShaderFile(fragment_file_path);
    std::stringstream fShaderStream;
    fShaderStream << fShaderFile.rdbuf();
    FragmentShaderCode = fShaderStream.str();
    fShaderFile.close();

    const char* vShaderSource = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &vShaderSource, NULL);
    glCompileShader(VertexShaderID);

    const char* fShaderSource = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &fShaderSource, NULL);
    glCompileShader(FragmentShaderID);

    GLuint ProgramID = glCreateProgram();
    m_programID = ProgramID;
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void ShaderManager::use()
{
    glUseProgram(m_programID);
}

void ShaderManager::setMat4Value(const char* name, const glm::mat4& mat)
{
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name), 1, GL_FALSE, &mat[0][0]);
}

void ShaderManager::setVec3Value(const char* name, const glm::vec3& value)
{
    glUniform3fv(glGetUniformLocation(m_programID, name), 1, &value[0]);
}

void ShaderManager::setVec4Value(const char* name, const glm::vec4& value)
{
    glUniform4fv(glGetUniformLocation(m_programID, name), 1, &value[0]);
}

void ShaderManager::setFloatValue(const char* name, float value)
{
    glUniform1f(glGetUniformLocation(m_programID, name), value);
}