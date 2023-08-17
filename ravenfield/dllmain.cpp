#include "Init.h"

#include "library/Font.hpp"
#include "library/d3d11hook.h"
#include "library/log.h"
#include "library/imgui/imgui_impl_dx11.h"
#include "library/imgui/imgui_impl_win32.h"

extern auto ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT;

auto APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) -> BOOL {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            std::thread([] {
                // 打开控制台
                easyLog::Log::OpenConsole();

                // 初始化Mono
                unity::Mono::SetModule(GetModuleHandleA("mono-2.0-bdwgc.dll"));

                // 初始化功能列表
                initSpace::Feature::Init();

                // 安装D3D11HOOK
                dxhook::Hk11::Build([] {
                    if (!initSpace::guiInfo.imGuiInit) {
                        IMGUI_CHECKVERSION();

                        // 创建ImGui上下文
                        ImGui::CreateContext();

                        // 获取ImGui IO 并设置 键盘和手柄控制
                        auto io = ImGui::GetIO();
                        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
                        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

                        // 设置中文字体
                        io.Fonts->AddFontFromMemoryTTF(fontdata,
                                                       sizeof fontdata,
                                                       13,
                                                       nullptr,
                                                       io.Fonts->GetGlyphRangesChineseFull());

                        // 设置ImGui主题
                        ImGui::StyleColorsClassic();

                        // 初始化ImGui D3D11设备和窗口
                        ImGui_ImplWin32_Init(dxhook::Hk11::GetHwnd());
                        ImGui_ImplDX11_Init(dxhook::Hk11::GetDevice(), dxhook::Hk11::GetContext());

                        // 接管窗口消息
                        dxhook::Hk11::SetWndProc([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> char {
                            // ImGui消息处理
                            if (initSpace::guiInfo.mainShow)
                                ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

                            // 按键处理
                            switch (msg) {
                                case WM_KEYDOWN:
                                    // 处理界面显示/隐藏
                                    if (wParam == VK_DELETE) {
                                        if (initSpace::guiInfo.mainShow)
                                            initSpace::guiInfo.mainShow = false;
                                        else
                                            initSpace::guiInfo.mainShow = true;
                                    }
                                    break;
                                case WM_KEYUP:
                                    break;
                                case WM_CLOSE:
                                    const int result = MessageBox(nullptr,
                                                                  L"你确定要退出游戏吗?",
                                                                  L"Confirmation",
                                                                  MB_YESNO | MB_ICONQUESTION);

                                    if (result == IDYES) {
                                        // 执行关闭操作
                                        exit(0);
                                    }
                                    if (result == IDNO) {
                                        // 用户选择不关闭
                                        return 2;
                                    }
                                    break;
                            }

                            return !initSpace::guiInfo.mainShow;
                        });

                        initSpace::guiInfo.imGuiInit = true;
                        initSpace::guiInfo.mainShow  = true;
                    }

                    // 创建新的画面帧
                    ImGui_ImplDX11_NewFrame();
                    ImGui_ImplWin32_NewFrame();
                    ImGui::NewFrame();

                    // 主界面
                    if (initSpace::guiInfo.mainShow) {
                        if (ImGui::Begin(reinterpret_cast<const char*>(u8"Ravenfield Cheat By 遂沫"))) {
                            if (ImGui::BeginTabBar("memList")) {
                                for (const auto& [name, _features] : initSpace::Feature::features) {
                                    if (ImGui::BeginTabItem(name.c_str())) {
                                        for (const auto func : _features) {
                                            if (func->GetInfo().needGroup) {
                                                if (ImGui::CollapsingHeader(func->GetInfo().groupName.c_str()))
                                                    func->Render();
                                            }
                                            else
                                                func->Render();
                                        }
                                        ImGui::EndTabItem();
                                    }
                                }

                                ImGui::EndTabBar();
                            }

                            ImGui::End();
                        }
                    }

                    for (const auto& _features : initSpace::Feature::features | std::views::values) {
                        for (const auto func : _features)
                            func->DrawStatus();
                    }

                    // 结束并渲染
                    ImGui::EndFrame();
                    ImGui::Render();
                    dxhook::Hk11::GetContext()->OMSetRenderTargets(1, dxhook::Hk11::GetTargetView(), nullptr);
                    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                });

                // 线程异步更新
                std::vector<std::future<void>> futuresUpdate;
                while (true) {
                    Sleep(1);

                    // 多线程并发
                    for (const auto feature : initSpace::Feature::features | std::views::values) {
                        for (const auto func : feature)
                            futuresUpdate.push_back(std::async(std::launch::async, [&func] { func->Update(); }));
                    }

                    // 检查是否所有任务都已完成
                    for (auto& future : futuresUpdate) {
                    wait: if (future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                            Sleep(1);
                            goto wait;
                        }
                    }

                    futuresUpdate.clear();
                }
            }).detach();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
