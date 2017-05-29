#include "stdafx.h"
#include "Camera_2D.h"

//D2D_RECT_F CCamera_2D::m_rtWorldPosition{ 0.0f, 0.0f, 0.0f, 0.0f };
//D2D_RECT_F CCamera_2D::m_rtCamView{ 0.0f, 0.0f, 0.0f, 0.0f };

namespace Radar
{
	Vector2 convert_viewPos(const D2D_RECT_F& camera, const Vector2& v2)
	{
		return (v2 - camera);
	}
}

CCamera_2D::CCamera_2D(const std::string& name) : CGameObject_2D(name)
{
}

CCamera_2D::~CCamera_2D()
{
}

void CCamera_2D::Create(const Vector2& position, const Vector2& size)
{
	CCamera_2D::Create(position.x, position.y, size.x, size.y);
}

void CCamera_2D::Create(const float& x, const float& y, const float& width, const float& height)
{
	CGameObject_2D::Create(x, y, width, height);
	SetCamView(0.0f, 0.0f, width/2, height/2);
}

void CCamera_2D::SetCamView(const D2D_RECT_F& rect)
{
	m_rtCamView = rect;
}

void CCamera_2D::SetCamView(const float& left, const float& top, const float& right, const float& bottom)
{
	SetCamView(RectF(left, top, right, bottom));
}

void CCamera_2D::Update(const float& frame_time)
{
	CGameObject_2D::Update(frame_time);

	if (m_pPlayer)
	{
		//auto dir = m_pPlayer->GetPosition() - GetWorldPosition();
		auto dir = m_pPlayer->GetPosition() - GetRect();
		
		// offset범위보다 작게 있으면 무시
		if (dir.scale() < m_v2Offset.scale())
			return;

		dir.normalize();
		Move(dir * m_v2Veolocity);
		//WorldMove(dir * m_v2Veolocity);

	}
}

bool CCamera_2D::isVisible(CGameObject_2D& gameObject)
{
	return GetBoundingBox().collision(gameObject.GetBoundingBox());
}

bool CCamera_2D::isVisible(D2D_RECT_F& rect)
{
	return GetBoundingBox().collision(rect);
}
