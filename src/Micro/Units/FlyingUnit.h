#pragma once

#include "Micro/Units/BayesianUnit.h"

class FlyingUnit : public BayesianUnit
{
public:
	FlyingUnit(BWAPI::Unit* u);
	virtual ~FlyingUnit();
	virtual void micro() = 0;
	virtual void check() = 0;
	virtual int getAttackDuration() = 0;
	virtual std::set<BWAPI::UnitType> getSetPrio() = 0;
private:
	static ProbTables _sProbTables;
};
