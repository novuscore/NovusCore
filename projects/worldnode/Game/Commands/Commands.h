/*
    MIT License

    Copyright (c) 2018-2019 NovusCore

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once
/*#include <NovusTypes.h>
#include <Utils/StringUtils.h>
#include "../../ECS/Components/Singletons/CommandDataSingleton.h"
#include "../../ECS/Components/Singletons/MapSingleton.h"

#include "Commands_Character.h"*/

namespace Commands
{
/*static entt::registry* _registry = nullptr;

    bool _GPS(std::vector<std::string> commandStrings, PlayerConnectionComponent& clientConnection)
    {
        PlayerPositionComponent& playerPos = _registry->get<PlayerPositionComponent>(clientConnection.entityId);
        MapSingleton& mapSingleton = _registry->ctx<MapSingleton>();

        u16 mapId = playerPos.mapId;
        f32 x = playerPos.x;
        f32 y = playerPos.y;
        f32 z = playerPos.z;

        Vector2 pos(x, y);
        f32 height = mapSingleton.maps[mapId].GetHeight(pos);
        clientConnection.SendChatNotification("MapID: %u (%f, %f, %f) Height: %f", mapId, x, y, z, height);

        return true;
    }

    void LoadCommands(entt::registry& registry)
    {
        CommandDataSingleton& commandDataSingleton = registry.set<CommandDataSingleton>();
        commandDataSingleton.commandMap["gps"_h] = CommandEntry(_GPS, 0);

        Commands_Character::LoadCharacterCommands(registry, commandDataSingleton);

        _registry = &registry;
    }*/
}
