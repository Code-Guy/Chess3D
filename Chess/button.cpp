#include "button.h"

using namespace std;
using namespace ork;

Button::Button(string commonImgPath, string hoverImgPath, string pressImgPath, vec2i pos) : pos(pos)
{
	commonBtn = new Overlay2D(commonImgPath);
	hoverBtn = new Overlay2D(hoverImgPath);
	pressBtn = new Overlay2D(pressImgPath);

	size = commonBtn->GetSize();
	state = Common;
	isClicked = false;
}

Button::~Button()
{
	delete commonBtn;
	delete hoverBtn;
	delete pressBtn;
}

void Button::Render()
{
	switch(state)
	{
	case Common:
		commonBtn->Draw(pos.x, pos.y);
		break;
	case Hover:
		hoverBtn->Draw(pos.x, pos.y);
		break;
	case Press:
		pressBtn->Draw(pos.x, pos.y);
		break;
	}
}

void Button::MouseMove(int x, int y)
{
	if (Contains(x, y))
	{
		SetState(Hover);
	}
	else
	{
		SetState(Common);
	}
}

void Button::MousePress(int x, int y)
{
	if (Contains(x, y))
	{
		SetState(Press);
	}
}

void Button::MouseRelease(int x, int y)
{
	if (Contains(x, y))
	{
		isClicked = true;
	}
}

void Button::SetState(ButtonState state)
{
	this->state = state;
}

bool Button::Contains(int x, int y)
{
	x /= ratioX;
	y /= ratioY;

	return x > pos.x & x < pos.x + size.x && y > pos.y - size.y && y < pos.y;
}