#ifndef _BUTTON_H
#define _BUTTON_H

#include <string>
#include "ork/math/vec2.h"
#include "overlay2d.h"

enum ButtonState
{
	Common, Hover, Press
};

class Button
{
public: 
	Button(std::string commonImgPath, std::string hoverImgPath, std::string pressImgPath, ork::vec2i pos);
	~Button();

	void Render();

	void MouseMove(int x, int y);
	void MousePress(int x, int y);
	void MouseRelease(int x, int y);

	bool isClicked;

private:
	Overlay2D *commonBtn;
	Overlay2D *hoverBtn;
	Overlay2D *pressBtn;

	ork::vec2i pos;
	ork::vec2i size;

	ButtonState state;

	void SetState(ButtonState state);
	bool Contains(int x, int y);
};

#endif //_BUTTON_H