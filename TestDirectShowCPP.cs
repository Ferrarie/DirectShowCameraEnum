using System;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using System.Runtime.InteropServices;
using Sirenix.OdinInspector;


public class TestDirectShowCPP : MonoBehaviour
{

    [SerializeField]
    private List<CamDevice> _devices = new List<CamDevice>();

    [Button]
    private void Init()
    {
        _devices = CameraEnumPlugin.GetCameras().ToList();

        foreach (var dev in _devices)
        {
            Debug.Log(dev);
        }
    }
}


[System.Serializable]
public class CamDevice
{
    public int Index;
    public string ID;
    public string Name;

    public override string ToString()
    {
        return $"[idx:{Index},name:{Name},id:{ID}]";
    }
}

public class CameraEnumPlugin
{
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct NativeCameraDevice
    {
        public int index;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string id;

        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string name;
    }

    [DllImport("CameraEnum", CallingConvention = CallingConvention.Cdecl)]
    private static extern int EnumCameras([Out] NativeCameraDevice[] devices, int maxCount);

    [DllImport("CameraEnum", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr GetLastErrorMessage();

    public static CamDevice[] GetCameras()
    {
        NativeCameraDevice[] devices = new NativeCameraDevice[10]; // 假设最多10个摄像头
        int count = EnumCameras(devices, devices.Length);

        if (count < 0)
        {
            IntPtr errorPtr = GetLastErrorMessage();
            string error = Marshal.PtrToStringUni(errorPtr);
            throw new System.Exception($"Failed to enumerate cameras: {error}");
        }

        // 转换成Unity需要的格式
        CamDevice[] result = new CamDevice[count];
        for (int i = 0; i < count; i++)
        {
            result[i] = new CamDevice
            {
                Index = devices[i].index,
                ID = devices[i].id,
                Name = devices[i].name
            };
        }

        return result;
    }
}
