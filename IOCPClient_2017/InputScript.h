#pragma once

#include "stdafx.h"

class InputScript : public Component
{
	float	fSpeed{ 10.0f };

public:

	InputScript(std::string name = "InputScript") : Component(name)
	{
	}

	virtual ~InputScript()
	{

	}

	virtual void Start(CGameObject_2D* gameObject)
	{
		Component::Start(gameObject);
	}

	virtual void BeforeMove(CGameObject_2D* other)
	{

	}

	virtual void Update(const float&  frame_time)
	{

		if (INPUT->KeyDown(YT_KEY::YK_W) || INPUT->KeyDown(YT_KEY::YK_UP))
			gameObject->Move(0, -fSpeed);
		if (INPUT->KeyDown(YT_KEY::YK_A) || INPUT->KeyDown(YT_KEY::YK_LEFT))
			gameObject->Move(-fSpeed, 0);
		if (INPUT->KeyDown(YT_KEY::YK_S) || INPUT->KeyDown(YT_KEY::YK_DOWN))
			gameObject->Move(0, fSpeed);
		if (INPUT->KeyDown(YT_KEY::YK_D) || INPUT->KeyDown(YT_KEY::YK_RIGHT))
			gameObject->Move(fSpeed, 0);

		if (INPUT->KeyDown(YT_KEY::YK_O))
			gameObject->OffSetSize(0.5f * fSpeed);

		if (INPUT->KeyDown(YT_KEY::YK_P))
			gameObject->OffSetSize(-0.5f * fSpeed);


		auto rect = gameObject->GetRect();
		
		if (rect.left < 0.0f)
		{
			rect.left = 0.0f;
			rect.right = rect.left + gameObject->GetWidth();
		}
		if (rect.top < 0.0f)
		{
			rect.top = 0.0f;
			rect.bottom = rect.top + gameObject->GetHeight();
		}
		if (rect.right > 2380.0f)
		{
			rect.right = 2380.0f;
			rect.left = rect.right - gameObject->GetWidth();
		}
		if (rect.bottom > 793.0f)
		{
			rect.bottom = 793.0f;
			rect.top = rect.bottom - gameObject->GetHeight();
		}
		gameObject->Set(rect);

	}

	virtual void PlayerUpdate(float frame_time)
	{

	}


};