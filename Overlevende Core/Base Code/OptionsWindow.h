#ifndef __OptionsWindow_H__
#define __OptionsWindow_H__

#include "Module.h"
#include "Globals.h"
#include "glmath.h"
#include "WindowManager.h"
#include "SDL/include/SDL.h"

class GameObject;

class OptionsWindow : public Window
{
public:

	OptionsWindow();

	// Destructor
	virtual ~OptionsWindow();

	bool Init();
	bool Draw(float dt);
	bool PreUpdate(float dt);
	bool CleanUp();
	void DoCheckers();
	void RecursiveNormals(GameObject* _root, bool _active);

	bool GetDrawAABB() { return drawaabb_bool; };
	bool GetDrawOBB() { return drawobb_bool; };

private:
	bool depth_bool = false;
	bool wireframe_bool = false;
	bool drawaabb_bool = false;
	bool drawobb_bool = false;
	bool cullface_bool = false;
	bool light_bool = false;
	bool texture_bool = true;
	bool normals_bool = false;
	std::string worldorlocalguizmo = "WORLD";
};

#endif // __OptionsWindow_H__