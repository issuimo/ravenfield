#pragma once
#include "../Init.h"

class PlayerList : public initSpace::Feature {
public:
    struct Actor {
        char      _space2[0xD8];
        unity::CSharper::String* name;
        char      _space[0X38];
        float     health;
        float     maxHealth;
        float     balance;
        float     maxBalance;
        bool      dead;
    };

    static auto  GetInstance() -> PlayerList&;
    virtual auto GetInfo() const -> const GuiInfo& override;
    virtual auto Render() -> void override;
    virtual auto Update() -> void override;
    virtual auto DrawStatus() -> void override;
    virtual auto Save(nlohmann::json& json) -> void override;
    virtual auto Load(nlohmann::json& json) -> void override;

protected:
    using Actor_Awake = void(*)(Actor* _this);
    static auto Actor_Awake_Hook(Actor* p) -> void;
    using Actor_Deactivate = void(*)(Actor* _this);
    static auto Actor_Deactivate_Hook(Actor* actor) -> void;

    PlayerList();

private:
    inline static std::mutex          mutex_;
    inline static std::vector<Actor*> actors_;
    inline static Actor_Awake         actor_awake_;
    inline static Actor_Deactivate    actor_deactivate_;
};
