#pragma once
#include "Army.h"
#include <vector>

class ArmyManager {
private:
    int nextId = -10000;//////ой вернется мне это, когда система создает армию ей нужен айди
public:
    std::vector<Army> armies;

    int generateId()
    {
        return nextId++;
    }

    Army* getById(int id);      // армия по id
    Army* getInCity(int cityId);// армиия по id города
    std::vector<Army*> getAllInCity(int cityId); // армии по id города

    void init();
};
