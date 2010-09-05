#pragma once

#include <GroundUnit.h>
#include <BWAPI.h>

class ArchonUnit : public GroundUnit
{
protected:
    static std::set<BWAPI::UnitType> setPrio;
public:
    ArchonUnit(BWAPI::Unit* u, UnitsGroup* ug);
    ~ArchonUnit();
    virtual void micro();
    virtual void check();
    virtual int getAttackDuration();
    virtual std::set<BWAPI::UnitType> getSetPrio();
};