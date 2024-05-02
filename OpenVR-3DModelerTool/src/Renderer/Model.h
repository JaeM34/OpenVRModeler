#pragma once
#define STB_IMAGE_IMPLEMENTATION
#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "../OpenGL/Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
using namespace std;

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false);

class Model
{
public:
    // model data 
    vector<Mesh::Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    Shader shader;
    string directory;
    bool gammaCorrection;
    glm::mat4 m = glm::mat4(1.0f);      // Matrix for transformation

    // constructor, expects a filepath to a 3D model.
    Model(string const& path, bool gamma = false) : 
        shader(Shader("shaders/BaseLightedShader.shader")),
        gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw()
    {
        shader.SetVec3("light.direction", -0.2f, -1.0f, -0.3f);

        // light properties
        shader.SetVec3("light.ambient", 0.2f, 0.2f, 0.2f);
        shader.SetVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
        shader.SetVec3("light.specular", 1.0f, 1.0f, 1.0f);

        // material properties
        shader.SetFloat("material.shininess", 32.0f);

        shader.SetMat4("model", m);
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    // Scale
    void Scale(float scale) {
        m = glm::scale(m, glm::vec3(scale));  // Update the scale matrix
    }

    // Rotate
    void RotateX(float angles) {
        m = glm::rotate(m, glm::radians(angles), glm::vec3(1.0f, 0.0f, 0.0f));  // Update the rotation matrix
    }

    // Rotate
    void RotateY(float angles) {
        m = glm::rotate(m, glm::radians(angles), glm::vec3(0.0f, 1.0f, 0.0f));  // Update the rotation matrix
    }

    // Rotate
    void RotateZ(float angles) {
        m = glm::rotate(m, glm::radians(angles), glm::vec3(0.0f, 0.0f, 1.0f));  // Update the rotation matrix
    }

    // Transforms
    void Forward(float distance) {
        glm::mat4 transformation; // your transformation matrix.
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m, scale, rotation, translation, skew, perspective);

        glm::mat4 translat = glm::translate(glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, distance / 10))), translation);
        m = glm::scale(translat * glm::mat4_cast(rotation), scale);
    }

    void Strafe(float distance) {
        glm::mat4 transformation; // your transformation matrix.
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m, scale, rotation, translation, skew, perspective);

        glm::mat4 translat = glm::translate(glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(distance / 10, 0.0f, 0.0f))), translation);
        m = glm::scale(translat * glm::mat4_cast(rotation), scale);
    }

    void Upwards(float distance) {
        glm::mat4 transformation; // your transformation matrix.
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m, scale, rotation, translation, skew, perspective);

        glm::mat4 translat = glm::translate(glm::inverse(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, distance / 10, 0.0f))), translation);
        m = glm::scale(translat * glm::mat4_cast(rotation), scale);
    }

    void Transform(glm::mat4 mat4, glm::mat4 mat, glm::vec3 pos) {
        glm::mat4 transformation; // your transformation matrix.
        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(m, scale, rotation, translation, skew, perspective);

        glm::mat4 difference = m - mat;
        glm::vec3 xAxis(difference[0][0], difference[1][0], difference[2][0]);
        glm::vec3 yAxis(difference[0][1], difference[1][1], difference[2][1]);
        glm::vec3 zAxis(difference[0][2], difference[1][2], difference[2][2]);
        float distX = glm::length(xAxis);
        float distY = glm::length(yAxis);
        float distZ = glm::length(zAxis);

        distX = std::abs(translation.x - pos.x);
        distY = std::abs(translation.y - pos.y);
        distZ = std::abs(translation.z - pos.z);

        std::cout << "Distance" << distX << std::endl;
        std::cout << "Distance" << distY << std::endl;
        std::cout << "Distance" << distZ << std::endl;

        std::cout << "Translate Mat X: " << mat4[3][0] << std::endl;
        std::cout << "Translate Mat Y: " << mat4[3][1] << std::endl;
        std::cout << "Translate Mat Z: " << mat4[3][2] << std::endl;

        std::cout << "Model Mat X: " << m[3][0] << std::endl;
        std::cout << "Model Mat Y: " << m[3][1] << std::endl;
        std::cout << "Model Mat Z: " << m[3][2] << std::endl;

        glm::vec3 totalDistance = xAxis + yAxis + zAxis;
        float length = glm::length(totalDistance);
        std::cout << "Total distance: " << length << endl;
        length = distX + distY + distZ;
        std::cout << "Total distance2: " << length << endl;
        glm::mat4 translat = glm::translate(glm::inverse( glm::translate(glm::mat4(1.0f), glm::vec3(mat4[3][0] * length, mat4[3][1] * length, mat4[3][2] * length))), translation);
        glm::quat rotat = glm::quat_cast(glm::mat3(mat4));
        m = glm::scale(translat * glm::mat4_cast(rotation) * glm::inverse(glm::mat4_cast(rotat)), scale);
    }

    void SetMatrix(glm::mat4 mat) {
        m = mat;
    }

private:
    string m_path;
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));
        m_path = path;

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Mesh::Vertex> vertices;
        vector<unsigned int> indices;
        vector<Mesh::Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Mesh::Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Mesh::Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        for (Mesh::Texture diffuse : diffuseMaps) {
            shader.SetInt("material.diffuse", diffuse.id);
        }
        // 2. specular maps
        vector<Mesh::Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        for (Mesh::Texture specular : specularMaps) {
            shader.SetInt("material.specular", specular.id);
        }
        // 3. normal maps
        std::vector<Mesh::Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        for (Mesh::Texture normal : normalMaps) {
            shader.SetInt("material.normal", normal.id);
            shader.SetBool("material.normalLoad", true);
        }
        // 4. height maps
        std::vector<Mesh::Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
        for (Mesh::Texture height : heightMaps) {
            shader.SetInt("material.height", height.id);
        }
        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Mesh::Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Mesh::Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Mesh::Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
            }
        }
        // No texture included with model, attempt to load user defined textures
        if (mat->GetTextureCount(type) == 0) {
            string strr = m_path.substr((m_path.find_last_of('/') + 1));
            int lastLoc = strr.find_last_of('.');
            strr = strr.substr(0, lastLoc);
            strr = "textures/" + strr + "/" + typeName + ".png";
            Mesh::Texture texture;
            texture.id = TextureFromFile(strr.c_str(), this->directory);
            texture.type = typeName;
            texture.path = "shaders/default.png";
            textures.push_back(texture);
            textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecessary load duplicate textures.
        }
        return textures;
    }
};

unsigned int TextureFromFile(const char* path, const string& directory, bool gamma)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    // Texture not found at location, give a default one back
    if (!data) {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        data = stbi_load("shaders/default.png", &width, &height, &nrComponents, 0);
        path = "shaders/default.png";
    }
    if (data)
    {
        std::cout << "loaded texture at path: " << path << std::endl;
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    // Error, texture failed. Something seriously went wrong
    else
    {
        std::cout << "Texture loading failed" << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif