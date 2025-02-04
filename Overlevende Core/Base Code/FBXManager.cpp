#include "FBXManager.h"
#include "Globals.h"
#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

#include"Application.h"

#include"ModuleRenderer3D.h"
#include"ModuleGameObject.h"
#include "ModuleSceneIntro.h"
#include "ModuleFileSystem.h"

#include"ComponentMaterial.h"
#include"ComponentMesh.h"
#include"ComponentTransform.h"

#include "MathGeoLib/src/Math/float3.h"
#include "MathGeoLib/src/Math/Quat.h"

#include"Devil/include/ilu.h"
#include"Devil/include/ilut.h"

#pragma comment (lib, "Assimp/libx86/assimp.lib")
#include<map>
#include<string>

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

void FBXLoader::ImportFBX(char* _buffer, int _size, int _idTexturesTemporal, const char* _name)
{
	const aiScene* scene = aiImportFileFromMemory(_buffer, _size, aiProcessPreset_TargetRealtime_MaxQuality,nullptr);

	if (scene != nullptr && scene->HasMeshes())
	{
		std::string _p = _name;
		_p = _p.substr(0, _p.find_last_of("."));
		scene->mRootNode->mName = _p;

		aiMesh* new_mesh = nullptr;
		std::vector<Mesh*> meshVector;
		std::vector<GLuint> texturesVector;
		std::map<uint, std::string> textureNames;

		aiString texName;
		if (scene->HasMaterials()) 
		{

			for (int i = 0; i < scene->mNumMaterials; i++)
			{
				aiMaterial* material = scene->mMaterials[i];
				uint numTextures = material->GetTextureCount(aiTextureType_DIFFUSE);

				if (numTextures > 0) 
				{
					material->GetTexture(aiTextureType_DIFFUSE, 0, &texName);

					char* buffer = nullptr;
					std::string _localpath = "Assets/Textures/" + (std::string)texName.C_Str();
					int size = ExternalApp->file_system->Load(_localpath.c_str(), &buffer);

					std::string _tn = (std::string)texName.C_Str();
					_tn = _tn.substr(0, _tn.find_last_of("."));
					//texturesVector.push_back(FBXLoader::LoadTexture(buffer, size, &_mesh->textureWidth, &_mesh->textureHeight, _tn));
					int id = FBXLoader::LoadTexture(buffer, size, nullptr, nullptr, _tn);
					texturesVector.push_back(id);
					textureNames[id] = texName.C_Str();

					/*_mesh->textureID = texturesVector[texturesVector.size() - 1];*/
					delete[] buffer;
				}
				else {
					texturesVector.push_back(0);
					textureNames[0] = "ErrorTex.dds";
				}

			}
		}

		for (size_t i = 0; i < texturesVector.size(); i++)
		{
			ExternalApp->renderer3D->cleanUpTextures.push_back(texturesVector[i]);
		}

		aiMeshToMesh(scene, meshVector, texturesVector);

		for (size_t i = 0; i < meshVector.size(); i++)
		{
			ExternalApp->renderer3D->cleanUpMeshes.push_back(meshVector[i]);
		}

		GameObject* _go = ExternalApp->scene_intro->CreateGameObject("ModelRoot", ExternalApp->scene_intro->rootGO);

		NodeToGameObject(scene, scene->mRootNode, _go, meshVector, textureNames);

		std::string _path = MODELS_PATH + _go->children[0]->name + MODELS_FORMAT;
		ExternalApp->scene_intro->SaveModel(_go, _path.c_str());


		aiReleaseImport(scene);
	}
	else {
		LOG("Error loading scene /Assets/FBXs/&s.fbx", _name);
	}
}

int FBXLoader::LoadTexture(char* buffer, int _size, int* _width, int* _height, std::string _texname, bool loadDDS)
{
	ILuint imageID;
	ilGenImages(1, &imageID);
	ilBindImage(imageID);

	if (!ilLoadL(IL_TYPE_UNKNOWN,buffer, _size))
	{
		LOG("Error loading texture");
	}

	if (loadDDS == true) {
		_texname = MATERIALS_PATH + _texname + ".dds";
		ImageToDDS(_texname);
	}

	if(_height != nullptr)
		*_height = ilGetInteger(IL_IMAGE_HEIGHT);

	if (_width != nullptr)
		*_width = ilGetInteger(IL_IMAGE_WIDTH);

	GLuint glID = ilutGLBindTexImage();
	
	ilDeleteImages(1, &imageID);
	glBindTexture(GL_TEXTURE_2D, 0);
	ilBindImage(0);

	return glID;
}


void FBXLoader::aiMeshToMesh(const aiScene* scene, std::vector<Mesh*>& meshVector, std::vector<GLuint>& textureVector)
{
	aiMesh* new_mesh;
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		Mesh* _mesh = new Mesh();
		new_mesh = scene->mMeshes[i];

		_mesh->meshPath = MESHES_PATH;
		_mesh->meshPath += new_mesh->mName.C_Str();
		_mesh->meshPath += MESHES_FORMAT;

		_mesh->name = new_mesh->mName.C_Str();
		
		LOG("%s", scene->mMeshes[i]->mName.C_Str());

		// copy vertices
		_mesh->num_vertices = new_mesh->mNumVertices;
		_mesh->name = (std::string)new_mesh->mName.C_Str();
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

		if (new_mesh->mMaterialIndex < textureVector.size()) 
		{
			_mesh->textureID = textureVector[new_mesh->mMaterialIndex];
		}

		SaveMeshToOwnFormat(_mesh);
		LoadMeshFromOwnFormat(_mesh->meshPath);
	

		_mesh->GenBuffers(MeshType::FBXNone);
		meshVector.push_back(_mesh);
	}

}

void FBXLoader::NodeToGameObject(const aiScene* scene, aiNode* node, GameObject* parent, std::vector<Mesh*>& meshVector, std::map<uint, std::string> textureMap)
{
	//GameObject* go = new GameObject(node->mName.C_Str(), parent);

	for (size_t i = 0; i < node->mNumMeshes; ++i)
	{
		GameObject* childGO = new GameObject(node->mName.C_Str() , parent);

		//Load mesh here
		ComponentTransform* transform = (ComponentTransform*)(childGO->GetComponent(ComponentType::C_Transform));
		ComponentTransform* transform_parent = (ComponentTransform*)childGO->parent->GetComponent(ComponentType::C_Transform);
		aiVector3D translation, scaling;
		aiQuaternion rotation;

		node->mTransformation.Decompose(scaling, rotation, translation);

		float3 pos(translation.x, translation.y, translation.z);
		float3 scale(scaling.x, scaling.y, scaling.z);
		Quat rot(rotation.x, rotation.y, rotation.z, rotation.w);

		transform->SetTransform(pos, rot, scale);
		
		ComponentMesh* meshRenderer = (ComponentMesh*)(childGO->CreateComponent(ComponentType::C_Mesh));
		meshRenderer->mesh = meshVector[node->mMeshes[i]];
		childGO->UpdateAABB();

		ComponentMaterial* materialRenderer = (ComponentMaterial*)(childGO->CreateComponent(ComponentType::C_Material));
		materialRenderer->textureID = meshRenderer->mesh->textureID;

		std::string name = textureMap[materialRenderer->textureID];

		std::string p = "Assets/Textures/" + (std::string)name.c_str();
		std::string l = (std::string)name.c_str();
		l = l.substr(0, l.find_last_of("."));
		l += ".dds";
		l = MATERIALS_PATH + l;
		materialRenderer->textureAssetsPath = p;
		materialRenderer->textureLibraryPath = l;
	}

	GameObject* check = parent;
	if(node->mNumChildren != 0)
		check = new GameObject(node->mName.C_Str(), parent);

	for (size_t i = 0; i < node->mNumChildren; i++)
	{
		NodeToGameObject(scene, node->mChildren[i], check, meshVector, textureMap);
	}
}

void FBXLoader::SaveMeshToOwnFormat(Mesh *_mesh)
{
	uint variables[4] = { _mesh->num_indices, _mesh->num_vertices, _mesh->num_normals, _mesh->num_textures };
	uint size = sizeof(variables) + (sizeof(uint) * _mesh->num_indices) + (sizeof(float) * _mesh->num_vertices * 3) + (sizeof(float) * _mesh->num_normals * 3) + (sizeof(float) * _mesh->num_textures * 2);

	char* fileBuffer = new char[size];
	char* cursor = fileBuffer;

	uint bytes = sizeof(variables);
	memcpy(cursor, variables, bytes);
	cursor += bytes;

	bytes = sizeof(uint) * _mesh->num_indices;
	memcpy(cursor, _mesh->indices, bytes);
	cursor += bytes;

	bytes = sizeof(float) * _mesh->num_vertices * 3;
	memcpy(cursor, _mesh->vertices, bytes);
	cursor += bytes;

	bytes = sizeof(float) * _mesh->num_normals * 3;
	memcpy(cursor, _mesh->normals, bytes);
	cursor += bytes;

	bytes = sizeof(float) * _mesh->num_textures * 2;
	memcpy(cursor, _mesh->textures, bytes);
	cursor += bytes;

	std::string path = MESHES_PATH + _mesh->name + MESHES_FORMAT;

	ExternalApp->file_system->Save(path.c_str(), fileBuffer, size);
	delete[] fileBuffer;
	fileBuffer = nullptr;
}

Mesh* FBXLoader::LoadMeshFromOwnFormat(std::string _meshname)
{
	Mesh* loadedMesh = new Mesh();
	char* fileBuffer = nullptr;

	uint size = ExternalApp->file_system->Load(_meshname.c_str(), &fileBuffer);

	char* cursor = fileBuffer;
	uint variables[4];

	uint bytes = sizeof(variables);
	memcpy(variables, cursor, bytes);
	loadedMesh->num_indices = variables[0];
	loadedMesh->num_vertices = variables[1];
	loadedMesh->num_normals = variables[2];
	loadedMesh->num_textures = variables[3];
	loadedMesh->meshPath = _meshname.c_str();
	cursor += bytes;


	bytes = sizeof(uint) * loadedMesh->num_indices;

	loadedMesh->indices = new uint[loadedMesh->num_indices];
	memcpy(loadedMesh->indices, cursor, bytes);
	cursor += bytes;

	loadedMesh->vertices = new float[loadedMesh->num_vertices * 3];
	bytes = sizeof(float) * loadedMesh->num_vertices * 3;
	memcpy(loadedMesh->vertices, cursor, bytes);
	cursor += bytes;

	loadedMesh->normals = new float[loadedMesh->num_normals * 3];
	bytes = sizeof(float) * loadedMesh->num_normals * 3;
	memcpy(loadedMesh->normals, cursor, bytes);
	cursor += bytes;

	loadedMesh->textures = new float[loadedMesh->num_textures * 2];
	bytes = sizeof(float) * loadedMesh->num_textures * 2;
	memcpy(loadedMesh->textures, cursor, bytes);
	cursor += bytes;

	delete[] fileBuffer;
	fileBuffer = nullptr;
	return loadedMesh;
}

void FBXLoader::ImageToDDS(std::string _texturename)
{
	ILuint size = 0;
    ILubyte* data = nullptr;
    ilSetInteger(IL_DXTC_FORMAT, IL_DXT5);
    size = ilSaveL(IL_DDS, nullptr, 0);
    if (size > 0) {
    	data = new ILubyte[size];
    	ilSaveL(IL_DDS, data, size);
		//std::string path = MATERIALS_PATH + _texturename + ".dds";
    	ExternalApp->file_system->Save(_texturename.c_str(), data, size);
    
    	delete[] data;
    	data = nullptr;
    }
}
