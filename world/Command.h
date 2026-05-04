#pragma once

enum class CommandState
{
    Created,
    Activated,
    InBattle,
    InRetreat,
    Animating
};

struct Command {
    int owner;
    int armyId;
    int fromCity;
    int toCity;
    int units;

    int remainingTurns;  // сколько осталось
    int totalTurns;      // сколько ходов всего

    float offset;

    float battleDot = 0;

    CommandState state = CommandState::Created;
};
