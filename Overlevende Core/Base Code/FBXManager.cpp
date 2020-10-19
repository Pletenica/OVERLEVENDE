#include "FBXManager.h"
#include "Globals.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#include"ModuleRenderer3D.h"

#pragma comment (lib, "Assimp/libx86/assimp.lib")

void myFunction(const char* message, char* user)
{
	LOG(message);
}

void FBXLoader::EnableDebug()
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	stream.callback = myFunction;
	aiAttachLogStream(&stream);
}

void FBXLoader::DisableDebug()
{
	aiDetachAllLogStreams();
}

void FBXLoader::ImportFBX(const char* full_path, Mesh& _meshes, int _idTexturesTemporal)
{
	const aiScene* scene = aiImportFile(full_path, aiProcessPreset_TargetRealtime_MaxQuality);

	if (scene != nullptr && scene->HasMeshes())
	{
		// Use scene->mNumMeshes to iterate on scene->mMeshes array
		aiMesh* new_mesh = nullptr;
		for (int i = 0; i < scene->mNumMeshes; i++)
		{
			new_mesh = scene->mMeshes[i];
			LOG("%s", scene->mMeshes[i]->mName.C_Str());

			Mesh* _mesh = &_meshes;

			// copy vertices
			_mesh->num_vertices = new_mesh->mNumVertices;
			_mesh->vertices = new float[_mesh->num_vertices * 3];
			memcpy(_mesh->vertices, new_mesh->mVertices, sizeof(float) * _mesh->num_vertices * 3);
			LOG("New mesh with %d vertices", _mesh->num_vertices);

			if (new_mesh->HasNormals()) 
			{
				_mesh->num_normals = new_mesh->mNumVertices;
				_mesh->normals = new float[_mesh->num_normals * 3];
				memcpy(_mesh->normals, new_mesh->mNormals, sizeof(float) * _mesh->num_normals * 3);
				LOG("New mesh with %d normals", _mesh->num_normals);
			}

			if (new_mesh->HasTextureCoords(0))
			{
				_mesh->num_textures = new_mesh->mNumVertices;
				_mesh->textures = new float[new_mesh->mNumVertices * 2];

				for (unsigned int i = 0; i < _mesh->num_textures; i++)
				{
					_mesh->textures[i * 2] = new_mesh->mTextureCoords[0][i].x;
					_mesh->textures[i * 2 + 1] = new_mesh->mTextureCoords[0][i].y;
				}
				_mesh->textureID = _idTexturesTemporal;

				LOG("New mesh with %d texture coords", _mesh->num_textures);
			}

			// copy faces
			if (new_mesh->HasFaces())
			{
				_mesh->num_indices = new_mesh->mNumFaces * 3;
				_mesh->indices = new uint[_mesh->num_indices]; // assume each face is a triangle
				for (uint j = 0; j < new_mesh->mNumFaces; ++j)
				{
					if (new_mesh->mFaces[j].mNumIndices != 3)
					{
						LOG("WARNING, geometry face with != 3 indices!");
					}
					else
					{
						memcpy(&_mesh->indices[j * 3], new_mesh->mFaces[j].mIndices, 3 * sizeof(uint));
					}
				}
			}
		}

		_meshes.GenBuffers(MeshType::FBXNone);
		aiReleaseImport(scene);
	}
	else
		LOG("Error loading scene % s", full_path);
}
