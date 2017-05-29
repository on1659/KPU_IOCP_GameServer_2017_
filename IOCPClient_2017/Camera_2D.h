#pragma once

namespace Radar
{

	static Vector2 convert_viewPos(const D2D_RECT_F& camera, const Vector2& v2);
}
class CCamera_2D : public CGameObject_2D
{
public:
	CCamera_2D(const std::string& name = "Camera_2D");

	~CCamera_2D();

	virtual void Create(const Vector2& position, const Vector2& size = Vector2(0.0f, 0.0f)) override;
	virtual void Create(const float& x, const float& y, const float& width = 0.0f, const float& height = 0.0f) override;


	void SetCamView(const D2D_RECT_F& rect);
	void SetCamView(const float& left, const float& top, const float& right, const float& bottom);
	D2D_RECT_F GetCameView() 
	{ 
		m_rtCamView.left = GetLeft();
		m_rtCamView.top  = GetTop();
		return m_rtCamView; 
	}

	void Update(const float& frame_time) override;

	bool isVisible(CGameObject_2D& gameObject);
	bool isVisible(D2D_RECT_F& rect);

	void SetSmooth(const bool& mode) { m_bSmooth = mode; }

	void SetVelocity(const Vector2& v2) { SetVelocity(v2.x, v2.y); }
	void SetVelocity(const float& x, const float& y) { m_v2Veolocity.x = x; m_v2Veolocity.y = y; }

	void SetOffset(const Vector2& v2) { SetOffset(v2.x, v2.y); }
	void SetOffset(const float& x, const float& y){	m_v2Offset.x = x; m_v2Offset.y = y; }

	void SetViewSize(const Vector2& v2) { SetViewSize(v2); }
	void SetViewSize(const float& w, const float& h) { m_rtCamView.right = w; 	m_rtCamView.bottom = h; }
	void OffSetViewSize(const float& offset)   
	{
		m_rtCamView.right += offset;
		m_rtCamView.bottom += offset;
	}
	Vector2 GetViewSize() const { return  Vector2{ m_rtCamView.right, m_rtCamView.bottom }; }

	/*
		D2D_RECT_F GetWorldPosition()
		{
			return m_rtWorldPosition;
		}
		void SetWorldPosition(const D2D_RECT_F& world_pos)
		{
			m_rtWorldPosition = world_pos;
		}

		void WorldMove(const Vector2& v2)
		{
			m_rtWorldPosition.left   += v2.x;
			m_rtWorldPosition.right  += v2.x;

			m_rtWorldPosition.top	 += v2.y;
			m_rtWorldPosition.bottom += v2.y;
		}
	*/

private:

	CGameObject_2D*		m_pPlayer{ nullptr };

	bool			    m_bSmooth{ true };
	Vector2				m_v2Veolocity;

	Vector2				m_v2Offset;

	//Vector2			m_v2LT_Boundary;
	//Vector2			m_v2RB_Boundary;

public:

	//static D2D_RECT_F	m_rtWorldPosition;
	D2D_RECT_F			m_rtCamView;


};



