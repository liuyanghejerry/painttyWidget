// +build !windows

package language

import "os/exec"
import "regexp"
import "bytes"

var re *regexp.Regexp

func init() {
    re = regexp.MustCompile("LANG=([a-zA-Z_]{1,})")
}

func GetSystemLanguage() string {
    out, err := exec.Command("locale").Output()
    if err != nil {
        return "en_US"
    }
    buf := bytes.NewBuffer(out)
    line, err := buf.ReadString('\n')

    if err != nil {
        return "en_US"
    }

    results := re.FindStringSubmatch(line)

    if len(results) < 2 {
        return "en_US"
    }

    return results[1]
}