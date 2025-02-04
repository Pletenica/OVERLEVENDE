#include "Globals.h"
#include "Application.h"
#include "ModuleGameObject.h"
#include "ComponentTransform.h"

#include "lib/Assimp/include/vector3.h"
#include "MathGeoLib/src/Math/Quat.h"
#include"MathGeoLib/src/Math/float4x4.h"

#include "Assimp/include/cimport.h"
#include "Assimp/include/scene.h"
#include "Assimp/include/postprocess.h"

//Include ImGui
#include "Glew/include/glew.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_sdl.h"
#include "ImGui/imgui_impl_opengl3.h"

///WINDOW NOW
ComponentTransform::ComponentTransform(GameObject* _go) :Component(_go)
{
	gameobject = _go;
	type = ComponentType::C_Transform;
	position = { 0,0,0 };
	rotation = { 0,0,0 };
	scale = { 1,1,1 };
	local_transform.SetIdentity();
	global_transform.SetIdentity();
}

// Destructor
ComponentTransform::~ComponentTransform()
{
}

// Called before render is available
bool ComponentTransform::Enable()
{

	return true;
}

// Called before quitting
bool ComponentTransform::Disable()
{

	return true;
}

// Called before quitting
bool ComponentTransform::Update(float dt)
{

	return true;
}

void ComponentTransform::OnEditor(GameObject* _go)
{
	ComponentTransform* transform = (ComponentTransform*)_go->GetComponent(ComponentType::C_Transform);
	
	if (ImGui::CollapsingHeader("Transform")) {
		///////// POSITION ///////////
		float3 pos = transform->position;
		bool newPos = false;

		if(ImGui::Button("Reset Position", ImVec2(120, 20))) {
			ResetTransform();
		}

		ImGui::Text("Position");

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##posx", &pos.x, 0.5f, -500000, 500000)) newPos = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##posy", &pos.y, 0.5f, -500000, 500000)) newPos = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##posz", &pos.z, 0.5f, -500000, 500000)) newPos = true;
		ImGui::PopItemWidth();
		if (newPos) _go->transform->position = pos;

		///////// ROTATION ///////////
		float3 rot = transform->rotation;
		bool newRot = false;

		ImGui::Text("Rotation");

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##rotx", &rot.x, 0.5f, -360, 360)) newRot = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##roty", &rot.y, 0.5f, -360, 360)) newRot = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##rotz", &rot.z, 0.5f, -360, 360)) newRot = true;
		ImGui::PopItemWidth();
		if (newRot) _go->transform->rotation = rot;

		///////// SCALE ///////////
		float3 scale = transform->scale;
		bool newScale = false;

		ImGui::Text("Scale");

		ImGui::Text("X");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##scalex", &scale.x, 0.5f, -500000, 500000)) newScale = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text("Y");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##scaley", &scale.y, 0.5f, -500000, 500000)) newScale = true;
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::Text("Z");
		ImGui::SameLine();
		ImGui::PushItemWidth(70);
		if (ImGui::DragFloat("##scalez", &scale.z, 0.5f, -500000, 500000)) newScale = true;
		ImGui::PopItemWidth();
		if (newScale) _go->transform->scale = scale;

		if (newScale == true || newRot == true || newPos == true) {
			UpdateTransform();
		}
	}
}

void ComponentTransform::SetTransform(float3 _pos, Quat _rot, float3 _scale)
{
	position = _pos;
	rotation = _rot.ToEulerXYZ()*RADTODEG;
	scale = _scale;

	RecursiveUpdateTransform();
}

void ComponentTransform::SetTransformWithGlobal(float4x4 _global)
{
	global_transform = _global;
	if (gameobject->parent != nullptr) {
		local_transform = gameobject->parent->transform->global_transform.Inverted() * global_transform;
	}

	Quat rot;
	local_transform.Decompose(position, rot, scale);

	rotation = rot.ToEulerXYZ() * RADTODEG;

	UpdateTransform();
}

void ComponentTransform::RecursiveUpdateTransform()
{
	local_transform = float4x4::FromTRS(position, Quat::FromEulerXYZ(rotation.x * DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD), scale);

	if (gameobject->parent->transform != nullptr) {
		global_transform = gameobject->parent->transform->global_transform * local_transform;
	}
	if (gameobject->parent->transform == nullptr) {
		global_transform = local_transform;
	}

	for (int i = 0; i < gameobject->children.size(); i++) {
		gameobject->children[i]->transform->RecursiveUpdateTransform();
	}
}

void ComponentTransform::UpdateTransform()
{
	Quat _newRotation;
	_newRotation = _newRotation.FromEulerXYZ(rotation.x * DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD);
	SetTransform(position, _newRotation, scale);
	gameobject->UpdateAABB();
}

void ComponentTransform::ResetTransform()
{
	SetTransform(float3::zero, Quat::identity, float3::one);
}

void ComponentTransform::SaveComponent(JsonManager* _man)
{
	Component::SaveComponent(_man);

	_man->AddVector3("Position", position);
	_man->AddQuaternion("Rotation", Quat::FromEulerXYZ(rotation.x*DEGTORAD, rotation.y * DEGTORAD, rotation.z * DEGTORAD));
	_man->AddVector3("Scale", scale);
}

void ComponentTransform::LoadComponent(JsonManager* _man)
{
	position = _man->GetVector3("Position");
	rotation = _man->GetQuaternion("Rotation").ToEulerXYZ();
	scale = _man->GetVector3("Scale");

	local_transform.SetIdentity();
	global_transform.SetIdentity();

	Quat rot = Quat::FromEulerXYZ(rotation.x, rotation.y, rotation.z);
	SetTransform(position, rot, scale);
}
