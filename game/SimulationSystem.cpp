#include "SimulationSystem.h"
#include <algorithm>
#include <unordered_map>
#include <cstdlib> //rand

#include <iostream>

// самая сложная хреновина которуя в жизни писал
void SimulationSystem::endTurn(CommandManager& commandMgr, ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr)
{
    // ============================================
    // СПИСАНИЕ ЮНИТОВ С ТОЧКИ И АКТИВАЦИЯ СОЗДАННЫХ КОМАНД
    // ============================================
    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.state != CommandState::Created) continue;

        Army* army = armyMgr.getById(cmd.armyId);
        if (!army) continue;

        army->soldiers -= cmd.units;

        cmd.state = CommandState::Activated;
    }

    // =====================
    // 5. ДВИЖЕНИЕ ХОДОВ
    // =====================
    for (auto &cmd : commandMgr.commands)
    {
        if (!(cmd.state == CommandState::InBattle) && cmd.remainingTurns > 0)
            cmd.remainingTurns--;
    }

    // это вообще капяо
    // =====================
    // ГРУППИРОВКА В ФРОНТЫ (A↔B)
    // =====================
    // если на 2 точки есть две противоположные команды, то образуется фронт
    struct Front
    {
        int a;
        int b;
        std::vector<Command*> cmds;
        float battleT = 0.f;
    };
    std::vector<Front> fronts;
    // создание фронта при совпадении в командах
    auto getFront = [&](int a, int b) -> Front*
    {
        for (auto &f : fronts)
        {
            if ((f.a == a && f.b == b) || (f.a == b && f.b == a)) return &f;
        }

        fronts.push_back({a, b, {}});
        return &fronts.back();
    };
    // в этот фронт копируюся нужные команды
    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.state == CommandState::Created || cmd.state == CommandState::InRetreat) continue;

        Front* f = getFront(cmd.fromCity, cmd.toCity);
        f->cmds.push_back(&cmd);

    }

    // =====================
    // ОБНАРУЖЕНИЕ СТОЛКНОВЕНИЙ
    // =====================
    // тут вводим все команды в состояние "битвы", когда войска не могут дальше идти, нужно драться
    for (auto &f : fronts)
    {
        if (f.cmds.size() < 2)
            continue;

        for (auto *a : f.cmds)
        {
            for (auto *b : f.cmds)
            {
                if (a == b) continue;
                if (a->owner == b->owner) continue;

                float progA = commandMgr.getProgress(*a);
                float progB = commandMgr.getProgress(*b);

                if (progA >= (1.f - progB))
                {
                    a->state = CommandState::InBattle;
                    b->state = CommandState::InBattle;

                    float meet = (progA + (1.f - progB)) * 0.5f;
                    a->battleDot = meet;
                    b->battleDot = meet;
                }
            }
        }
    }

    // =====================
    // БОЙ (ВНУТРИ ФРОНТА)
    // =====================
    for (auto &f : fronts)
    {
        if (f.cmds.size() < 2)
            continue;

        for (auto *a : f.cmds)
        for (auto *b : f.cmds)
        {
            if (a == b) continue;
            if (a->owner == b->owner) continue;

            if (!(a->state == CommandState::InBattle) || !(b->state == CommandState::InBattle)) continue;

            float lossFactor = 0.2f;

            int lossA = std::min(a->units, (int)(b->units * lossFactor));
            int lossB = std::min(b->units, (int)(a->units * lossFactor));

            a->units -= lossA;
            b->units -= lossB;

        }
    }

    // =====================
    // ОКОНЧАНИЕ БИТВЫ НА ФРОНТЕ
    // =====================
    // нужно вывести умершие команды из боев и удалить их
    for (auto &f : fronts)
    {
        bool hasA = false;
        bool hasB = false;

        int ownerA = -1;
        if (!f.cmds.empty()) ownerA = f.cmds.front()->owner;

        for (auto *c : f.cmds)
        {
            if (c->units <= 0)continue;

            if (c->owner == ownerA) hasA = true;
            else hasB = true;
        }
        // если одной стороны нет — бой заканчивается
        if (!hasA || !hasB)
        {
            for (auto *c : f.cmds)
            {
                c->state = CommandState::Activated;
            }
        }
    }


    // =====================
    // ПРИБЫТИЕ
    // =====================
    // на новом месте создаются новые армии, (прибывшие из команд)
    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.remainingTurns > 0)
            continue;

        if (cmd.units <= 0)
            continue;

        City* toCity = cityMgr.findById(cmd.toCity);
        if (!toCity)
            continue;

        Army newArmy;
        newArmy.id = armyMgr.generateId();
        newArmy.owner = cmd.owner;
        newArmy.soldiers = cmd.units;
        newArmy.currentCityId = toCity->id;

        armyMgr.armies.push_back(newArmy);

        cmd.units = 0; // важно для удаления старой армии
    }

    // =====================
    //  УДАЛЕНИЕ КОМАНД
    // =====================
    // в интернете нашел, по условию удаляет, вообще круто и даже работает
    commandMgr.commands.erase(
        std::remove_if(commandMgr.commands.begin(), commandMgr.commands.end(), [](const Command& cmd)
            {
                return cmd.remainingTurns == 0 || cmd.units <= 0;
            }
        ),
        commandMgr.commands.end()
    );

    // =====================
    //  СЛИЯНИЕ АРМИЙ
    // =====================
    // когда армии пришли на новые места их всех нужно слить в одну, чтобы на точке осталась одна
    SimulationSystem::mergeArmiesInCities(armyMgr, cityMgr);

    // =====================
    //  БОИ В ГОРОДАХ
    // =====================
    // бим бим бам бам
    SimulationSystem::resolveBattles(armyMgr, cityMgr);

}



// ===================== MERGE ARMIES =====================
void SimulationSystem::mergeArmiesInCities(ArmyManager& armyMgr, CityManager& cityMgr)
{
    for (auto &city : cityMgr.cities)
    {
        std::vector<Army*> cityArmies;

        // =====================
        // собрать армии в городе
        // =====================
        for (auto &army : armyMgr.armies)
        {
            if (army.currentCityId == city.id) cityArmies.push_back(&army);
        }
        if (cityArmies.empty()) continue;

        // =====================
        // СЛИЯНИЕ АРМИЙ ОДНОГО ВЛАДЕЛЬЦА
        // =====================
        // (в будущем тут будет мораль армии я надеюсь очень что сделаю)
        for (size_t i = 0; i < cityArmies.size(); i++)
        {
            // выделяем одну главную и туда всех переводим
            Army* main = cityArmies[i];
            if (main->soldiers <= 0) continue;
            // скидываем начиная со второй
            for (size_t j = i + 1; j < cityArmies.size(); j++)
            {
                Army* other = cityArmies[j];
                if (other->soldiers <= 0) continue;

                if (main->owner == other->owner)
                {
                    main->soldiers += other->soldiers;
                    other->soldiers = 0;
                }
            }
        }

        // =====================
        // ОПРЕДЕЛЕНИЕ ОСАДЫ - когда в городе сразу две армиии - город ничей
        // =====================
        int firstOwner = -1;
        bool conflict = false;

        for (auto *army : cityArmies)
        {
            if (army->soldiers <= 0) continue;

            if (firstOwner == -1) firstOwner = army->owner;
            else if (army->owner != firstOwner) conflict = true;
        }

        if (conflict)
        {
            city.owner = -1; // ОСАДА
        }
        else if (firstOwner != -1)
        {
            city.owner = firstOwner;
        }
    }

    // =====================
    // удалить мёртвые армии
    // =====================
    armyMgr.armies.erase(
        std::remove_if(
            armyMgr.armies.begin(),
            armyMgr.armies.end(),
            [](const Army& a)
            {
                return a.soldiers <= 0;
            }
        ),
        armyMgr.armies.end()
    );
}


// ===================== RESOLVE BATTLES =====================
// когда собрали все армии в одну начинаем выяснять отношения
void SimulationSystem::resolveBattles(ArmyManager& armyMgr, CityManager& cityMgr)
{
    for (auto &city : cityMgr.cities)
    {
        std::vector<Army*> armiesInCity;

        for (auto &army : armyMgr.armies)
        {
            if (army.currentCityId == city.id) armiesInCity.push_back(&army);
        }
        if (armiesInCity.size() <= 1) continue;

        // =====================
        // группировка по владельцу (cборка сил)
        // =====================
        std::unordered_map<int, int> power;

        for (auto *army : armiesInCity)
        {
            power[army->owner] += army->soldiers;
        }

        if (power.size() <= 1) continue;

        // =====================
        // берём 2 стороны
        // =====================
        auto it = power.begin();
        int ownerA = it->first;
        int powerA = it->second;

        ++it;
        int ownerB = it->first;
        int powerB = it->second;

        // =====================
        // ОСАДНЫЙ КОЭФФИЦИЕНТ
        // =====================
        float siegeFactor = (city.owner == -1) ? 0.4f : 1.0f;// крутая структура нашел в интеренете как if else короче
        float lossFactor = 0.5f;

        // !!!!!позже добавить рандом!!!!

        int lossesA = std::min(powerA, (int)(powerB * lossFactor * siegeFactor));
        int lossesB = std::min(powerB, (int)(powerA * lossFactor * siegeFactor));

        // =====================
        // применяем потери
        // =====================
        // это мне дал чат жипити потом переделать пока лень (применение потерь)
        for (auto *army : armiesInCity)
        {
            if (army->owner == ownerA)
            {
                int share = army->soldiers;
                int loss = (int)((float)share / powerA * lossesA);
                army->soldiers -= loss;
            }
            else if (army->owner == ownerB)
            {
                int share = army->soldiers;
                int loss = (int)((float)share / powerB * lossesB);
                army->soldiers -= loss;
            }
        }

        // =====================
        // ура победа
        // =====================

        int remainingA = 0;
        int remainingB = 0;

        for (auto *army : armiesInCity)
        {
            if (army->owner == ownerA) remainingA += army->soldiers;
            else if (army->owner == ownerB) remainingB += army->soldiers;
        }
        if (remainingA > 0 && remainingB > 0) city.owner = -1; // бой продолжается
        // победа какой то из сторон
        else if (remainingA > 0 && remainingB <= 0) city.owner = ownerA;

        else if (remainingB > 0 && remainingA <= 0) city.owner = ownerB;
    }

    // =====================
    // удалить мёртвые армии
    // =====================
    armyMgr.armies.erase(
        std::remove_if(
            armyMgr.armies.begin(),
            armyMgr.armies.end(),
            [](const Army& a)
            {
                return a.soldiers <= 0;
            }
        ),
        armyMgr.armies.end()
    );
}



// пу пу пу
#include <cstdlib> // rand()

void SimulationSystem::makeAITurns(CommandManager& commandMgr, ArmyManager& armyMgr, CityManager& cityMgr)
{
    for (auto &army : armyMgr.armies)
    {
        // проверки всякие важные
        // только враг
        if (army.owner != 1) continue;
        // не отправляем слабые армии
        if (army.soldiers < 10)continue;
        // город существует
        City* from = cityMgr.findById(army.currentCityId);
        if (!from) continue;
        // соседи есть
        if (from->neighbors.empty())continue;

        int fromCity = from->id;
        int bestTarget = -1;

        // =========================
        // 1. ищем врага
        // =========================
        for (int neighbor : from->neighbors)
        {
            City* c = cityMgr.findById(neighbor);
            if (!c) continue;

            if (c->owner != army.owner)
            {
                bestTarget = neighbor;
                break;
            }
        }

        // =========================
        // 2. если нет — случайный
        // =========================
        if (bestTarget == -1)
        {
            int index = rand() % from->neighbors.size();
            bestTarget = from->neighbors[index];
        }

        // =========================
        // снова проверки
        // =========================
        if (!cityMgr.canMoveBetweenCities(fromCity, bestTarget)) continue;

        if (commandMgr.isDuplicateInTurn(army.id, fromCity, bestTarget)) continue;

        if (commandMgr.getPendingCount() > 5) break;

        int turns = cityMgr.distance(fromCity, bestTarget);

        int sendUnits = army.soldiers / 2;

        if (sendUnits <= 0) continue;

        // =========================
        // создание команды
        // =========================
        commandMgr.commands.push_back({
            army.owner,
            army.id,
            fromCity,
            bestTarget,
            sendUnits,
            turns,
            turns
        });

        commandMgr.updateOffsets(fromCity, bestTarget);
    }
}


//
