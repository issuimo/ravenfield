#include "PlayerList.h"

auto PlayerList::Actor_Awake_Hook(Actor* actor) -> void {
    std::cout << std::format("Player Awake | {:#X}\n", reinterpret_cast<INT64>(actor));

    std::lock_guard lock(mutex_);
    actors_.push_back(actor);

    return HookManager::call(Actor_Awake_Hook, actor);
}

auto PlayerList::Actor_Deactivate_Hook(Actor* actor) -> void {
    std::cout << std::format("Player Deactivate | {:#X}\n", reinterpret_cast<INT64>(actor));

    std::lock_guard lock(mutex_);
    auto it = std::find(actors_.begin(), actors_.end(), actor);
    if (it != actors_.end())
        actors_.erase(it);

    return HookManager::call(Actor_Deactivate_Hook, actor);
}

auto Actor_GetMain_Hook(unity::CSharper::Camera* camera) -> unity::CSharper::Camera* {
    camera->SetDepth(0);
    const auto temp = HookManager::call(Actor_GetMain_Hook, camera);
    temp->SetDepth(0);
    camera->GetCurrent()->SetDepth(0);
    return temp;
}

PlayerList::PlayerList() : Feature{} {
    actor_awake_ = reinterpret_cast<Actor_Awake>(unity::Mono::Method::GetAddress("Actor", "Awake"));
    std::cout << std::hex << reinterpret_cast<INT64>(actor_awake_) << std::endl;
    HookManager::install(actor_awake_, Actor_Awake_Hook);

    actor_deactivate_ = reinterpret_cast<Actor_Deactivate>(unity::Mono::Method::GetAddress("Actor", "Deactivate"));
    std::cout << std::hex << reinterpret_cast<INT64>(actor_deactivate_) << std::endl;
    HookManager::install(actor_deactivate_, Actor_Deactivate_Hook);

    HookManager::install(reinterpret_cast<unity::CSharper::Camera*(*)(unity::CSharper::Camera*)>(unity::Mono::Method::GetAddress("Camera", "get_main", 0)), Actor_GetMain_Hook);
}

auto PlayerList::GetInstance() -> PlayerList& {
    static PlayerList Instance;
    return Instance;
}

auto PlayerList::GetInfo() const -> const GuiInfo& {
    static GuiInfo info{reinterpret_cast<const char*>(u8"玩家"), reinterpret_cast<const char*>(u8"玩家列表"), true};
    return info;
}

auto PlayerList::Render() -> void {
    if (ImGui::BeginTable("PlayerList",
                          3,
                          ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
                          ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                          ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable,
                          ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 13))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn(reinterpret_cast<const char*>(u8"[操作]"), ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn(reinterpret_cast<const char*>(u8"[名称]"), ImGuiTableColumnFlags_None);
        ImGui::TableSetupColumn(reinterpret_cast<const char*>(u8"[血量]"), ImGuiTableColumnFlags_None);
        ImGui::TableHeadersRow();

        std::lock_guard lock(mutex_);
        for (const auto& actor : actors_) {
            ImGui::TableNextRow();
            try {
                ImGui::PushID(actor);

                if (ImGui::TableSetColumnIndex(0)) { }
                if (ImGui::TableSetColumnIndex(1))
                    ImGui::Text(actor->name->ToString().c_str());
                if (ImGui::TableSetColumnIndex(2))
                    ImGui::Text("%f", actor->health);

                ImGui::PopID();
            } catch (...) { }
        }

        ImGui::EndTable();
    }
}

auto PlayerList::Update() -> void {}

auto PlayerList::DrawStatus() -> void {}

auto PlayerList::Save(nlohmann::json& json) -> void {}

auto PlayerList::Load(nlohmann::json& json) -> void {}