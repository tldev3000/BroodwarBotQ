#pragma once
#include <PrecompiledHeader.h>
#include "Intelligence/EUnitsFilter.h"
#include "Intelligence/ETechEstimator.h"
#include "Macro/InformationManager.h"
#include "Regions/MapManager.h"
#include "Intelligence/FirstScoutController.h"
#include "Defines.h"

class Intelligence : public CSingleton<Intelligence>
{
	friend class CSingleton<Intelligence>;
	Intelligence();
	~Intelligence();
public:
	EUnitsFilter* eUnitsFilter;
	ETechEstimator* eTechEstimator;
	MapManager* mapManager;
	FirstScoutController* firstScout;
	void update();
    void onUnitCreate(BWAPI::Unit* u);
    void onUnitDestroy(BWAPI::Unit* u);
    void onUnitShow(BWAPI::Unit* u);
    void onUnitHide(BWAPI::Unit* u);
	void onUnitMorph(BWAPI::Unit* u);
	void onUnitRenegade(BWAPI::Unit* u);
};