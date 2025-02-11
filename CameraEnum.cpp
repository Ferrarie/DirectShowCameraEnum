#include "CameraEnum.h"
#include <comdef.h>

namespace CameraEnum {

    static std::wstring g_lastError;

    // 设置错误信息
    void SetError(const wchar_t* error) {
        g_lastError = error;
    }

    // 获取最后的错误信息
    const wchar_t* GetLastErrorMessage() {
        return g_lastError.c_str();
    }

    // 检查COM初始化状态
    bool IsComInitialized() {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr)) {
            CoUninitialize();
            return false;
        }
        return (hr == RPC_E_CHANGED_MODE || hr == S_FALSE);
    }

    // 主要的枚举函数实现
    int EnumCameras(CameraDevice* devices, int maxCount) {
        if (!devices || maxCount <= 0) {
            SetError(L"Invalid parameters");
            return -1;
        }

        // 检查COM初始化状态
        bool needCoUninitialize = false;
        if (!IsComInitialized()) {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            if (FAILED(hr)) {
                SetError(L"Failed to initialize COM");
                return -1;
            }
            needCoUninitialize = true;
        }

        // 创建系统设备枚举器
        ICreateDevEnum* pDevEnum = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void**)&pDevEnum);
        if (FAILED(hr)) {
            SetError(L"Failed to create system device enumerator");
            if (needCoUninitialize) CoUninitialize();
            return -1;
        }

        // 创建视频捕获设备的枚举器
        IEnumMoniker* pEnum = NULL;
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
        if (hr != S_OK) {
            SetError(L"No video capture devices found");
            pDevEnum->Release();
            if (needCoUninitialize) CoUninitialize();
            return 0;
        }

        // 枚举所有设备
        IMoniker* pMoniker = NULL;
        int deviceCount = 0;
        ULONG fetched;

        while (pEnum->Next(1, &pMoniker, &fetched) == S_OK && deviceCount < maxCount) {
            IPropertyBag* pPropBag = NULL;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

            if (SUCCEEDED(hr)) {
                // 获取设备名称
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);

                if (SUCCEEDED(hr)) {
                    // 获取设备路径作为ID
                    LPOLESTR displayName = NULL;
                    hr = pMoniker->GetDisplayName(NULL, NULL, &displayName);

                    if (SUCCEEDED(hr)) {
                        // 填充设备信息
                        devices[deviceCount].index = deviceCount;
                        wcsncpy_s(devices[deviceCount].name, varName.bstrVal, 255);
                        wcsncpy_s(devices[deviceCount].id, displayName, 255);

                        CoTaskMemFree(displayName);
                        deviceCount++;
                    }
                }

                VariantClear(&varName);
                pPropBag->Release();
            }

            pMoniker->Release();
        }

        // 清理
        pEnum->Release();
        pDevEnum->Release();
        if (needCoUninitialize) CoUninitialize();

        return deviceCount;
    }

} 