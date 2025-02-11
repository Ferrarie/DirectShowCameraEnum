// include/CameraEnum.h
#pragma once

#include <windows.h>
#include <dshow.h>
#include <vector>
#include <string>
#include <objbase.h>

#pragma comment(lib, "strmiids")

namespace CameraEnum {

    // ���嵼���ṹ��,��Unity��CamDevice��Ӧ
    struct CameraDevice {
        int index;
        wchar_t id[256];  // ʹ��wchar_t֧��Unicode
        wchar_t name[256];
    };

    // ������������
    extern "C" {
        __declspec(dllexport) int EnumCameras(CameraDevice* devices, int maxCount);
        __declspec(dllexport) const wchar_t* GetLastErrorMessage();
    }

} // namespace CameraEnum