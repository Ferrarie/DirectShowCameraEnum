#include "CameraEnum.h"
#include <comdef.h>

namespace CameraEnum {

    static std::wstring g_lastError;

    // ���ô�����Ϣ
    void SetError(const wchar_t* error) {
        g_lastError = error;
    }

    // ��ȡ���Ĵ�����Ϣ
    const wchar_t* GetLastErrorMessage() {
        return g_lastError.c_str();
    }

    // ���COM��ʼ��״̬
    bool IsComInitialized() {
        HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr)) {
            CoUninitialize();
            return false;
        }
        return (hr == RPC_E_CHANGED_MODE || hr == S_FALSE);
    }

    // ��Ҫ��ö�ٺ���ʵ��
    int EnumCameras(CameraDevice* devices, int maxCount) {
        if (!devices || maxCount <= 0) {
            SetError(L"Invalid parameters");
            return -1;
        }

        // ���COM��ʼ��״̬
        bool needCoUninitialize = false;
        if (!IsComInitialized()) {
            HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
            if (FAILED(hr)) {
                SetError(L"Failed to initialize COM");
                return -1;
            }
            needCoUninitialize = true;
        }

        // ����ϵͳ�豸ö����
        ICreateDevEnum* pDevEnum = NULL;
        HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
            IID_ICreateDevEnum, (void**)&pDevEnum);
        if (FAILED(hr)) {
            SetError(L"Failed to create system device enumerator");
            if (needCoUninitialize) CoUninitialize();
            return -1;
        }

        // ������Ƶ�����豸��ö����
        IEnumMoniker* pEnum = NULL;
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
        if (hr != S_OK) {
            SetError(L"No video capture devices found");
            pDevEnum->Release();
            if (needCoUninitialize) CoUninitialize();
            return 0;
        }

        // ö�������豸
        IMoniker* pMoniker = NULL;
        int deviceCount = 0;
        ULONG fetched;

        while (pEnum->Next(1, &pMoniker, &fetched) == S_OK && deviceCount < maxCount) {
            IPropertyBag* pPropBag = NULL;
            hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&pPropBag);

            if (SUCCEEDED(hr)) {
                // ��ȡ�豸����
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"FriendlyName", &varName, 0);

                if (SUCCEEDED(hr)) {
                    // ��ȡ�豸·����ΪID
                    LPOLESTR displayName = NULL;
                    hr = pMoniker->GetDisplayName(NULL, NULL, &displayName);

                    if (SUCCEEDED(hr)) {
                        // ����豸��Ϣ
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

        // ����
        pEnum->Release();
        pDevEnum->Release();
        if (needCoUninitialize) CoUninitialize();

        return deviceCount;
    }

} 