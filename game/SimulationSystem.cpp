#include "SimulationSystem.h"
#include <algorithm>
#include <unordered_map>
#include <cstdlib> //rand
#include <unordered_set>
#include <cmath>
#include <iostream>

// самая сложная хреновина которуя в жизни писал


// конец хода - все итоги сражения. все.
void SimulationSystem::endTurn(CommandManager& commandMgr, ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr, int turnCount)
{
    if (turnCount > 15)
    {
        // каждый ход армия наполеона потихоньку сдает
        for (auto &army : armyMgr.armies)
        {

            if (army.owner == 1) army.morale *= 0.97f;
            if (army.owner == 0) army.morale *= 1.03f;
        }
    }

    // ============================================
    // СПИСАНИЕ ЮНИТОВ С ТОЧКИ И АКТИВАЦИЯ СОЗДАННЫХ КОМАНД
    // ============================================
    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.state == CommandState::Created || cmd.state == CommandState::Animating)
        {
            Army* army = armyMgr.getById(cmd.armyId);
            if (!army) continue;

            army->soldiers -= cmd.units;
            cmd.state = CommandState::Activated;

            //anim cписание
            //City *city = cityMgr.findById(army->currentCityId);
            //animMgr.spawnNumber(city->position, cmd.units, 25.0);
        }
    }
    // =====================
    // ДВИЖЕНИЕ ХОДОВ
    // =====================
    for (auto &cmd : commandMgr.commands)
    {
        if (!(cmd.state == CommandState::InBattle) && cmd.remainingTurns > 0)
            cmd.remainingTurns--;
    }


   // =====================
    // ФРОНТЫ
    // =====================

    struct Front
    {
        int a;
        int b;

        std::vector<Command*> cmds;

        bool activeBattle = false;
        float battleDot = 0.5f;
    };

    std::vector<Front> fronts;

    auto getFront = [&](int a, int b) -> Front*
    {
        for (auto &f : fronts)
        {
            if ((f.a == a && f.b == b) ||
                (f.a == b && f.b == a))
            {
                return &f;
            }
        }

        fronts.push_back({
            a,
            b
        });

        return &fronts.back();
    };

    // =====================
    // СОЗДАНИЕ ФРОНТОВ
    // =====================

    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.units <= 0)
            continue;

        if (cmd.state == CommandState::InRetreat)
            continue;

        Front* f = getFront(cmd.fromCity, cmd.toCity);

        f->cmds.push_back(&cmd);
    }

    // =====================
    // ОБНАРУЖЕНИЕ СТОЛКНОВЕНИЙ
    // =====================

    for (auto &f : fronts)
    {
        if (f.cmds.size() < 2)
            continue;

        // ищем существующий battleDot
        bool existingBattle = false;
        float existingDot = 0.5f;

        for (auto *cmd : f.cmds)
        {
            if (cmd->state == CommandState::InBattle)
            {
                existingBattle = true;

                if (cmd->fromCity == f.a)
                    existingDot = cmd->battleDot;
                else
                    existingDot = 1.f - cmd->battleDot;

                break;
            }
        }

        // =====================
        // ФРОНТ УЖЕ СУЩЕСТВУЕТ
        // =====================

        if (existingBattle)
        {
            for (auto *cmd : f.cmds)
            {
                // уже стоит на фронте
                if (cmd->state == CommandState::InBattle)
                    continue;

                float prog = 1.f - ((float)cmd->remainingTurns / cmd->totalTurns);

                bool reachedBattle = false;

                float targetDot;

                // команда идёт со стороны A
                if (cmd->fromCity == f.a)
                {
                    targetDot = existingDot;
                }
                else
                {
                    targetDot = 1.f - existingDot;
                }

                cmd->battleDot = targetDot;

                reachedBattle = prog >= targetDot;

                // дошёл до фронта
                if (reachedBattle)
                {
                    cmd->state = CommandState::InBattle;
                }
            }

            continue;
        }

        // =====================
        // НОВОЕ СТОЛКНОВЕНИЕ
        // =====================

        bool battleStarted = false;

        for (auto *a : f.cmds)
        {
            for (auto *b : f.cmds)
            {
                if (a == b) continue;

                if (a->owner == b->owner)continue;

                if (a->fromCity == b->fromCity) continue;

                // путь
                int path = std::max(a->totalTurns, b->totalTurns);

                // позиции армий
                int posA = a->totalTurns - a->remainingTurns;
                int posB = b->totalTurns - b->remainingTurns;

                // расстояние между ними
                int distance = path - (posA + posB);

                // столкновение
                if (distance > 0)
                    continue;

                // =====================
                // СОЗДАЁМ ФРОНТ
                // =====================

                float meet = (float)posA / path;

                meet = std::clamp(meet, 0.05f, 0.95f);

                a->state = CommandState::InBattle;
                b->state = CommandState::InBattle;

                a->battleDot = meet;
                b->battleDot = 1.f - meet;

                battleStarted = true;

                break;
            }

            if (battleStarted)
                break;
        }
    }


    // =====================
    // БОЙ (ЧЕРЕЗ СТОРОНЫ)
    // =====================
    for (auto &f : fronts)
    {
        if (f.cmds.size() < 2)
            continue;

        struct Group
        {
            int owner;
            std::vector<Command*> cmds;

            int totalUnits = 0;
            float morale;
        };

        std::vector<Group> groups;

        // -------- ГРУППИРОВКА --------
        for (auto *cmd : f.cmds)
        {
            if (cmd->state != CommandState::InBattle)
                continue;

            auto it = std::find_if(groups.begin(), groups.end(),
                [&](const Group &g){ return g.owner == cmd->owner; });

            if (it == groups.end())
            {
                groups.push_back({cmd->owner});
                it = std::prev(groups.end());
            }

            it->cmds.push_back(cmd);
        }

        if (groups.size() < 2)
            continue;

        // -------- СЧИТАЕМ ЧИСЛЕННОСТЬ --------
        for (auto &g : groups)
        {
            int count = 0;
            for (auto *cmd : g.cmds)
            {
                g.totalUnits += cmd->units;
                g.morale += cmd->morale;

                count++;
            }
            g.morale = g.morale / count;
        }

        // -------- БОЙ --------
        float lossFactor = 0.2f;

        auto &gA = groups[0];
        auto &gB = groups[1];

        int unitsA = gA.totalUnits;
        int unitsB = gB.totalUnits;


        if (unitsA <= 0 || unitsB <= 0) continue;

        // -------- СЧИТАЕМ ПОТЕРИ --------
        float k = 0.25f;

        int lossA = std::max(30, (int)(k * gB.totalUnits));
        int lossB = std::max(30, (int)(k * gA.totalUnits));


        lossA = lossA * gB.morale;
        lossB = lossB * gA.morale;

        //std::cout << "lossA: " << lossA << " lossB: " << lossB << std::endl;
        //std::cout << "MoraleA: " << gA.morale << " moraleB: " << gB.morale << std::endl;

        // -------- НАКАПЛИВАЕМ УРОН --------
        std::unordered_map<Command*, int> pendingLoss;


        // =====================
        // РАСПРЕДЕЛЕНИЕ ДЛЯ A
        // =====================
        int distributedA = 0;
        Command* biggestA = nullptr;
        int maxUnitsA = 0;

        for (auto *cmd : gA.cmds)
        {
            if (cmd->units <= 0)
                continue;

            if (cmd->units > maxUnitsA)
            {
                maxUnitsA = cmd->units;
                biggestA = cmd;
            }

            float share = (float)cmd->units / gA.totalUnits;
            int loss = (int)(lossA * share);

            loss = std::min(loss, cmd->units / 2);
            loss = std::min(loss, cmd->units);

            pendingLoss[cmd] += loss;
            distributedA += loss;
        }

        // добиваем остаток
        int remainderA = lossA - distributedA;
        if (remainderA > 0 && biggestA)
        {
            int add = std::min(remainderA, biggestA->units);
            pendingLoss[biggestA] += add;
        }


        // =====================
        // РАСПРЕДЕЛЕНИЕ ДЛЯ B
        // =====================
        int distributedB = 0;
        Command* biggestB = nullptr;
        int maxUnitsB = 0;

        for (auto *cmd : gB.cmds)
        {
            if (cmd->units <= 0)
                continue;

            if (cmd->units > maxUnitsB)
            {
                maxUnitsB = cmd->units;
                biggestB = cmd;
            }

            float share = (float)cmd->units / gB.totalUnits;
            int loss = (int)(lossB * share);

            loss = std::min(loss, cmd->units / 2);
            loss = std::min(loss, cmd->units);

            pendingLoss[cmd] += loss;
            distributedB += loss;
        }

        // добиваем остаток
        int remainderB = lossB - distributedB;
        if (remainderB > 0 && biggestB)
        {
            int add = std::min(remainderB, biggestB->units);
            pendingLoss[biggestB] += add;
        }


        // =====================
        // ПРИМЕНЯЕМ УРОН и пробуем мораль
        // =====================
        for (auto &[cmd, loss] : pendingLoss)
        {
            float moraleloss =  (float)loss / (cmd->units + 1) * (float)loss / (cmd->units + 1);

            cmd->morale -= moraleloss;
            if (cmd->morale < 0.4f) cmd->morale = 0.4f;

            cmd->units -= loss;


            City* a = cityMgr.findById(cmd->fromCity);
            City* b = cityMgr.findById(cmd->toCity);
            auto curve = cityMgr.buildCurve(a->position, b->position, cmd->offset);
            sf::Vector2f battlePoint = cityMgr.getPointOnCurve(curve, cmd->battleDot);

            if (cmd->owner == 0) animMgr.spawnNumber(battlePoint, loss, 0); // plr
            else animMgr.spawnNumber(battlePoint, loss, 0); // ii
        }
    }



    // =====================
    // ОКОНЧАНИЕ БОЯ
    // =====================

    for (auto &f : fronts)
    {
        // =====================
        // ТОЛЬКО УЧАСТНИКИ БОЯ
        // =====================

        std::vector<Command*> battleParticipants;

        for (auto *c : f.cmds)
        {
            if (c->state != CommandState::InBattle)
                continue;

            if (c->units <= 0)
                continue;

            battleParticipants.push_back(c);
        }

        if (battleParticipants.empty())
            continue;

        // =====================
        // ПРОВЕРКА ПОБЕДИТЕЛЯ
        // =====================

        std::unordered_set<int> aliveOwners;

        for (auto *c : battleParticipants)
        {
            aliveOwners.insert(c->owner);
        }

        // бой ещё идёт
        if (aliveOwners.size() > 1)
            continue;

        // =====================================================
        // ГРУППИРОВКА ПО:
        // owner + сторона фронта
        // =====================================================

        struct GroupKey
        {
            int owner;
            bool fromA;

            bool operator==(const GroupKey& other) const
            {
                return owner == other.owner
                    && fromA == other.fromA;
            }
        };

        struct GroupKeyHash
        {
            size_t operator()(const GroupKey& k) const
            {
                return (k.owner * 10) + k.fromA;
            }
        };

        std::unordered_map<
            GroupKey,
            std::vector<Command*>,
            GroupKeyHash
        > winnerGroups;

        // =====================
        // ГРУППИРУЕМ
        // =====================

        for (auto *c : battleParticipants)
        {
            GroupKey key;

            key.owner = c->owner;

            // true = шёл со стороны A
            // false = со стороны B
            key.fromA = (c->fromCity == f.a);

            winnerGroups[key].push_back(c);
        }

        // =====================
        // ОБЪЕДИНЕНИЕ
        // =====================

        for (auto &[key, cmds] : winnerGroups)
        {
            if (cmds.empty())
                continue;

            Command* main = cmds.front();

            int totalUnits = 0;
            float morale = 0.f;

            int count = 0;

            for (auto *c : cmds)
            {
                totalUnits += c->units;

                morale += c->morale;

                count++;
            }

            morale /= std::max(1, count);

            // =====================
            // ГЛАВНАЯ КОМАНДА
            // =====================

            main->units = totalUnits;
            main->morale = morale;

            main->state = CommandState::Activated;

            // больше не стоит на фронте
            main->battleDot = 1.f;

            // =====================
            // УДАЛЯЕМ ОСТАЛЬНЫЕ
            // =====================

            for (size_t i = 1; i < cmds.size(); i++)
            {
                cmds[i]->units = 0;
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
        newArmy.morale = cmd.morale;

        armyMgr.armies.push_back(newArmy);

        cmd.units = 0; // важно для удаления старой армии

        // анимируем прибытие команды
        City* a = cityMgr.findById(cmd.fromCity);
        City* b = cityMgr.findById(cmd.toCity);

        std::vector<sf::Vector2f> curve = cityMgr.buildCurve(b->position, a->position, -cmd.offset);

        sf::Color color;
        if (cmd.owner==1) color = sf::Color::Blue;
        if (cmd.owner==0) color = sf::Color::Yellow;

        animMgr.spawnLineDisappear(curve, color);
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

    animMgr.delay(0.5f, [&]()
        {
            SimulationSystem::mergeArmiesInCities(armyMgr, cityMgr);
            SimulationSystem::resolveBattles(armyMgr, cityMgr, animMgr);
        });
    // эти пункты чуууть выше
    // =====================
    //  СЛИЯНИЕ АРМИЙ
    // =====================
    // когда армии пришли на новые места их всех нужно слить в одну, чтобы на точке осталась одна

    // =====================
    //  БОИ В ГОРОДАХ
    // =====================
    // бим бим бам бам (но нужно подождать пока анимация отработает)

}

// ===================== MERGE ARMIES =====================
void SimulationSystem::mergeArmiesInCities(ArmyManager& armyMgr, CityManager& cityMgr)
{
    for (auto &city : cityMgr.cities)
    {
        std::vector<Army*> cityArmies;

        // =====================
        // СОБРАТЬ АРМИИ В ГОРОДЕ
        // =====================
        for (auto &army : armyMgr.armies)
        {
            if (army.currentCityId == city.id)
                cityArmies.push_back(&army);
        }

        if (cityArmies.empty())
            continue;

        // =====================
        // СЛИЯНИЕ АРМИЙ ОДНОГО ВЛАДЕЛЬЦА
        // =====================
        for (size_t i = 0; i < cityArmies.size(); i++)
        {
            Army* main = cityArmies[i];

            if (main->soldiers <= 0)
                continue;

            // =====================
            // ДЛЯ СРЕДНЕЙ МОРАЛИ
            // =====================
            float moraleSum = main->morale;
            int moraleCount = 1;

            for (size_t j = i + 1; j < cityArmies.size(); j++)
            {
                Army* other = cityArmies[j];

                if (other->soldiers <= 0)
                    continue;

                // только армии одного владельца
                if (main->owner != other->owner)
                    continue;

                // =====================
                // ОБЪЕДИНЕНИЕ
                // =====================
                main->soldiers += other->soldiers;

                moraleSum += other->morale;
                moraleCount++;

                other->soldiers = 0;
            }

            // =====================
            // СРЕДНЯЯ МОРАЛЬ
            // =====================
            main->morale = moraleSum / moraleCount;
        }

        // =====================
        // ОПРЕДЕЛЕНИЕ ОСАДЫ
        // =====================
        int firstOwner = -1;
        bool conflict = false;

        for (auto *army : cityArmies)
        {
            if (army->soldiers <= 0)
                continue;

            if (firstOwner == -1)
            {
                firstOwner = army->owner;
            }
            else if (army->owner != firstOwner)
            {
                conflict = true;
            }
        }

        // =====================
        // ВЛАДЕЛЕЦ ГОРОДА
        // =====================
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
    // УДАЛЕНИЕ МЁРТВЫХ АРМИЙ
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

void SimulationSystem::resolveBattles(ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr)
{
    for (auto &city : cityMgr.cities)
    {
        auto armiesInCity = armyMgr.getAllInCity(city.id);

        if (armiesInCity.size() < 2)
            continue;

        Army* armyA = armiesInCity[0];
        Army* armyB =  armiesInCity[1];

        // надо чтобы A-0 plr, B-1 ii, если не тот, меняем
        if (armyA->owner==1)
        {
            armyA = armiesInCity[1];
            armyB = armiesInCity[0];
        }


        int powerA = armyA->soldiers;
        int powerB = armyB->soldiers;

        // =====================
        // коэффициенты
        // =====================
        float siegeFactor = (city.owner == -1) ? 0.4f : 1.0f;
        float lossFactor  = 0.5f;

        int lossesA = std::min(powerA, (int)(powerB * lossFactor * siegeFactor));
        int lossesB = std::min(powerB, (int)(powerA * lossFactor * siegeFactor));

        lossesA *= armyB->morale;
        if (lossesA<30) lossesA = 30;
        lossesB *= armyA->morale;
        if (lossesB<30) lossesB = 30;


        // =====================
        // применяем потери
        // =====================
        float moraleLossA =  (float)lossesA / ((float)powerA + 1) * (float)lossesA / ((float)powerA + 1) * 0.1f;
        float moraleLossB =  (float)lossesB / ((float)powerB + 1) * (float)lossesB / ((float)powerB + 1) * 0.1f;

        armyA->morale -= moraleLossA;
        if (armyA->morale< 0.4f) armyA->morale = 0.4f;
        armyB->morale -= moraleLossB;
        if (armyB->morale< 0.4f) armyB->morale = 0.4f;


        armyA->soldiers -= lossesA;
        armyB->soldiers -= lossesB;

        animMgr.spawnNumber(city.position, lossesA, 25.f); // plr
        animMgr.spawnNumber(city.position, lossesB, -100.f); // ii


        // =====================
        // результат боя
        // =====================
        if (armyA->soldiers > 0 && armyB->soldiers > 0)
        {
            city.owner = -1; // бой продолжается
        }
        else if (armyA->soldiers > 0)
        {
            city.owner = armyA->owner;
            animMgr.spawnWin(city.position + sf::Vector2f(0, 30));
        }
        else if (armyB->soldiers > 0)
        {
            city.owner = armyB->owner;
            animMgr.spawnLose(city.position + sf::Vector2f(0, 30));
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


// ===================== игровой эээйй аай ============================

// эта дурында делает:
/*
1. найти все и вся (фронты и города)
2. Сгенерировать команды:
   - поддержка фронтов (пока только поддержка. надо дополнить)
   - атаки по городам (обработка ситуаций при осаде (отступление с города и помощь), экспансия, захват города с боем (оценка рисков))
   - анализ входящих атак и формирование фронтов (ПОКА НЕ РЕАЛИЗОВАНО)
3. Посчитать score каждой команды
4. Отсортировать
5. Взять топ-3
6. выполнить
*/
void SimulationSystem::makeAITurns(CommandManager& commandMgr, ArmyManager& armyMgr, CityManager& cityMgr, AnimationManager& animMgr)
{
    struct AICmd
    {
        int armyId;
        int fromCity;
        int toCity;
        int sendUnits;
        float score;

        enum Type { AttackCity, ReinforceFront } type;

        float morale;
    };

    std::vector<AICmd> candidates;

    // =====================
    // ФРОНТЫ                   // этот же код используется в endturn, не хочу выносить, потому что хочу локально поработать с анализом и отдельно с изменениями
    // =====================
    struct Front
    {
        int a;
        int b;
        std::vector<Command*> cmds;
    };

    std::vector<Front> fronts;

    auto getFront = [&](int a, int b) -> Front*
    {
        for (auto &f : fronts)
        {
            if ((f.a == a && f.b == b) || (f.a == b && f.b == a))
                return &f;
        }

        fronts.push_back({a, b, {}});
        return &fronts.back();
    };

    for (auto &cmd : commandMgr.commands)
    {
        if (cmd.state == CommandState::InRetreat) continue;

        Front* f = getFront(cmd.fromCity, cmd.toCity);
        f->cmds.push_back(&cmd);
    }

    // =====================
    // REINFORCE FRONT (1 команда на фронт)
    // =====================
    for (auto &f : fronts)
    {
        // считаем силы
        int allyUnits = 0;
        int enemyUnits = 0;

        for (auto *cmd : f.cmds)
        {
            if (cmd->units <= 0) continue;

            if (cmd->owner == 1) allyUnits += cmd->units;
            else enemyUnits += cmd->units;
        }
        // надо нет вообше лезть
        int need = enemyUnits - allyUnits;

        if (need <= 0) continue;

        AICmd bestCmd;
        float bestScore = -999999.f;
        bool found = false;

        // смотрим подходящие армии ищем для помощи
        for (auto &army : armyMgr.armies)
        {
            if (army.owner != 1) continue;
            if (army.soldiers < 10) continue;

            City* from = cityMgr.findById(army.currentCityId);
            if (!from) continue;

            int distA = cityMgr.distance(from->id, f.a);
            int distB = cityMgr.distance(from->id, f.b);

            int targetCity = (distA < distB) ? f.a : f.b;
            int distance = std::min(distA, distB);

            if (!cityMgr.canMoveBetweenCities(from->id, targetCity, 1)) continue;

            // пока отправляем столько юнитов
            int sendUnits = std::min(army.soldiers, std::max(10, static_cast<int>(std::round(need*1.2f))));
            if (sendUnits <= 0) continue;

            float score = 0.f;

            // приоритет фронта
            score += 100.f;

            // насколько плохо на фронте
            score += (float)need * 0.5f;

            // полезность армии
            float usefulness = (float)sendUnits / (need + 1);
            score += usefulness * 60.f;

            // слабая армия
            if (sendUnits < need * 0.3f)
                score -= 80.f;

            if (score > bestScore)
            {
                bestScore = score;
                bestCmd = {
                    army.id,
                    from->id,
                    targetCity,
                    sendUnits,
                    score,
                    AICmd::ReinforceFront,
                    army.morale
                };
                found = true;
            }
        }

        if (found)
            candidates.push_back(bestCmd);
    }

    // =====================
    // ATTACK CITY
    // =====================
    for (auto &army : armyMgr.armies)
    {
        if (army.owner != 1) continue;
        if (army.soldiers < 10) continue;

        City* from = cityMgr.findById(army.currentCityId);
        if (!from) continue;


        // среди соседей смотрим
        for (int neighbor : from->neighbors)
        {
            // как же без проверок
            City* target = cityMgr.findById(neighbor);
            if (!target) continue;

            if (!cityMgr.canMoveBetweenCities(from->id, neighbor, 1))
                continue;

            if (commandMgr.isDuplicateInTurn(army.id, from->id, neighbor))
                continue;

            int distance = cityMgr.distance(from->id, neighbor);

            float score = 0.f;
            int sendUnits = 0;

            // НО ПРЕЖДЕ ВСЕГО надо обработать осаду
            if (from->owner == -1)
            {
                auto armiesInBattleCity = armyMgr.getAllInCity(target->id);
                int enemyUnits = 0;
                int allyUnits = 0;
                for (auto *a : armiesInBattleCity)
                {
                    if (a->owner == army.owner)
                        allyUnits += a->soldiers;
                    else
                        enemyUnits += a->soldiers;
                }
                // сюда тоже dir бонус, что отступать лучше назад
                // =========================== риски =========================
                if (allyUnits < enemyUnits * 0.4f) {score += 60.f; sendUnits=allyUnits*0.8f;}; // сильно проигрываем - лучше отступить
                if (allyUnits > enemyUnits) {score -= 400.f; sendUnits=0;}; // у тебя город в осаде и ты выйгрываешь, куда собсветнно собрался

                // добавляем приоритет, если отступаем на свои земли
                bool isEnemyCity = (target->owner != army.owner);
                //
                if (isEnemyCity) score -= 40.f;
                else score += 10.f;

            }
            else // если не в осде - в атаку!
            {
                //
                // ПРИОРИТЕТЫ
                //
                //=========== 1. Направление атаки ( на москву ) - вправо========
                City* fromCityPtr = from;
                City* toCityPtr   = cityMgr.findById(neighbor);

                float dx = 0.f;
                if (fromCityPtr && toCityPtr)
                {
                    dx = (float)(toCityPtr->position.x - fromCityPtr->position.x);
                }
                // ограничиваем влияние, чтобы не раздувало score
                float directionBonus = std::clamp(dx * 0.05f, -50.f, 40.f);
                score += directionBonus;

                // ============ 2. куда идем, на врага или так ===================
                // добавляем приоритет, если идем на врага
                bool isEnemyCity = (target->owner != army.owner);

                if (isEnemyCity)
                    score += 35.f;
                else
                    score += 10.f;


                // ================= 3. оценка сил ===========================
                auto armiesInCity = armyMgr.getAllInCity(target->id);

                // считаем силы
                int enemyUnits = 0;
                int allyUnits = 0;
                for (auto *a : armiesInCity)
                {
                    if (a->owner == army.owner)
                        allyUnits += a->soldiers;
                    else
                        enemyUnits += a->soldiers;
                }
                // на врага идти выгоднее
                if (enemyUnits > 0)
                {
                    int targetUnits = enemyUnits - allyUnits;
                    sendUnits = targetUnits * 1.2f;
                    sendUnits = std::clamp(sendUnits, 10, army.soldiers);
                }
                else
                {
                    // экспансия
                    sendUnits = army.soldiers;
                    score -= 40.f;

                    for (int neighborId : from->neighbors) // спишем по 30 очков за экспанисию, если рядом бои
                    {
                        City *c = cityMgr.findById(neighborId);
                        if (c->owner == -1) score -= 30.f;
                    }
                }

                // риски при поддержке города
                if (allyUnits * 1.5f < enemyUnits) score += 100.f; // города нужно удерживать


                // риски при нападении на город

                int ourTotal = sendUnits + allyUnits;

                // потенциально проигрываем
                if (ourTotal < enemyUnits) score += 25.f;


                // сильно проигрываем - игра не стоит свеч пупупу увы и ах
                if (ourTotal < enemyUnits * 0.3f)
                    score -= 80.f;

                // потенциальная победа
                if (ourTotal > enemyUnits * 1.1f)
                    score += 35.f;

                // ======================= 4. лучше двигать большие армии ===================
                float armyPower = (float)army.soldiers;
                float powerBonus = std::log(armyPower + 1.f) * 15.f;

                score += powerBonus;

            }
            if (sendUnits <= 0) continue;
            if (neighbor==30) score-=1000.f;

            candidates.push_back({
                army.id,
                from->id,
                neighbor,
                sendUnits,
                score,
                AICmd::AttackCity,
                army.morale
            });
        }
    }

    std::cout << "============" << std::endl;
    int place = 0;
    for (auto &cmd : candidates)
    {
        place++;
        std::cout << "place " << place << std::endl;

        std::cout << "command" << std::endl;
        std::cout << "from " << cityMgr.findById(cmd.fromCity)->name << std::endl;
        std::cout << "to " << cityMgr.findById(cmd.toCity)->name << std::endl;
        std::cout << "units " << cmd.sendUnits << std::endl;
        std::cout << "type " << cmd.type << std::endl;
        std::cout << "points " << cmd.score << std::endl;
        std::cout << std::endl;
    }


    // =====================
    // ФИЛЬТР ( пока на одну армию одно действие) надо потом пофиксить потому что нет много задачности как у игррока
    // ====================
    std::unordered_map<int, AICmd> bestPerArmy;

    for (auto &cmd : candidates)
    {
        auto it = bestPerArmy.find(cmd.armyId);

        if (it == bestPerArmy.end() || cmd.score > it->second.score)
        {
            bestPerArmy[cmd.armyId] = cmd;
        }
    }

    candidates.clear();

    for (auto &[id, cmd] : bestPerArmy)
    {
        candidates.push_back(cmd);
    }


    // =====================
    // SORT
    // =====================
    std::sort(candidates.begin(), candidates.end(),
        [](const AICmd& a, const AICmd& b)
        {
            return a.score > b.score;
        });


    // =====================
    // EXECUTE TOP 3
    // =====================
    int executed = 0;

    for (auto &cmdAI : candidates)
    {
        if (executed >= 3) break;

        int turns = cityMgr.distance(cmdAI.fromCity, cmdAI.toCity);

        commandMgr.commands.push_back({
            1,
            cmdAI.armyId,
            cmdAI.fromCity,
            cmdAI.toCity,
            cmdAI.sendUnits,
            turns,
            turns,
            0,
            1,
            CommandState::Animating,
            cmdAI.morale
        });

        commandMgr.updateOffsets(cmdAI.fromCity, cmdAI.toCity);

        Command cmd = commandMgr.commands.back();

        City* a = cityMgr.findById(cmd.fromCity);
        City* b = cityMgr.findById(cmd.toCity);

        auto curve = cityMgr.buildCurve(a->position, b->position, cmd.offset);

        animMgr.spawnLineAppear(curve, sf::Color::Blue);

        executed++;
    }
}
