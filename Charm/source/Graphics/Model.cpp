#include "Graphics/Model.h"
#include "Graphics/Renderer.h"
#include "Core/AssetManager.h"
#include "Core/Log.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace Charm::Core;

namespace Charm
{
    namespace Graphics
    {
        namespace Models
        {
            using MeshMap = std::unordered_map<u32, u32>;
            void ProcessNode(Model& model, aiNode* assimpNode, const aiScene* scene, const std::filesystem::path& directory, MeshMap& processedMeshes);
            Mesh ProcessMesh(Model& model, aiMesh* assimpMesh, const aiScene* scene, const std::filesystem::path& directory);

            Model Load(const std::filesystem::path& path)
            {
                Model model;

                Assimp::Importer importer;
                const u32 importFlags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices;
                const aiScene* scene = importer.ReadFile(path, importFlags);
                ASSERT_RETURN((scene != NULL && !(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) && scene->mRootNode != NULL), model, "Failed to load model %s!", path.c_str());

                model.materials.resize(scene->mNumMaterials);

                // Maps an Assimp mesh index to the index of its already-processed Mesh in model.meshes.
                // When a node references the same aiMesh index a second time, we reuse the canonical
                // Mesh entry (sharing GPU resources) rather than allocating a new VAO/VBO.
                MeshMap processedMeshes;
                processedMeshes.reserve(scene->mNumMeshes);

                const std::filesystem::path modelDirectory = path.parent_path();
                ProcessNode(model, scene->mRootNode, scene, modelDirectory, processedMeshes);
                model.isValid = true;

                INFO("Model \"%s\" loaded successfully with %d meshes and %d materials", path.c_str(), model.meshes.size(), model.materials.size());
                return model;
            }

            void Unload(Model& model)
            {
                for (Mesh& mesh : model.meshes)
                    Meshes::Unload(mesh);
            }

            void ProcessNode(Model& model, aiNode* assimpNode, const aiScene* scene, const std::filesystem::path& directory, MeshMap& processedMeshes)
            {
                for (u32 i = 0; i < assimpNode->mNumMeshes; i++)
                {
                    const u32 assimpMeshIndex = assimpNode->mMeshes[i];
                    const auto it = processedMeshes.find(assimpMeshIndex);

                    if (it != processedMeshes.end())
                    {
                        // This aiMesh has already been processed. Copy the canonical Mesh struct
                        // so it shares the same VAO/VBO/IBO, but mark it as non-owning to
                        // prevent Unload() from double-deleting those GPU resources.
                        Mesh duplicate = model.meshes[it->second];
                        duplicate.ownsGPUResources = false;
                        model.meshes.emplace_back(std::move(duplicate));
                    }
                    else
                    {
                        const u32 canonicalIndex = (u32)model.meshes.size();
                        processedMeshes[assimpMeshIndex] = canonicalIndex;

                        aiMesh* assimpMesh = scene->mMeshes[assimpMeshIndex];
                        model.meshes.emplace_back(ProcessMesh(model, assimpMesh, scene, directory));
                    }
                }

                for (u32 i = 0; i < assimpNode->mNumChildren; i++)
                    ProcessNode(model, assimpNode->mChildren[i], scene, directory, processedMeshes);
            }

            Mesh ProcessMesh(Model& model, aiMesh* assimpMesh, const aiScene* scene, const std::filesystem::path& directory)
            {
                std::vector<MeshVertex> vertices;
                vertices.resize(assimpMesh->mNumVertices);

                for (u32 i = 0; i < assimpMesh->mNumVertices; i++)
                {
                    MeshVertex& vertex = vertices[i];
                    vertex.position.x = assimpMesh->mVertices[i].x;
                    vertex.position.y = assimpMesh->mVertices[i].y;
                    vertex.position.z = assimpMesh->mVertices[i].z;

                    if (assimpMesh->HasNormals())
                    {
                        vertex.normal.x = assimpMesh->mNormals[i].x;
                        vertex.normal.y = assimpMesh->mNormals[i].y;
                        vertex.normal.z = assimpMesh->mNormals[i].z;
                    }

                    if (assimpMesh->HasTextureCoords(0))
                    {
                        vertex.texCoord.x = assimpMesh->mTextureCoords[0][i].x;
                        vertex.texCoord.y = assimpMesh->mTextureCoords[0][i].y;
                    }
                    else
                    {
                        vertex.texCoord.x = 0.f;
                        vertex.texCoord.y = 0.f;
                    }
                }

                u32 indexCount = 0;
                for (u32 i = 0; i < assimpMesh->mNumFaces; i++)
                {
                    aiFace& face = assimpMesh->mFaces[i];
                    indexCount += face.mNumIndices;
                }

                std::vector<u32> indices;
                indices.reserve(indexCount);

                for (u32 i = 0; i < assimpMesh->mNumFaces; i++)
                {
                    aiFace& face = assimpMesh->mFaces[i];
                    for (u32 j = 0; j < face.mNumIndices; j++)
                        indices.emplace_back(face.mIndices[j]);
                }

                Mesh mesh;
                Meshes::Validate(mesh, vertices.data(), vertices.size(), indices.data(), indices.size());

                if (assimpMesh->mMaterialIndex >= 0)
                {
                    mesh.materialIndex = assimpMesh->mMaterialIndex;
                    aiMaterial* assimpMaterial = scene->mMaterials[mesh.materialIndex];
                    Material& material = model.materials[mesh.materialIndex];

                    aiColor4D albedoColor;
                    if (assimpMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, albedoColor) == aiReturn_SUCCESS)
                    {
                        material.albedo.r = albedoColor.r;
                        material.albedo.g = albedoColor.g;
                        material.albedo.b = albedoColor.b;
                        material.albedo.a = albedoColor.a;
                    }

                    aiString albedoTexturePath;
                    assimpMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &albedoTexturePath);
                    if (!albedoTexturePath.Empty())
                    {
                        const std::filesystem::path albedoPath = directory / albedoTexturePath.C_Str();
                        AssetHandle albedoTextureHandle = AssetManager::Import(albedoPath, AssetType::Texture);
                        Texture* albedoTexture = AssetManager::GetAsset<Texture>(albedoTextureHandle);
                        material.albedoTexture = albedoTexture;
                    }

                    material.shader = &Renderer::GetShaderBlinnPhong();
                }

                return mesh;
            }
        }
    }
}
