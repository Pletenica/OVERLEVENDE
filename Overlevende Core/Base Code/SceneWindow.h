#ifndef __SceneWindow_H__
#define __SceneWindow_H__

#include "Module.h"
#include "Globals.h"
#include "glmath.h"
#include "WindowManager.h"
#include "SDL/include/SDL.h"
#include "ImGuizmo/ImGuizmo.h"
#include "MathGeoLib/src/Geometry/LineSegment.h"

class SceneWindow : public Window
{
public:

	SceneWindow();

	// Destructor
	virtual ~SceneWindow();

	bool Init();
	bool Draw(float dt);
	bool PreUpdate(float dt);
	bool CleanUp();

	void DoGuizmo();
	void ToogleModeGuizmo();
	ImVec2 GlobalToWindow(ImRect _rect, ImVec2 _point);
	GameObject* MeshIntersection(LineSegment _line);

private:
	ImGuizmo::OPERATION gOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE mode = ImGuizmo::MODE::WORLD;
};

#endif // __SceneWindow_H__