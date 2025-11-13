#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

class ShaderManager
{
public:
    ShaderManager();
    ~ShaderManager();

    GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path);
    void use();

    void setMat4Value(const char* name, const glm::mat4& mat);
    void setVec3Value(const char* name, const glm::vec3& value);
    void setVec4Value(const char* name, const glm::vec4& value);
    void setFloatValue(const char* name, float value);

    // Enhancement: Light intensity control
    void SetLightIntensity(float value)
    {
        setFloatValue("lightIntensity", value);
    }

private:
    GLuint m_programID;
};

#endif