// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.

#pragma comment(lib, "D2D1.lib")
#pragma comment(lib, "Dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")

// Windows 헤더 파일:
#include <windows.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <memory>
#include <tchar.h>


//C++
#include <iostream>
#include <chrono>
#include <algorithm>
#include <memory>	

//STL Container
#include <string>
#include <vector>
#include <map>
#include <array>

// D2D1

#include <d2d1.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwrite_2.h>
#include <d2d1_2helper.h>
#include <wincodec.h>

// DirectX Math
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>

// U W P
#include <wrl.h>
#include <wrl/client.h>
#include <wrl/internal.h>

// Texture Loader : in Utils

using namespace D2D1;
using namespace DirectX;
using namespace DirectX::PackedVector;


#include "MyDefine.h"
#include "MyEnum.h"
#include "MyStruct.h"

#include "Object_D2D.h"
#include "SingleTon.h"
#include "SceneState.h"
#include "InputManager.h"
#include "DropManager.h"
#include "DrawHelper.h"

#include "Timer.h"
#include "Util.h"
#include "Draw.h"
#include "GraphicObject_2D.h"
#include "SpriteObject_2D.h"

#include "RenderManager_2D.h"
#include "Component.h"
#include "GameObject_2D.h"
#include "Camera_2D.h"

#include "Player.h"

#include "../IOCPServer_2017//protocol.h"
#include "IOCPClient.h"


static HWND gHwnd;

#ifdef UNICODE
	#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#else 
	#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
