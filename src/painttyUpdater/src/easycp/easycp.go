package easycp

import (
    "io"
    "os"
    "io/ioutil"
)

func isDirExists(path string) bool {
    fi, err := os.Stat(path) 
    if err != nil {         
        return false
    } else {         
        return fi.IsDir()
    }
}

func CopyFile(src, dest string) bool {
    srcFile, err := os.Open(src)
    if err != nil {
        return false
    }
    info, err := os.Stat(src)
    if err != nil {
        return false
    }

    defer srcFile.Close()

    desFile, err := os.Create(dest)     
    if os.IsExist(err) {
        desFile, err = os.OpenFile(dest, os.O_WRONLY, info.Mode())
        if err != nil {
            return false
        }
    }
    defer desFile.Close()
    _, err = io.Copy(desFile, srcFile)
    if err != nil {
        return false
    }
    return true
}

func CopyDir(srcDir, destDir string) bool {    
    b := isDirExists(srcDir)
    info, err := os.Stat(srcDir)
    if !b || err != nil {
        return false
    }

    b = isDirExists(destDir)

    if !b {
        err = os.MkdirAll(destDir, info.Mode())
        if err != nil {
            return false
        }
    }

    b = true

    files, _ := ioutil.ReadDir(srcDir)
    s := string(os.PathSeparator)
    for _, f := range files {
        if f.IsDir() {
            b  = b && CopyDir(srcDir + s + f.Name(), destDir + s + f.Name())
        } else {
            b  = b && CopyFile(srcDir + s + f.Name(), destDir + s + f.Name())
        }
    }
    return b
}
