#include "stdafx.h"
#include "SceneMain.h"
#include "GameFrameWork_D2D.h"

#include "Draw.h"
#include "InputScript.h"
#include "CameraInputScript.h"
#include "MapObject.h"


CSceneMain::CSceneMain(const std::string& name) : CSceneState(name)
{
}


CSceneMain::~CSceneMain()
{
}

bool CSceneMain::Release()
{
	CSceneState::Release();


	return true;
}


bool CSceneMain::Start(HINSTANCE hInstance, HWND hWnd, Microsoft::WRL::ComPtr<ID2D1Factory2> pd2dFactory2, ID2D1HwndRenderTarget *pd2dRenderTarget)
{
	CSceneState::Start(hInstance, hWnd, pd2dFactory2, pd2dRenderTarget);

	player.set(100, 100, 25, 25);
	player.set_speed(500);

	CIOCPClient::GetInstance()->SetPlayer(&player);

	return true;
}

void CSceneMain::Render(ID2D1HwndRenderTarget *pd2dRenderTarget)
{
	player.render(pd2dRenderTarget);

}
void CSceneMain::Update(const float& fTime)
{
	player.update(fTime);
}

void CSceneMain::LateUpdate(const float& fTime)
{
}


void CSceneMain::FixedUpdate(const float& fTime)
{
}