#include "CommandManager.h"


// проверка на дупликаты на создаваемых командах (нельзя две одинаковых на ход)
bool CommandManager::isDuplicateInTurn(int armyId, int fromCity, int toCity)
{
    for (auto &cmd : commands)
    {
        if (cmd.state == CommandState::Created) // только команды текущего хода
        {
            if (cmd.armyId == armyId && cmd.fromCity == fromCity && cmd.toCity == toCity) return 1;
        }
    }
    return 0;
}

// сколько юнитов зарезервировано на командах rn
int CommandManager::getReservedUnits(int armyId)
{
    int sum = 0;
    for (auto &c : commands)
        if (c.armyId == armyId && c.state == CommandState::Created)
            sum += c.units;
    return sum;
}
// из кол-ва зарезервированных можно получить остаток
int CommandManager::getAvailableUnits(int armyId, ArmyManager& armyMgr)
{
    Army* a = armyMgr.getById(armyId);
    if (!a) return 0;

    return a->soldiers - getReservedUnits(armyId);
}

// кол-во команд этого хода (чтобы не превысить максимум на ход)
int CommandManager::getPendingCount()
{
    int count = 0;

    for (auto &c : commands)
    {
        if (c.state == CommandState::Created)
            count++;
    }
    return count;
}

// группировка аээ не помню. по моему это для дупликатов, чтобы форму поменять при наслоении команд
std::vector<Command*> CommandManager::getGroup(int fromCity, int toCity)
{
    std::vector<Command*> group;

    for (auto &cmd : commands)
    {
        if (cmd.fromCity == fromCity && cmd.toCity == toCity)
            group.push_back(&cmd);
    }
    return group;
}

// обновление формы кривой для наслоенной команды
void CommandManager::updateOffsets(int fromCity, int toCity)
{
    auto group = getGroup(fromCity, toCity);

    int n = group.size();
    if (n == 0) return;

    float spacing = 25.f;

    for (int i = 0; i < n; i++)
    {
        group[i]->offset += 0.1f;
    }
}

// прогресс ходов до прибытия команды на точку
float CommandManager::getProgress(Command& cmd)
{
    if (cmd.totalTurns == 0) return 0.f;

    return 1.f - (float)cmd.remainingTurns / cmd.totalTurns;
}
// являются ли команды противоположными
bool CommandManager::isOpposite(const Command& a, const Command& b)
{
    return a.fromCity == b.toCity && a.toCity == b.fromCity && a.owner != b.owner;
}

// отступ с линии фронта
void CommandManager::retreat(Command& cmd, int units)
{
    // =====================
    // ПРОВЕРКИ
    // =====================
    if (units <= 0 || units > cmd.units)return;

    // =====================
    // 2. СКОЛЬКО ПРОШЛИ
    // =====================
    int passed = cmd.totalTurns - cmd.remainingTurns;
    if (passed <= 0) return; // ещё не вышли из города

    // =====================
    // 3. СОЗДАЁМ КОМАНДУ НАЗАД
    // =====================
    Command r;

    r.owner = cmd.owner;
    r.armyId = cmd.armyId;

    r.fromCity = cmd.toCity;   // ️ важно (идём обратно)
    r.toCity   = cmd.fromCity;

    r.units = units;

    r.totalTurns = passed;
    r.remainingTurns = passed;

    r.state = CommandState::InRetreat;

    r.offset = 0.2f; // можно оставить тот же или пересчитать

    r.battleDot = 1.f - getProgress(cmd);

    r.morale = cmd.morale;

    // =====================
    // 4. ВЫЧИТАЕМ ЮНИТЫ
    // =====================
    cmd.units -= units;

    // =====================
    // 5. ДОБАВЛЯЕМ
    // =====================
    commands.push_back(r);
    //updateOffsets(r.fromCity, r.toCity);
    // после создания нужно проверить наслоения
}



