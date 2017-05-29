#pragma once

#include "stdafx.h"


class CameraInputScript : public Component
{
	CCamera_2D *m_pCamera;
	
	float	fSpeed{ 10.0f };
	Vector2		m_v2MaxBoundary;
	Vector2		m_v2ViewSize;

public:

	CameraInputScript(std::string name = "InputScript") : Component(name)
	{
	}

	 ~CameraInputScript()
	{

	}

	void Start(CGameObject_2D* gameObject)
	{
		Component::Start(gameObject);

		m_pCamera = static_cast<CCamera_2D*>(gameObject);
		
		SetView();

		gameObject = nullptr;
	}

	bool Release() override
	{
		Component::Release();
		m_pCamera = nullptr;
		return true;
	}

	void SetMaxBoundary(const Vector2& v2Maxboundary) { m_v2MaxBoundary = v2Maxboundary; }

	void Update(const float&  frame_time)
	{

		if (INPUT->KeyDown(YT_KEY::YK_W) || INPUT->KeyDown(YT_KEY::YK_UP))
			gameObject->Move(0, -fSpeed);
		if (INPUT->KeyDown(YT_KEY::YK_A) || INPUT->KeyDown(YT_KEY::YK_LEFT))
			gameObject->Move(-fSpeed, 0);
		if (INPUT->KeyDown(YT_KEY::YK_S) || INPUT->KeyDown(YT_KEY::YK_DOWN))
			gameObject->Move(0, fSpeed);
		if (INPUT->KeyDown(YT_KEY::YK_D) || INPUT->KeyDown(YT_KEY::YK_RIGHT))
			gameObject->Move(fSpeed, 0);


		//auto size = m_pCamera->GetViewSize();

		if (INPUT->KeyDown(YT_KEY::YK_O))
		{
			m_pCamera->OffSetViewSize(0.5f * fSpeed);
			SetView();
		}
		if (INPUT->KeyDown(YT_KEY::YK_P))
		{
			m_pCamera->OffSetViewSize(-0.5f * fSpeed);
			SetView();
		}
		//m_pCamera->SetSize(size);


		auto rect = gameObject->GetRect();

		if (rect.left < 0.0f)
		{
			rect.left = 0.0f;
			rect.right = rect.left + m_pCamera->GetWidth();
		}
		if (rect.top < 0.0f)
		{
			rect.top = 0.0f;
			rect.bottom = rect.top + m_pCamera->GetHeight();
		}
		if (rect.right > m_v2MaxBoundary.x)
		{
			rect.right = m_v2MaxBoundary.x;
			rect.left = rect.right - m_pCamera->GetWidth();
		}
		if (rect.bottom > m_v2MaxBoundary.y)
		{
			rect.bottom = m_v2MaxBoundary.y;
			rect.top = rect.bottom - m_pCamera->GetHeight();
		}
		m_pCamera->Set(rect);

	}

private:
	void SetView()
	{
		m_v2ViewSize.x = m_pCamera->GetCameView().right;
		m_v2ViewSize.y = m_pCamera->GetCameView().bottom;
	}

};