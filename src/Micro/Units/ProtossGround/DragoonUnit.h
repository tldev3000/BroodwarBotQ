#pragma once

#include "Micro/Units/GroundUnit.h"
#include <BWAPI.h>

class DragoonUnit : public GroundUnit
{
protected:
    static std::set<BWAPI::UnitType> setPrio;
    static int addRange;
    static int attackDuration;
    virtual bool decideToFlee();
public:
    DragoonUnit(BWAPI::Unit* u, UnitsGroup* ug);
    ~DragoonUnit();
    virtual int addRangeGround();
    virtual int addRangeAir();
    virtual void micro();
    virtual void check();
    static double getMaxRange();
    virtual int getAttackDuration();
    virtual std::set<BWAPI::UnitType> getSetPrio();
};