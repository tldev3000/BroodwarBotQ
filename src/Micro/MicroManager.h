#pragma once
#define __DEBUG_NICOLAS__
#include <BWAPI.h>
#include <BWTA.h>
#include <windows.h>
//#include "Goals.h"
#include <CSingleton.h>
#include <Arbitrator.h>
#include <stdlib.h>

class UnitsGroup;
class Regions;
class ScoutManager;

class MicroManager: public CSingleton<MicroManager>, public Arbitrator::Controller<BWAPI::Unit*,double>
{
	friend class CSingleton<MicroManager>;

private:
	MicroManager();
	~MicroManager();
	std::list<UnitsGroup *> promptedRemove;
	bool remove(UnitsGroup* u);
public:
	std::list<UnitsGroup*> unitsgroups;
	Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
	Regions* regions;

	virtual void onOffer(std::set<BWAPI::Unit*> units);
	virtual void onRevoke(BWAPI::Unit* unit, double bid);
	virtual std::string getName() const;
	virtual void update();
	void onUnitCreate(BWAPI::Unit* unit);
	void onUnitDestroy(BWAPI::Unit* unit);
	void display();

	void sendGroupToAttack( UnitsGroup* ug);
	void sendGroupToDefense( UnitsGroup* ug);
	void promptRemove(UnitsGroup* ug);
};
