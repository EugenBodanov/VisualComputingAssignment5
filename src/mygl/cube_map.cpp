#include "cube_map.h"
#include "shader.h"

#include <stdexcept>
#include <iostream>
#include <stb_image/stb_image.h>

/* Helper function to create the VAO, VBO, and EBO for the Skybox cube */
MeshCubeMap meshCubeMapCreate(const std::vector<Vector3D>& vertices, const std::vector<unsigned int>& indices)
{
    GLuint vao = 0, vbo = 0, ebo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector3D), vertices.data(), GL_STATIC_DRAW);
        glCheckError();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glCheckError();

        glEnableVertexAttribArray(eDataIdxCubeMap::PositionCubeMap);
        glVertexAttribPointer(eDataIdxCubeMap::PositionCubeMap, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3D), nullptr);
        glCheckError();
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return MeshCubeMap{vao, vbo, ebo, (unsigned int)vertices.size(), (unsigned int)indices.size()};
}

void meshCubeMapDelete(const MeshCubeMap& mesh)
{
    glDeleteBuffers(1, &mesh.vbo);
    glDeleteBuffers(1, &mesh.ebo);
    glDeleteVertexArrays(1, &mesh.vao);
}

TextureCube textureCubeLoad(const std::array<std::string, 6>& image_paths)
{
    int width = 0, height = 0, components = 0;

    stbi_set_flip_vertically_on_load(false); // Do not flip for cube map textures

    GLuint id = 0;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (unsigned int i = 0; i < image_paths.size(); ++i)
    {
        unsigned char* data = stbi_load(image_paths[i].c_str(), &width, &height, &components, 4);
        if (!data)
        {
            std::cerr << "[TextureCube] Couldn't load image file: " << image_paths[i] << std::endl;
            stbi_image_free(data);
            glDeleteTextures(1, &id);
            throw std::runtime_error("[TextureCube] Couldn't load image file: " + image_paths[i]);
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glCheckError();
        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glCheckError();

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return TextureCube{id, (unsigned int)width, (unsigned int)height};
}

void textureCubeDelete(const TextureCube& texture)
{
    glDeleteTextures(1, &texture.id);
}

CubeMap cubeMapCreate(const std::vector<Vector3D>& vertices, const std::vector<unsigned int>& indices, const std::array<std::string, 6>& image_paths)
{
    MeshCubeMap mesh = meshCubeMapCreate(vertices, indices);
    TextureCube texture = textureCubeLoad(image_paths);
    return CubeMap{mesh, texture, Matrix4D::identity()};
}

void cubeMapDelete(const CubeMap& cubeMap)
{
    meshCubeMapDelete(cubeMap.mesh);
    textureCubeDelete(cubeMap.texture);
}