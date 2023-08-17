#include "Init.h"
#include "features/PlayerList.h"

namespace initSpace {
#define ADD(name) Feature::features[name::GetInstance().GetInfo().tableName].push_back(&name::GetInstance())

    auto Feature::Init() -> void { ADD(PlayerList); }
}
