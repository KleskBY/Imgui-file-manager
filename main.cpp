#ifndef _DEBUG
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif // !_DEBUG
#pragma comment(lib, "d3dx9.lib")
#define DIRECTINPUT_VERSION 0x0800
#include "main.h"
#include "c://Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include/d3dx9tex.h"
#include "img/trash.h"
#include "img/copy.h"
#include "img/imageicon.h"
#include "img/fileicon.h"
#include "img/foldericon.h"
#include <iostream>
#include <fstream>
#include <vector>

struct File
{
    std::string path;
    std::string name;
    std::string size;
    std::string type;
    std::string dateline;
};
std::vector<File> ScannedFiles;
std::vector<std::string> Disks;
bool FileSelected[255];
char ExplorerPath[MAX_PATH] = "C:\\";

static std::string ConvertSize(DWORD size)
{
    if (size == 4294967295) return ("FOLDER");
    std::string format = std::to_string(size) + (" B");
    if (size / 1024 > 1)
    {
        format = std::to_string(size / 1024) + (" KB");
        if (size / 1024 / 1024 > 1)
        {
            format = std::to_string(size / 1024 / 1024) + (" MB");
            if (size / 1024 / 1024 / 1024 > 1)
            {
                format = std::to_string(size / 1024 / 1024 / 1024) + (" GB");
            }
        }
    }
    return format;
}
static time_t GetEditDate(const char* path)
{
    struct stat result;
    if (stat(path, &result) == 0)
    {
        time_t time = result.st_mtime;
        return time;
    }
    return NULL;
}

std::string TimeStampToDateTime(const time_t rawtime)
{
    char buff[20];
    strftime(buff, 20, ("%Y-%m-%d %H:%M:%S"), localtime(&rawtime));
    return std::string(buff);
}
static bool IsFolder(const char* path)
{
    DWORD dwAttrib = GetFileAttributes(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

static std::vector<std::string> GetAllDisks()
{
    std::vector<std::string> ret;
    char buff[100] = "";
    if (GetLogicalDriveStrings(100, buff) != 0)
    {
        ret.push_back(&buff[0]);
        ret.push_back(&buff[4]);
        ret.push_back(&buff[8]);
        ret.push_back(&buff[12]);
        ret.push_back(&buff[16]);
        ret.push_back(&buff[20]);
    }
    return ret;
}

void ScanDir(std::string filePath, std::string extension, std::vector<File>& ret)
{
    ScannedFiles.clear();
    if (strlen(ExplorerPath) > 3)
    {
        File file;
        file.path = "GO UP";
        file.name = "GO UP";
        file.dateline = "";
        file.type = "FOLDER";
        ret.push_back(file);
    }
    WIN32_FIND_DATA fileInfo;
    HANDLE hFind;
    std::string  fullPath = filePath + extension;
    hFind = FindFirstFile(fullPath.c_str(), &fileInfo);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        File file;
        file.path = filePath + fileInfo.cFileName;
        file.name = file.path.substr(file.path.find_last_of("\\")+1, file.path.size());
        file.size = ConvertSize(fileInfo.nFileSizeLow);
        file.dateline = TimeStampToDateTime(GetEditDate(file.path.c_str()));
        file.type = "FILE";
        if (IsFolder(file.path.c_str()))
        {
            file.type = "FOLDER";
            file.size = "FOLDER";
        }
        if(file.name != "." && file.name != "..") ret.push_back(file);
        while (FindNextFile(hFind, &fileInfo) != 0)
        {
            file.path = filePath + fileInfo.cFileName;
            file.name = file.path.substr(file.path.find_last_of("\\") + 1, file.path.size());
            file.size = ConvertSize(fileInfo.nFileSizeLow);
            file.dateline = TimeStampToDateTime(GetEditDate(file.path.c_str()));
            file.type = "FILE";
            if (IsFolder(file.path.c_str()))
            {
                file.type = "FOLDER";
                file.size = "FOLDER";
            }
            if (file.name != "." && file.name != "..") ret.push_back(file);
        }
    }
}













int main(int, char* argv[])
{
    if (FindWindow(AppClass, NULL)) return EXIT_SUCCESS;
    Disks = GetAllDisks();
    ScanDir(ExplorerPath, "*.*", ScannedFiles);

    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, AppClass, NULL };
    RegisterClassEx(&wc);
    hwnd = CreateWindowExA(WS_EX_APPWINDOW, AppClass, AppName, WS_POPUP, (desktop.right / 2) - (WindowWidth / 2), (desktop.bottom / 2) - (WindowHeight / 2), WindowWidth, WindowHeight, 0, 0, wc.hInstance, 0);
    NOTIFYICONDATA nid;
    if (CreateDeviceD3D(hwnd) < 0)
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    DefaultFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 20.0f, NULL, io.Fonts->GetGlyphRangesCyrillic());
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &trash, sizeof(trash), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tTrash);
    D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &copyicon, sizeof(copyicon), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tCopy);
    D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &fileicon, sizeof(fileicon), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tFile);
    D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &imageicon, sizeof(imageicon), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tImage);
    D3DXCreateTextureFromFileInMemoryEx(g_pd3dDevice, &foldericon, sizeof(foldericon), 32, 32, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &tFolder);

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    static bool open = true;
    DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        if (!open) ExitProcess(EXIT_SUCCESS);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        if (ShowMenu)
        {
            ImGui_ImplDX9_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            ImGui::SetNextWindowSize(ImVec2(WindowWidth, WindowHeight), ImGuiCond_Once);
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
            ImGui::Begin(AppName, &open, dwFlag);
            {
                static const char* current_item = Disks[0].c_str();
                ImGui::PushItemWidth(100.f);
                if (ImGui::BeginCombo("Disk", current_item)) // The second parameter is the label previewed before opening the combo.
                {
                    for (int n = 0; n < 2; n++)
                    {
                        bool is_selected = (current_item == Disks[n].c_str()); // You can store your selection however you want, outside or inside your objects
                        if (ImGui::Selectable(Disks[n].c_str(), is_selected)) current_item = Disks[n].c_str();
                        if (is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                    if (current_item == Disks[0].c_str()) { ExplorerPath[0] = Disks[0][0]; ExplorerPath[1] = ':'; ExplorerPath[2] = '\\'; ExplorerPath[3] = '\0';  ScanDir(ExplorerPath, "*.*", ScannedFiles); }
                    if (current_item == Disks[1].c_str()) { ExplorerPath[0] = Disks[1][0]; ExplorerPath[1] = ':'; ExplorerPath[2] = '\\'; ExplorerPath[3] = '\0';  ScanDir(ExplorerPath, "*.*", ScannedFiles); }
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                if (ImGui::InputText("Path", ExplorerPath, MAX_PATH, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    ScanDir(ExplorerPath, "*.*", ScannedFiles);
                }

                ImGui::BeginChild("##Files", ImVec2(590,300));
                ImGui::Columns(6, "FileInfo");
                ImGui::SetColumnWidth(0, 25);
                ImGui::SetColumnWidth(1, 200);
                ImGui::SetColumnWidth(2, 80);
                ImGui::SetColumnWidth(3, 80);
                ImGui::SetColumnWidth(4, 145);
                ImGui::NextColumn();
                ImGui::Text("Name");
                ImGui::NextColumn();
                ImGui::Text("Size");
                ImGui::NextColumn();
                ImGui::Text("Type");
                ImGui::NextColumn();
                ImGui::Text("Last edit");
                ImGui::NextColumn();
                ImGui::NextColumn();
                ImGui::Separator();
                for (int i = 0; i < ScannedFiles.size(); i++)
                {
                    if(ScannedFiles[i].type == "FOLDER") ImGui::Image(tFolder, ImVec2(16, 16));
                    else if (ScannedFiles[i].type == "IMAGE") ImGui::Image(tImage, ImVec2(16, 16));
                    else   ImGui::Image(tFile, ImVec2(16, 16));
                    
                    ImGui::NextColumn();
                    for (int i = 0; i < 255; i++) FileSelected[i] = false;
                    ImGui::Selectable(ScannedFiles[i].name.c_str(), &FileSelected[i]);
                    for (int i = 0; i < 255; i++)
                    {
                        if (FileSelected[i])
                        {
                            if (ScannedFiles[i].type == "FOLDER")
                            {
                                if (ScannedFiles[i].path == "GO UP")
                                {
                                    std::string newpath = ExplorerPath;
                                    newpath = newpath.substr(0, newpath.size() - 1);
                                    newpath = newpath.substr(0, newpath.find_last_of("\\"));
                                    for (int j = 0; j <= newpath.size(); j++) ExplorerPath[j] = newpath[j];
                                    ExplorerPath[newpath.size()] = '\\';
                                    ExplorerPath[newpath.size() + 1] = '\0';
                                    ScanDir(ExplorerPath, "*.*", ScannedFiles);
                                }
                                else
                                {
                                    for (int j = 0; j <= ScannedFiles[i].path.size(); j++) ExplorerPath[j] = ScannedFiles[i].path[j];
                                    ExplorerPath[ScannedFiles[i].path.size()] = '\\';
                                    ExplorerPath[ScannedFiles[i].path.size() + 1] = '\0';
                                    ScanDir(ExplorerPath, "*.*", ScannedFiles);
                                }
                            }
                        }
                    }
                    ImGui::NextColumn();
                    ImGui::Text(ScannedFiles[i].size.c_str());
                    ImGui::NextColumn();
                    ImGui::Text(ScannedFiles[i].type.c_str());
                    ImGui::NextColumn();
                    ImGui::Text(ScannedFiles[i].dateline.c_str());
                    ImGui::NextColumn();
                    ImGui::PushID(i);
                    if(ImGui::ImageButton(tTrash, ImVec2(16, 16)))
                    {
                        std::remove(ScannedFiles[i].path.c_str());
                        ScanDir(ExplorerPath, "*.*", ScannedFiles);
                    }
                    ImGui::PopID();
                    ImGui::NextColumn();
                }
                ImGui::Columns();
                ImGui::EndChild();

                if (tScreenShot != nullptr) ImGui::Image(tScreenShot, ImVec2(300, 200));
            }
            ImGui::End();

            ImGui::EndFrame();

            g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);
            if (g_pd3dDevice->BeginScene() >= 0)
            {
                ImGui::Render();
                ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
                g_pd3dDevice->EndScene();
            }
            HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
            if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) ResetDevice();
        }
        else std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
