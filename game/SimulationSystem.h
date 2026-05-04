#pragma once

#include "../world/ArmyManager.h"
#include "../world/CityManager.h"
#include "../world/CommandManager.h"
#include "AnimationManager.h"

class SimulationSystem
{
public:

    static void endTurn(CommandManager& commandMgr, ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr); // закончить ход

    static void mergeArmiesInCities(ArmyManager& armyMgr, CityManager& cityMgr); // работа с армиями после хода (обработка хода)

    static void resolveBattles(ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr); // бои на готовых армиях

    static void makeAITurns(CommandManager& commandMgr, ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr);
};

