#include "shaderProgram.h"

#include <iostream>

#include <stb_image.h>

ShaderProgram::ShaderProgram() : m_location(glCreateProgram())
{
    if(m_location == 0) {
        std::cerr << "Error creating shader program" << std::endl;
        throw;
    }
}

void
ShaderProgram::useShader(const Shader& shader)
{
    if(m_shaders.count(shader.m_type)) {
        glDetachShader(m_location, m_shaders[shader.m_type]);
    }

    glAttachShader(m_location, shader.m_location);
    m_shaders[shader.m_type] = shader.m_location;
}

extern GLuint G_OFFSET, G_TEXTURE_LOCATION;

void
ShaderProgram::compile()
{
    glLinkProgram(m_location);

    GLint success         = 0;
    GLchar errorLog[1024] = {0};

    glGetProgramiv(m_location, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_location, sizeof(errorLog), nullptr, errorLog);

        std::cerr << "Error linking shader program: " << errorLog << std::endl;
        throw;
    }

    glValidateProgram(m_location);
    glGetProgramiv(m_location, GL_VALIDATE_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_location, sizeof(errorLog), nullptr, errorLog);

        std::cerr << "Invalid shader program: " << errorLog << std::endl;
        throw;
    }

    glUseProgram(m_location);
    auto loadShader = [this](std::string name, GLuint& loc) {
        loc = glGetUniformLocation(m_location, name.c_str());
        if(loc == 0xFFFFFFFF) {
            std::cerr << "Failed to find variable " << loc << std::endl;
            throw;
        }
    };

    loadShader("offset", G_OFFSET);

    int width, height, nrChannels;
    unsigned char* const data =
            stbi_load("textures/texture.png", &width, &height, &nrChannels, 4);
    if(!data) {
        std::cout << "Failed to load texture" << std::endl;
        throw;
    }

    glGenTextures(1, &G_TEXTURE_LOCATION);
    glBindTexture(GL_TEXTURE_2D, G_TEXTURE_LOCATION);
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RGBA,
            width,
            height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);
}

void
ShaderProgram::setUniform(const std::string& name, const Matrix4f& value)
{
    glUniformMatrix4fv(locationOf(name), 1, GL_TRUE, &value.m[0][0]);
}

GLuint
ShaderProgram::locationOf(const std::string& name) const
{
    GLuint loc = glGetUniformLocation(m_location, name.c_str());
    if(loc == 0xFFFFFFFF) {
        std::cerr << "Failed to find variable " << name << std::endl;
        throw;
    }
    return loc;
}
