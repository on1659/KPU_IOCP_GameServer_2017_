#include "stdafx.h"
#include "MapObject.h"


CMapObject::CMapObject(const std::string name) : CGraphicObject_2D(name)
{
}


CMapObject::~CMapObject()
{
}

void CMapObject::Render(ID2D1HwndRenderTarget * pd2dRenderTarget, CCamera_2D * pCamera)
{

	if (!pCamera->isVisible(GetRect()))return;
	Render(pd2dRenderTarget, pCamera->GetCameView());
}

void CMapObject::Render(ID2D1HwndRenderTarget * pd2dRenderTarget, const D2D_RECT_F rtCamView)
{
	D2D_RECT_F rtPos{ 0.0f, 0.0f, WIDTH, HEIGHT };
	D2D_RECT_F rtSrc{ 0.0f, 0.0f, 0.0f, 0.0f };

	rtSrc.left	 = rtCamView.left;
	rtSrc.top	 = rtCamView.top;
	
	rtSrc.right  = rtCamView.right  + rtCamView.left;//min(rtSrc.left + (rtPos.right - rtPos.left), GetWidth());// (pCamera->GetWidth(), rtPos.left + GetWidth());
	rtSrc.bottom = rtCamView.bottom + rtCamView.top;//min(rtSrc.top + (rtPos.bottom - rtPos.top), GetHeight());//min(pCamera->GetHeight(), rtPos.top + GetHeight());

	RENDERMGR_2D->Render(pd2dRenderTarget, m_imageName, rtPos, m_alpha, rtSrc);

	Draw::drawText
	(
			pd2dRenderTarget
		, RectF(100, 100, 500, 130)
		, FONT_FORMAT::FONT_³ª´®¹Ù¸¥°íµñ
		, MY_COLOR(MyColorEnum::Red)
		, TEXT("(%1.f, %1.f, %1.f, %1.f) - w,h(%1.f, %1.f)")
		, rtSrc.left, rtSrc.top, rtSrc.right, rtSrc.bottom, rtCamView.right , rtCamView.bottom
	
	);

}
