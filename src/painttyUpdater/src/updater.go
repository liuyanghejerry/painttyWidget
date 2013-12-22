package main

import (
    "log"
    "archive/zip"
    "bytes"
    "strings"
    "strconv"
    "encoding/json"
    "io"
    "io/ioutil"
    "net/http"
    "os"
    "flag"
    "path/filepath"
    "osext"
    "easycp"
    "platform"
    "language"
)

const SERVER_ADDR_IPV4 = "http://localhost:17979"
const SERVER_ADDR_IPV6 = "http://localhost:17979"
const DEFAULT_NEW_PACKAGE = "http://download.mrspaint.com/0.4/%E8%8C%B6%E7%BB%98%E5%90%9B_Alpha_x86.zip"
const UPDATER_VER uint64 = 20

type VersionCheckReq struct {
    Request  string `json:"request"`
    Platform string `json:"platform"`
    Language string `json:"language"`
}

type VersionInfoRes struct {
    Version   uint64  `json:"version"`
    Level     uint64  `json:"level"`
    Changelog string `json:"changelog"`
    Url       string `json:"url,omitempty"`
}

type UpdaterRes struct {
    Version uint64 `json:"version"`
}

type VersionCheckRes struct {
    Response string         `json:"response"`
    Result   bool           `json:"result"`
    Info     VersionInfoRes `json:"info,omitempty"`
    Updater  UpdaterRes     `json:"updater,omitempty"`
}

type CopyFailedError struct {
    message string
}

func (self CopyFailedError) Error() string {
    return self.message
}

func Unzip(src, dest string) error {
    r, err := zip.OpenReader(src)
    if err != nil {
        return err
    }
    defer r.Close()

    for _, f := range r.File {
        rc, err := f.Open()
        if err != nil {
            return err
        }
        defer rc.Close()

        path := filepath.Join(dest, f.Name)
        if f.FileInfo().IsDir() {
            os.MkdirAll(path, f.Mode())
        } else {
            f, err := os.OpenFile(
                path, os.O_WRONLY|os.O_CREATE|os.O_TRUNC, f.Mode())
            if err != nil {
                return err
            }
            defer f.Close()

            _, err = io.Copy(f, rc)
            if err != nil {
                return err
            }
        }
    }

    return nil
}

func CopyDir(src string, dest string) bool {
    return easycp.CopyDir(src, dest)
}

func oldVersion() uint64 {
    v := flag.Uint64("v", 40, "current version, unsigned integer")
    flag.Parse()
    return *v
}

func queryPlatform() string {
    p := platform.GetPlatformName()
    a := platform.GetPlatformArch()
    if p == "mac" {
        return p
    }
    return p + " " + a
}

func queryLanguage() string {
    return language.GetSystemLanguage()
}

func complete() error {
    r := flag.String("i", "", "path to original updater")
    p := flag.Uint64("p", 0, "pid to original updater")
    flag.Parse()

    if *p <= 0 {
        return nil
    }

    old_proc, err := os.FindProcess(int(*p))

    if err != nil {
        return err
    }

    err = old_proc.Kill()

    if err != nil {
        return err
    }
    

    if len(*r) <= 0 {
        return nil
    }

    info, err := os.Stat(*r)
    if err != nil || !info.IsDir() {
        return err
    }

    current_path, err := osext.ExecutableFolder()

    if err != nil {
        return err
    }

    info, err = os.Stat(current_path)

    if err != nil || !info.IsDir() {
        return err
    }

    result := CopyDir(current_path, *r)

    if !result {
        return CopyFailedError {"Cannot copy files from " + current_path + " to " + *r}
    }

    return nil
}

func check() (bool, string, error) {
    m := VersionCheckReq{"version", queryPlatform(), queryLanguage()}
    jsonRequest, err := json.Marshal(m)
    if err != nil {
        return false, "", err
    }

    var jsonPkg bytes.Buffer
    json.Compact(&jsonPkg, jsonRequest)
    resp, err := http.Post(SERVER_ADDR_IPV4, "application/json", &jsonPkg)
    if err != nil {
        log.Panicln("error when post")
        return false, "", err
    }

    respBody, err := ioutil.ReadAll(resp.Body)
    resp.Body.Close()
    if err != nil {
        log.Panicln("error while read response")
        return false, "", err
    }

    var version_info VersionCheckRes
    err = json.Unmarshal(respBody, &version_info)
    if err != nil {
        log.Panicln("error when unmarshal")
        return false, "", err
    }
    log.Println(version_info.Info)

    if !version_info.Result {
        log.Println("server rejected request")
        return false, "", nil
    }

    if version_info.Info.Version > oldVersion() {
        if len(strings.TrimSpace(version_info.Info.Url)) <= 0 {
            return true, DEFAULT_NEW_PACKAGE, nil
        }
        
        return true, version_info.Info.Url, nil
    }

    return false, "", nil
}

func download(address string) (bool, *os.File, error) {
    res, err := http.Get(address)
    if err != nil {
        log.Panicln("failed to download new package")
        return false, nil, err
    }

    data, err := ioutil.ReadAll(res.Body)
    res.Body.Close()
    if err != nil {
        log.Panicln("failed to download new package")
        return false, nil, err
    }
    f, err := ioutil.TempFile("", "paintty_update_")
    if err != nil {
        log.Panicln("failed to write new package to disk")
        return false, nil, err
    }

    f.Write(data)

    return true, f, nil
}

func uncompress(file *os.File) (bool, string, error) {
    dir_name := file.Name() + "_dir"
    err := Unzip(file.Name(), dir_name)
    if err != nil {
        return false, "", err
    }

    return true, dir_name, nil
}

func install(dest string) error {
    current_path, err := osext.ExecutableFolder()
    if err != nil {
        return err
    }
    s := string(os.PathSeparator)
    new_updater := filepath.FromSlash(current_path + s +"updater")
    pid := os.Getpid()
    args := []string {"-i " + current_path + " ", "-p " + strconv.Itoa(pid) + " "}
    prcs, err := os.StartProcess(new_updater, args, nil)
    if err != nil {
        return err
    }
    // if everything goes smooth, updater will be killed before Wait() returns
    _, err = prcs.Wait()
    return err
}

func start(src string) bool {
    s := string(os.PathSeparator)
    _, err := os.StartProcess(src + s + "MrPaint", []string{}, nil)
    if err != nil {
        return false
    }

    return true
}

func main() {
    _ = complete()
    next, addr, err := check()
    if err != nil {
        log.Panicln("check failed")
        log.Panicln(err)
        return
    }

    if !next {
        return
    }

    next, f, err := download(addr)

    if f != nil {
        defer f.Close();
        defer os.Remove(f.Name())
    }

    if err != nil {
        log.Panicln("download failed")
        log.Panicln(err)
        return
    }

    if !next {
        return
    }

    log.Println(f.Name())

    next, file_path, err := uncompress(f)
    log.Println(file_path)

    if err != nil {
        log.Panicln("uncompress failed")
        log.Panicln(err)
        return
    }

    if !next {
        return
    }

    err = install(file_path)

    if err != nil {
        log.Panicln("install failed")
        log.Panicln(err)
        return
    }

    b := start(file_path)

    if !b {
        log.Panicln("start new MrPaint failed")
        return
    }
}
