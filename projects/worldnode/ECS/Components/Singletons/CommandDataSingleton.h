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
#include <NovusTypes.h>
#include <robin_hood.h>
#include <vector>

#include "../playerFieldDataComponent.h"

// Nix: I decided to make a struct for the handler because in the future we will want permissions and potentially other variables for each command.
typedef bool (*CommandHandler)(std::vector<std::string>, PlayerConnectionComponent&);
struct CommandEntry
{
    CommandEntry() {}
    CommandEntry(CommandHandler commandHandler, i32 inParameters) : handler(commandHandler), parameters(inParameters) {}
    CommandHandler handler;
    i32 parameters;
};
struct CommandDataSingleton
{
    CommandDataSingleton() : commandMap() {}
    robin_hood::unordered_map<u32, CommandEntry> commandMap;
};