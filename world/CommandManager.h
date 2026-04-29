#pragma once
#include "Command.h"
#include "ArmyManager.h"
#include <vector>

class CommandManager {
public:
    std::vector<Command> commands;

    bool isDuplicateInTurn(int armyId, int fromCity, int toCity); // одинаковые команды на одном ходу

    int getReservedUnits(int armyId);// зарезервированные юниты на этот ход
    int getAvailableUnits(int armyId, ArmyManager& armyMgr);// доступные на ход

    int getPendingCount();// кол-во коман на ход

    std::vector<Command*> getGroup(int fromCity, int toCity);// группировка при наслоении
    void updateOffsets(int fromCity, int toCity);//перерасчет линии при наслоении

    float getProgress(Command& cmd);// прогресс топ топ для команды
    bool isOpposite(const Command& a, const Command& b);// являются ли команды противоположными

    void retreat(Command& cmd, int units); // отступ с линии фронта


};
