// stdafx.h : 자주 사용하지만 자주 변경되지는 않는
// 표준 시스템 포함 파일 또는 프로젝트 관련 포함 파일이
// 들어 있는 포함 파일입니다.
//

#pragma once

#include "targetver.h"

#include <WinSock2.h>	//순서 중요 winwod 헤더 포함
#include <winsock.h>

#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <map>
#include <set>
#include <cstdio>
#include <unordered_set>
#include <string>

#include <mutex>
#include <atomic>

#include "Object.h"
#include "Util.h"
#include "protocol.h"
#include "MyDefine.h"
#include "MyStruct.h"

#pragma comment (lib, "ws2_32.lib")

// TODO: 프로그램에 필요한 추가 헤더는 여기에서 참조합니다.
