#pragma once

class CPlayer : public CObject_D2D
{
protected:
	Vector2 pos{ 0,0 };
	Vector2 size{ 0,0 };
	float fSpeed{ 1.0f };

	bool active{ 0 };
	std::wstring m_name;

public:
	CPlayer(const std::string& name = "Player") : CObject_D2D(name)
	{
		m_name.clear();
	}
	virtual ~CPlayer()
	{
	}
	void set_speed(const float& speed) { fSpeed = speed; }
	void set_name(const std::wstring& name) { m_name = name; }

	void set(int x, int y)
	{
		pos.set(x, y);
	}


	void set(int x, int y, int w, int h)
	{
		set(x, y);
		size.set(w, h);
	}

	void update(const float& frame_time)
	{
		INPUT->KeyDown(YT_KEY::YK_LEFT);// pos.x -= fSpeed * frame_time;
		INPUT->KeyDown(YT_KEY::YK_RIGHT);// pos.x += fSpeed * frame_time;
		INPUT->KeyDown(YT_KEY::YK_UP);// pos.y -= fSpeed * frame_time;
		INPUT->KeyDown(YT_KEY::YK_DOWN);// pos.y += fSpeed * frame_time;
	}

	void render(ID2D1HwndRenderTarget *pRTV)
	{
		if (m_name.empty())
		{
			Draw::drawRect(pRTV, pos.x, pos.y, size.x, size.y, MY_COLOR(MyColorEnum::Aquamarine));
		}
		else
		{
			RENDERMGR_2D->Render(pRTV, m_name, RectF(pos.x - size.x, pos.y - size.y, pos.x + size.x, pos.y + size.y));
		}

	}
};

class NPC : public CPlayer
{
protected:
	Vector2 dir{ 0, 0 };

public:

	void update()
	{
		pos += dir * fSpeed;
	}
};