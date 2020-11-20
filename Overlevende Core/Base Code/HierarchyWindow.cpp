#include "Globals.h"
#include "Application.h"
#include "HierarchyWindow.h"
#include "Glew/include/glew.h"

HierarchyWindow::HierarchyWindow() :Window()
{
	active = true;
}

// Destructor
HierarchyWindow::~HierarchyWindow()
{
}

// Called before render is available
bool HierarchyWindow::Init()
{
	bool ret = true;

	return ret;
}

bool HierarchyWindow::Draw(float dt)
{
	ImGui::Begin("Hierarchy", NULL);
	if (ImGui::Button("CLEAR", ImVec2(100, 30))) {
		ExternalApp->base_motor->inspector_window->DeleteSelectedGameObject();
		ExternalApp->scene_intro->ClearHierarchy();
	}
	ImGui::SameLine();
	if (ImGui::Button("CREATE EMPTY", ImVec2(130, 30))) {
		ExternalApp->scene_intro->CreateGameObject("Empty GameObject", ExternalApp->scene_intro->rootGO);
	}
	if(ExternalApp->scene_intro->rootGO != nullptr)
		RecursiveDraw(ExternalApp->scene_intro->rootGO);

	ImGui::End();

	return true;
}

// Called before quitting
bool HierarchyWindow::CleanUp()
{

	return true;
}

void HierarchyWindow::RecursiveDraw(GameObject* node)
{

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	if (node->children.size() == 0) 
	{
		flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
	}

	bool nodeIsOpened = ImGui::TreeNodeEx(node, flags, node->name.c_str());
	if (ImGui::IsItemClicked()) {
		ExternalApp->base_motor->inspector_window->PutNewSelectedGameObject(node);
	}

	bool showChildren = (node->children.size() == 0) ? false : nodeIsOpened;

	if (showChildren == true)
	{
		for (size_t i = 0; i < node->children.size(); ++i)
		{
			RecursiveDraw(node->children[i]);
		}
		ImGui::TreePop();
	}
}

// Called before quitting
bool HierarchyWindow::PreUpdate(float dt)
{

	return true;
}