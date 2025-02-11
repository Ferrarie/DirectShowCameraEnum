// include/CameraEnum.h
#pragma once

#include <windows.h>
#include <dshow.h>
#include <vector>
#include <string>
#include <objbase.h>

#pragma comment(lib, "strmiids")

namespace CameraEnum {

    // 定义导出结构体,与Unity的CamDevice对应
    struct CameraDevice {
        int index;
        wchar_t id[256];  // 使用wchar_t支持Unicode
        wchar_t name[256];
    };

    // 导出函数声明
    extern "C" {
        __declspec(dllexport) int EnumCameras(CameraDevice* devices, int maxCount);
        __declspec(dllexport) const wchar_t* GetLastErrorMessage();
    }

} // namespace CameraEnum