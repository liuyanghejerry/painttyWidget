package platform

import "runtime"

func GetPlatformName() string {
    if runtime.GOOS == "darwin" {
        return "mac"
    }
    return runtime.GOOS
}

func GetPlatformArch() (ret string) {
    switch runtime.GOARCH {
    case "amd64":
        ret =  "x64"
    case "386":
        ret = "x86"
    default:
        ret = "x86"
    }
    return ret
}