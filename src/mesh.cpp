#include "mesh.hpp"

#include <vector>

#include <glm/glm.hpp>

Mesh::Mesh()
{
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(0);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    for(auto const& [location, vbo] : m_attributes) {
        glDeleteBuffers(1, &vbo);
    }
    glDeleteBuffers(1, &m_ebo);
}

auto
Mesh::render() -> void
{
    if(m_texture != nullptr) {
        m_texture->activate();
    }

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    for(auto const& [location, vbo] : m_attributes) {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(location, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    }

    glDrawElements(GL_TRIANGLES, m_nrVertices, GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}

auto
Mesh::setVertices(std::vector<glm::vec3> const& vertices) -> void
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(vertices[0]),
            vertices.data(),
            GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

auto
Mesh::setVertices(
        std::vector<glm::vec3> const& vertices,
        int const start,
        int const size) -> void
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(
            GL_ARRAY_BUFFER,
            start * sizeof(vertices[0]),
            size * sizeof(vertices[0]),
            &vertices[start]);

    glBindVertexArray(0);
}

auto
Mesh::setIndices(std::vector<GLuint> const& indices) -> void
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(indices[0]),
            indices.data(),
            GL_DYNAMIC_DRAW);

    m_nrVertices = indices.size();

    glBindVertexArray(0);
}

auto
Mesh::newAttribute(int location) -> void
{
    glBindVertexArray(m_vao);

    GLuint vbo = 0;
    glEnableVertexAttribArray(location);
    glGenBuffers(location, &vbo);

    glBindVertexArray(0);

    // CPP20 {.vbo = vbo...}
    m_attributes[location] = vbo;
}

auto
Mesh::setAttribute(int location, std::vector<float> const& values) -> void
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_attributes[location]);
    glBufferData(
            GL_ARRAY_BUFFER,
            values.size() * sizeof(values[0]),
            values.data(),
            GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

auto
Mesh::setAttribute(
        int location,
        std::vector<float> const& values,
        int const start,
        int const size) -> void
{
    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_attributes[location]);
    glBufferSubData(
            GL_ARRAY_BUFFER,
            start * sizeof(values[0]),
            size * sizeof(values[0]),
            &values[start]);

    glBindVertexArray(0);
}
auto
Mesh::setTexture(std::shared_ptr<Texture> texture) -> void
{
    m_texture = std::move(texture);
}

auto
swap(Mesh& a, Mesh& b) -> void
{
    std::swap(a.m_vao, b.m_vao);
    std::swap(a.m_attributes, b.m_attributes);
    std::swap(a.m_vbo, b.m_vbo);
    std::swap(a.m_ebo, b.m_ebo);
    std::swap(a.m_nrVertices, b.m_nrVertices);
}
