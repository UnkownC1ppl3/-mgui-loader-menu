#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "imgui_freetype.h"

#include "imgui_edited.hpp"

#include <D3DX11tex.h>
#pragma comment(lib, "D3DX11.lib")

#include "image.h"
#include "font.h"

#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>

#include <vector>
#include <random>
#include <math.h>

std::vector<ImVec2> circles_pos;
std::vector<ImVec2> circles_dir;
std::vector<int> circles_radius;

void draw_circle(ImVec2 pos, int radius, ImU32 color)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    unsigned char* color_ptr = (unsigned char*)&color;
    draw_list->AddCircleFilled(pos, radius - 1, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 75));
}

void draw_line(ImVec2 pos1, ImVec2 pos2, ImU32 color, int radius)
{
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float distance = std::sqrt(
        std::pow(pos2.x - pos1.x, 2) + // std:: pow
        std::pow(pos2.y - pos1.y, 2)    // std:: pow
    );
    float alpha;
    if (distance <= 20.0f) {
        alpha = 255.0f;
    }
    else {
        alpha = (1.0f - ((distance - 20.0f) / 25.0f)) * 255.0f;
    }

    int r = (color & 0xFF0000) >> 16; // Extract red component
    int g = (color & 0x00FF00) >> 8;  // Extract green component
    int b = (color & 0x0000FF);       // Extract blue component

    unsigned char* color_ptr = (unsigned char*)&color;

    draw_list->AddLine(pos1, pos2, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], static_cast<int>(alpha)), 1.0f);
    if (distance >= 40.0f) {
        draw_list->AddCircleFilled(pos1, radius - 0.96f, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 30));
        draw_list->AddCircleFilled(pos2, radius - 0.96f, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 30));
    }
    else if (distance <= 20.0f) {
        draw_list->AddCircleFilled(pos1, radius, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 30));
        draw_list->AddCircleFilled(pos2, radius, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 30));
    }
    else {
        float radius_factor = 1.0f - ((distance - 20.0f) / 20.0f);
        float offset_factor = 1.0f - radius_factor;
        float offset = (radius - radius * radius_factor) * offset_factor;
        draw_list->AddCircleFilled(pos1, radius - offset, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 30));
        draw_list->AddCircleFilled(pos2, radius - offset, IM_COL32(color_ptr[0], color_ptr[1], color_ptr[2], 30));
    }
}

void move_circles()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 360);

    for (int i = 0; i < circles_pos.size(); i++)
    {
        ImVec2& pos = circles_pos[i];
        ImVec2& dir = circles_dir[i];
        int radius = circles_radius[i];

        pos.x += dir.x * 0.4;
        pos.y += dir.y * 0.4;

        if (pos.x - radius < 0 || pos.x + radius > ImGui::GetWindowWidth())
        {
            dir.x = -dir.x;
            dir.y = dis(gen) % 2 == 0 ? -1 : 1;
        }

        if (pos.y - radius < 0 || pos.y + radius > ImGui::GetWindowHeight())
        {
            dir.y = -dir.y;
            dir.x = dis(gen) % 2 == 0 ? -1 : 1;
        }
    }
}

void draw_circles_and_lines(ImU32 color)
{
    move_circles();

    for (int i = 0; i < circles_pos.size(); i++)
    {
        draw_circle(circles_pos[i], circles_radius[i], color);

        for (int j = i + 1; j < circles_pos.size(); j++)
        {
            float distance = ImGui::GetIO().FontGlobalScale * std::sqrt(std::pow(circles_pos[j].x - circles_pos[i].x, 2) + std::pow(circles_pos[j].y - circles_pos[i].y, 2) );

            if (distance <= 45.0f)
            {
                draw_line(circles_pos[i], circles_pos[j], color, circles_radius[i]);
            }
        }
    }
}

void setup_circles()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    std::uniform_int_distribution<> pos_dis(0, static_cast<int>(1980));
    std::uniform_int_distribution<> pos_dis_y(0, static_cast<int>(1080));

    for (int i = 0; i < 150; i++)
    {
        circles_pos.push_back(ImVec2(pos_dis(gen), pos_dis_y(gen)));
        circles_dir.push_back(ImVec2(dis(gen) == 0 ? -1 : 1, dis(gen) == 0 ? -1 : 1));
        circles_radius.push_back(3);
    }
}

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int tabs = 0;

int quest_count;
std::vector<std::string> quest_text;
char quest[250] = { "" };

float timer = 0;

using namespace ImGui;

namespace font
{
    inline ImFont* lexend_regular = nullptr;
    inline ImFont* lexend_medium = nullptr;
    inline ImFont* lexend_medium_x = nullptr;
    inline ImFont* lexend_bold = nullptr;

    inline ImFont* icomoon[19];

}

namespace texture
{
    ID3D11ShaderResourceView* background = nullptr;
    ID3D11ShaderResourceView* spoof_icon = nullptr;
    ID3D11ShaderResourceView* logo = nullptr;
}

void CricleProgress(const char* name, float progress, float max, float radius)
{
    static float tickness = 3.f;
    static float position = 0.f;
    static float alpha_text = 1.f;

    ImVec4 circle_loading = ImColor(0, 0, 0, 0);

    position = progress / max * 6.28f;

    ImVec2 vecCenter = { ImGui::GetContentRegionMax() / 2 };

    ImGui::GetForegroundDrawList()->PathClear();
    ImGui::GetForegroundDrawList()->PathArcTo(vecCenter, radius, 0.f, 2.f * IM_PI, 120.f);
    ImGui::GetForegroundDrawList()->PathStroke(ImGui::GetColorU32(circle_loading), 0, tickness);

    ImGui::GetForegroundDrawList()->PathClear();
    ImGui::GetForegroundDrawList()->PathArcTo(vecCenter, radius, IM_PI * 1.5f, IM_PI * 1.5f + position, 120.f);
    ImGui::GetForegroundDrawList()->PathStroke(ImGui::GetColorU32(c::accent), 0, tickness);

    int procent = progress / (int)max * 100;

    std::string procent_str = std::to_string(procent) + "%";

    ImGui::PushFont(font::lexend_regular);

    ImGui::GetForegroundDrawList()->AddText(vecCenter - ImGui::CalcTextSize(procent_str.c_str()) / 2, ImGui::GetColorU32(c::text::text_active), procent_str.c_str());

    alpha_text = ImLerp(alpha_text, procent > 80 ? 0.f : 1.f, ImGui::GetIO().DeltaTime * 6);

    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha_text);
    if (procent != 0) ImGui::GetForegroundDrawList()->AddText(ImVec2((ImGui::GetContentRegionMax().x - ImGui::CalcTextSize(name).x) / 2, ImGui::GetContentRegionMax().y - alpha_text * 60), ImGui::GetColorU32(c::red), name);
    ImGui::PopStyleVar();

    ImGui::PopFont();

}

HWND hwnd;
RECT rc;

void move_window() {

    ImGui::SetCursorPos(ImVec2(0, 0));
    if (ImGui::InvisibleButton("Move_detector", ImVec2(c::background::size)));
    if (ImGui::IsItemActive()) {

        GetWindowRect(hwnd, &rc);
        MoveWindow(hwnd, rc.left + ImGui::GetMouseDragDelta().x, rc.top + ImGui::GetMouseDragDelta().y, c::background::size.x, c::background::size.y, TRUE);
    }

}

int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{

    WNDCLASSEXW wc;
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = NULL;
    wc.cbWndExtra = NULL;
    wc.hInstance = nullptr;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = L"ImGui";
    wc.lpszClassName = L"Example";
    wc.hIconSm = LoadIcon(0, IDI_APPLICATION);

    RegisterClassExW(&wc);
    hwnd = CreateWindowExW(NULL, wc.lpszClassName, L"Example", WS_POPUP, (GetSystemMetrics(SM_CXSCREEN) / 2) - (c::background::size.x / 2), (GetSystemMetrics(SM_CYSCREEN) / 2) - (c::background::size.y / 2), c::background::size.x, c::background::size.y, 0, 0, 0, 0);

    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    

    ImFontConfig cfg;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;

    font::lexend_medium = io.Fonts->AddFontFromMemoryTTF(lexend_medium, sizeof(lexend_medium), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::lexend_regular = io.Fonts->AddFontFromMemoryTTF(lexend_regular, sizeof(lexend_regular), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
    font::lexend_medium_x = io.Fonts->AddFontFromMemoryTTF(lexend_medium, sizeof(lexend_medium), 13.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    for(int i = 14; i <= 40; i++) font::icomoon[i] = io.Fonts->AddFontFromMemoryTTF(icomoon, sizeof(icomoon), i, &cfg, io.Fonts->GetGlyphRangesCyrillic());

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    D3DX11_IMAGE_LOAD_INFO info; ID3DX11ThreadPump* pump{ nullptr };
    if (texture::background == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, background, sizeof(background), &info, pump, &texture::background, 0);
    if (texture::spoof_icon == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, spoof_icon, sizeof(spoof_icon), &info, pump, &texture::spoof_icon, 0);
    if (texture::logo == nullptr) D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), &info, pump, &texture::logo, 0);

    bool done = false;

    setup_circles();

    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done) break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        {

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImVec2(c::background::size));

            ImGuiStyle* style = &ImGui::GetStyle();

            style->WindowPadding = ImVec2(0, 0);
            style->ItemSpacing = ImVec2(0, 0);
            style->WindowBorderSize = 0;
            style->ScrollbarSize = 7.f;

            Begin("IMGUI", nullptr, ImGuiWindowFlags_NoDecoration);                          
            {

                const ImVec2& pos = ImGui::GetWindowPos();
                const ImVec2& region = ImGui::GetContentRegionMax();
                const ImVec2& spacing = style->ItemSpacing;

                GetBackgroundDrawList()->AddImage(texture::background, ImVec2(0, 0), ImVec2(c::background::size), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));

                static float tab_alpha = 0.f; /* */ static float tab_add; /* */ static int active_tab = 0;

                tab_alpha = ImClamp(tab_alpha + (4.f * ImGui::GetIO().DeltaTime * (tabs == active_tab ? 1.f : -1.f)), 0.f, 1.f);

                if (tab_alpha == 0.f && tab_add == 0.f)
                {
                    timer++;

                    CricleProgress("Hold on, verification is in progress ", timer, 300, 60);

                    if (timer > 300) {
                        active_tab = tabs;
                        timer = 0;
                    }
                }

                PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * style->Alpha);

                if (active_tab == 0)
                {

                    edited::BeginChild("Container##1", ImVec2(c::background::size.x / 2, c::background::size.y));
                    {
                        static int spoof_page = 0;

                        if (edited::Spoof_panel(texture::spoof_icon, 0 == spoof_page, "Easy Anti-Cheat", "Bypass everything", "[ V.1.0.0 ]", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 100), ImVec2(44, 44), 0)) spoof_page = 0;

                        if (edited::Spoof_panel(texture::spoof_icon, 1 == spoof_page, "Vanguard", "Bypass everything", "[ V.1.5.5 ]", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 100), ImVec2(44, 44), 1)) spoof_page = 1;

                        if (edited::Spoof_panel(texture::spoof_icon, 2 == spoof_page, "BattleEye", "Bypass everything", "[ V.2.0.0 ]", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 100), ImVec2(44, 44), 0)) spoof_page = 2;

                        if (edited::Spoof_panel(texture::spoof_icon, 4 == spoof_page, "Vulcan", "Bypass everything", "[ V.1.5.0 ]", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 100), ImVec2(44, 44), 0)) spoof_page = 4;

                        if (edited::Spoof_panel(texture::spoof_icon, 5 == spoof_page, "EAC/BE", "Bypass everything", "[ V.0.1.5 ]", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 100), ImVec2(44, 44), 2)) spoof_page = 5;

                        if (edited::Spoof_panel(texture::spoof_icon, 6 == spoof_page, "MRAC", "Bypass everything", "[ V.1.3.3 ]", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 100), ImVec2(44, 44), 2)) spoof_page = 6;
                    }
                    edited::EndChild();

  
                        PushFont(font::icomoon[40]);
                        GetWindowDrawList()->AddText(pos + ImVec2(c::background::size.x + (c::background::size.x / 2 - CalcTextSize("f").x), 80) / 2, GetColorU32(c::accent), "f");
                        PopFont();

                        GetWindowDrawList()->AddText(pos + ImVec2(c::background::size.x + (c::background::size.x / 2 - CalcTextSize("Disable any Anti-Virus before").x), 190) / 2, GetColorU32(c::accent), "Disable any Anti-Virus before");

                        SetCursorPos(ImVec2(c::background::size.x - (c::background::size.x / 2 + 280) / 2, 140));

                        edited::BeginChild("Container##2", ImVec2(280,  250), ImVec2(0, 15));
                        {
                            static char login[16] = { "" };
                            InputTextEx("a", "Login", login, 16, ImVec2(280, 40), NULL);

                            static char password[16] = { "" };
                            InputTextEx("b", "Password", password, 16, ImVec2(280, 40), ImGuiInputTextFlags_Password);

                            static bool remember = false;
                            edited::Checkbox("Remember me", &remember);

                            if (edited::Button("LAUNCH", ImVec2(280, 40))) tabs = 1;

                            SetCursorPosX(GetCursorPosX() + (270 - CalcTextSize("Forgot your passowrd? Restore").x) / 2);
                            TextColored(ImColor(GetColorU32(c::text::text_hov)), "Forgot your passowrd?");

                            SameLine(0, 5);

                            TextColored(ImColor(GetColorU32(c::accent)), "Restore?");
                        }
                        edited::EndChild();

                }
                else if (active_tab >= 1 && active_tab < 3)
                {

                    GetWindowDrawList()->AddRectFilled(pos, pos + ImVec2(c::background::size.x, 50), GetColorU32(c::element::panel), c::element::rounding, ImDrawFlags_RoundCornersTop);
                    GetWindowDrawList()->AddRectFilled(pos + ImVec2(0, 50), pos + ImVec2(100, c::background::size.y), GetColorU32(c::element::panel), c::element::rounding, ImDrawFlags_RoundCornersBottomLeft);

                    PushFont(font::icomoon[22]);
                    GetWindowDrawList()->AddText(pos + (ImVec2(100, 50) - CalcTextSize("f")) / 2, GetColorU32(c::accent), "f");
                    PopFont();

                    GetWindowDrawList()->AddText(pos + (ImVec2(c::background::size.x + 50, 48) - CalcTextSize("Eclipsebane")) / 2, GetColorU32(c::accent), "Eclipsebane");

                    SetCursorPos(ImVec2((100 - 60) / 2, 70));

                    BeginGroup();
                    {
                        if (edited::page(1 == tabs, "d", ImVec2(60, 60))) tabs = 1;

                        if (edited::page(2 == tabs, "e", ImVec2(60, 60))) tabs = 2;

                        SetCursorPosY(370);

                        if (edited::page(0 == tabs, "c", ImVec2(60, 60))) tabs = 0;
                    }
                    EndGroup();

                    ImGui::SetCursorPos(ImVec2(120, 70));

                    if (active_tab == 1)
                    {
                        edited::BeginChild("Container##1", ImVec2((c::background::size.x - 160) / 2, c::background::size.y - 90), ImVec2(18, 18), true);
                        {
                            edited::info_bar("Updated:", "21/12/23");

                            edited::Separator_line();

                            edited::info_bar("Launch:", "1min ago");

                            edited::Separator_line();

                            edited::info_bar("Status:", "Undetected");

                            edited::Separator_line();

                            edited::info_bar("Disk:", "e2e4 SSD 240GB");

                            edited::Separator_line();

                            edited::info_bar("GPU:", "NVIDIA GeForce GTX 1050 Ti");

                            ImGui::SetCursorPosY(GetCursorPosY() + 14);

                            if (edited::Button("Restore to factory settings", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 30)));

                            if (edited::Button("Serial Checker", ImVec2((GetContentRegionMax().x - (style->WindowPadding.x + 20)) / 2, 30)));
                            ImGui::SameLine(0, 20);
                            if (edited::Button("SPOOF", ImVec2((GetContentRegionMax().x - (style->WindowPadding.x + 20)) / 2, 30))) tabs = 3;
                        }
                        edited::EndChild();

                        SameLine(0, 20);

                        edited::BeginChild("Container##2", ImVec2((c::background::size.x - 160) / 2, c::background::size.y - 90), ImVec2(18, 18), true);
                        {
                            static bool baseboard = false;
                            edited::Checkbox("Baseboard", &baseboard);

                            edited::Separator_line();

                            static bool disk = false;
                            edited::Checkbox("Disk", &disk);

                            edited::Separator_line();

                            static bool bios = false;
                            edited::Checkbox("Bios", &bios);

                            edited::Separator_line();

                            static bool gpu = false;
                            edited::Checkbox("GPU", &gpu);

                            ImGui::SetCursorPosY(GetCursorPosY() + 88);

                            if (edited::Button("Confirm", ImVec2((GetContentRegionMax().x - (style->WindowPadding.x + 20)) / 2, 30)));
                            ImGui::SameLine(0, 20);
                            if (edited::Button("Save", ImVec2((GetContentRegionMax().x - (style->WindowPadding.x + 20)) / 2, 30)));
                        }
                        edited::EndChild();
                    }
                    else if (active_tab == 2)
                    {
                        edited::BeginChild("Container##1", ImVec2((c::background::size.x - 160) / 2, c::background::size.y - 90), ImVec2(10, 10), true);
                        {
                            edited::comments("12/12/23", "The great PastOwl has updated our\nstyle!");

                            edited::comments("14/14/23", "EAC\BE update now you don't have to\nworry!");

                            edited::comments("16/16/23", "New features have been added that\nwill speed up the performance of our\nspoofer!");

                            edited::comments("18/18/23", "Today we have updated, and now all\nproducts are completely in UD, you\ndon't haveto worry about your\naccount. We would also recommend\nbuying products from PastOwl.");
                        }
                        edited::EndChild();

                        SameLine(0, 20);

                        edited::BeginChild("Container##2", ImVec2((c::background::size.x - 160) / 2, c::background::size.y - 90), ImVec2(10, 10), true);
                        {
                            InputTextEx("e", "Submit your question", quest, 250, ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 40), NULL);

                            static bool anonim = false;
                            edited::Checkbox("Send anonymously", &anonim);

                            if (edited::Button("Send", ImVec2(GetContentRegionMax().x - style->WindowPadding.x, 40))) {
                                quest_text.push_back(quest);
                            };

                            edited::Separator_line();

                            for (const auto& str : quest_text) edited::question_box("PastOwl", str.c_str());
                        }
                        edited::EndChild();
                    }
                }
                else if (active_tab == 3)
                {
                    static float circle_size = 0.f;

                    circle_size = ImLerp(circle_size, active_tab == 3 ? 170.f : 0.f, ImGui::GetIO().DeltaTime * 3.f);

                    GetWindowDrawList()->AddCircle(pos + ImVec2(c::background::size) / 2, circle_size, GetColorU32(c::accent, 0.3), 100.f, 4.f);
                    GetWindowDrawList()->AddCircle(pos + ImVec2(c::background::size) / 2, circle_size * 2, GetColorU32(c::accent, 0.6), 100.f, 4.f);
                    GetWindowDrawList()->AddCircle(pos + ImVec2(c::background::size) / 2, circle_size * 3, GetColorU32(c::accent), 100.f, 4.f);
                    GetWindowDrawList()->AddCircle(pos + ImVec2(c::background::size) / 2, circle_size * 4, GetColorU32(c::accent), 100.f, 4.f);

                    GetWindowDrawList()->AddText(pos + (ImVec2(c::background::size) - CalcTextSize("Data substitution has been successfully completed.") - ImVec2(0, 20)) / 2, GetColorU32(c::accent), "Data substitution has been successfully completed.");
                    GetWindowDrawList()->AddText(pos + (ImVec2(c::background::size) - CalcTextSize("Restart your PC, and have a nice game!") + ImVec2(0, 20)) / 2, GetColorU32(c::accent, 0.3), "Restart your PC, and have a nice game!");

                    if (circle_size > 169.f) circle_size = 0.f;
                    
                }
                else if (active_tab == 4)
                {



                }

                PopStyleVar();

                SetCursorPos(ImVec2(c::background::size.x - 50, 0));
                if (edited::IconButton("k", ImVec2(50, 50), NULL)) {
                    ShowWindow(hwnd, SW_HIDE);
                    PostQuitMessage(0);
                }

                draw_circles_and_lines(GetColorU32(c::accent, 0.2f));
                move_window();

            }
            End();
        
        }
        Render();

        const float clear_color_with_alpha[4] = { 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
