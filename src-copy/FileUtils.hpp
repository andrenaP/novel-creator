#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP

#include <string>
#include <filesystem>
#include <cstdio>

std::string OpenFileDialog() {
    char filename[1024] = "";
    FILE* f = popen("zenity --file-selection", "r");
    if (f) {
        fgets(filename, 1024, f);
        pclose(f);
        size_t len = strlen(filename);
        if (len > 0 && filename[len - 1] == '\n') filename[len - 1] = '\0';
    }
    return std::string(filename);
}

bool IsValidImagePath(const std::string& path) {
    if (path.empty()) return false;
    std::filesystem::path fsPath(path);
    return std::filesystem::exists(fsPath) &&
           (fsPath.extension() == ".png" || fsPath.extension() == ".jpg" || fsPath.extension() == ".jpeg");
}

#endif // FILE_UTILS_HPP
