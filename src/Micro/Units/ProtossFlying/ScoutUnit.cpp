#include "ScoutUnit.h"

ScoutUnit::ScoutUnit(BWAPI::Unit* u, UnitsGroup* ug):FlyingUnit(u, ug)
{
}

ScoutUnit::~ScoutUnit()
{
}

void ScoutUnit::micro()
{

}

const std::list<BWAPI::UnitType> ScoutUnit::getListePrio() const
{
    return listPriorite;
}