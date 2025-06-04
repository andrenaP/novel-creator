#ifndef FILE_UTILS_HPP
#define FILE_UTILS_HPP
#include <cstring>
#include <string>
#include <filesystem>
#include <cstdio>

inline std::string OpenFileDialog() {
    std::string filename;
#ifdef _WIN32
    // Windows: Use PowerShell with System.Windows.Forms
    // const char* cmd = "powershell -Command \""
    //                   "Add-Type -AssemblyName System.Windows.Forms; "
    //                   "$dlg = New-Object System.Windows.Forms.OpenFileDialog; "
    //                   "$dlg.Title = 'Select a File'; "
    //                   "$dlg.Filter = 'All Files (*.*)|*.*|Text Files (*.txt)|*.txt'; "
    //                   "$dlg.InitialDirectory = [Environment]::GetFolderPath('Desktop'); "
    //                   "if ($dlg.ShowDialog() -eq 'OK') { Write-Output $dlg.FileName } else { Write-Output '' }\"";
const char* cmd = "powershell -Command \""
                  "Add-Type -AssemblyName System.Windows.Forms; "
                  "$dlg = New-Object System.Windows.Forms.OpenFileDialog; "
                  "$dlg.Title = 'Select an Image'; "
                  "$dlg.Filter = 'Image Files (*.png;*.jpg;*.jpeg)|*.png;*.jpg;*.jpeg|All Files (*.*)|*.*'; "
                  "$dlg.InitialDirectory = [Environment]::GetFolderPath('Desktop'); "
                  "if ($dlg.ShowDialog() -eq 'OK') { Write-Output $dlg.FileName } else { Write-Output '' }\"";

    char buffer[1024];
    char result[1024] = {0};
    FILE* pipe = _popen(cmd, "r"); // Use popen for Windows (or _popen for MSVC)
    if (!pipe) {
        printf("Failed to run command\n");
        return "";
    }

    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        strncat(result, buffer, sizeof(result) - strlen(result) - 1);
    };
    _pclose(pipe);
    // Remove trailing newline if present
    size_t len = strlen(result);
    if (len > 0 && result[len - 1] == '\n') {
        result[len - 1] = '\0';
    }
    // printf("Selected file: %s\n", result);
    filename=result;
#else
    // Linux: Use zenity
    char buffer[1024] = "";
    FILE* f = popen("zenity --file-selection", "r");
    if (f) {
        if (fgets(buffer, sizeof(buffer), f) != nullptr) {
            filename = buffer;
            // Remove trailing newline if present
            if (!filename.empty() && filename.back() == '\n') {
                filename.pop_back();
            }
        }
        pclose(f);
    }
#endif
    return filename;
}

inline bool IsValidImagePath(const std::string& path) {
    if (path.empty()) return false;
    std::filesystem::path fsPath(path);
    return std::filesystem::exists(fsPath) &&
           (fsPath.extension() == ".png" || fsPath.extension() == ".jpg" || fsPath.extension() == ".jpeg");
}

#endif // FILE_UTILS_HPP
