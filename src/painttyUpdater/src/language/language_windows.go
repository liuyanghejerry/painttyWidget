// +build windows

package language

import "syscall"

// LID can be found at http://support.microsoft.com/kb/193080

func GetSystemLanguage() string {
    dll := syscall.NewLazyDLL("kernel32.dll")
    GetUserDefaultLCID := dll.NewProc("GetUserDefaultLCID")

    ret, _, _ := GetUserDefaultLCID.Call()
    LID := uint32(ret)
    switch LID {
    case 2052:
        return "zh_CN"
    case 1028:
        return "zh_TW"
    case 3076:
        return "zh_HK"
    case 4100:
        return "zh_CN"
    case 1041:
        return "ja"
    default:
        return "en_US"
    }
}