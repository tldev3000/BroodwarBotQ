#include <BWTA.h>
#include <ScoutManager.h>
#include "Defines.h"

using namespace BWAPI;
ScoutManager::ScoutManager( )
: BaseObject("ScoutManager")
{
	regions = NULL;
<<<<<<< HEAD
	warManager = NULL;
=======
	microManager = NULL;
>>>>>>> essai
	to_remove = false;
}

ScoutManager::~ScoutManager( )
{
}

<<<<<<< HEAD
void ScoutManager::setDependencies(){
	this->regions = & Regions::Instance();
	this->warManager = & WarManager::Instance();
=======
void ScoutManager::setDependencies(Regions * region, MicroManager * micro){
	this->regions = region;
	this->microManager = micro;
>>>>>>> essai
}

void ScoutManager::update()
{
<<<<<<< HEAD
	//if(regions->enemyFound()){
	//	findUnitsGroup(pGoal(new ExploreGoal(regions->whereIsEnemy())));
	//}
=======
	if(regions->enemyFound()){
		findUnitsGroup(pGoal(new ExploreGoal(regions->whereIsEnemy())));
	}
>>>>>>> essai
}

std::string ScoutManager::getName() const
{
	return "Scout Manager";
}

void ScoutManager::scoutAllEnemies()
{
	// TODO
}

void ScoutManager::counterWall()
{
	// TODO
	// se placer ou construire a l'endroit du wall
	// pour photon ensuite ?
}

void ScoutManager::counterBuild()
{
	// TODO
	// En placant le drone aux endroits de construction.
	// Si terran, attaquer le scv qui construit
}

void ScoutManager::harassWorkers()
{
	// TODO
	// hit and run dans les drones aux min
}

void ScoutManager::checkEmptyXP()
{
	//TODO
}



////////////////////////////NEW SECTION

void ScoutManager::onUnitCreate(BWAPI::Unit* unit){
	//if(BWAPI::Broodwar->self()->supplyUsed() == 10 && unit->getType().isWorker() && !regions->enemyFound()){
	//	findEnemy();
		//exploreRegion( (*BWTA::getRegions().begin()));
	//}
}


void ScoutManager::findEnemy(){
	pGoal goal = pGoal(new FindEnemyGoal());
	findUnitsGroup(goal);
}


void ScoutManager::onUnitShow(BWAPI::Unit* unit){

}


UnitsGroup* ScoutManager::findUnitsGroup(pGoal goal){

		//Find the appropriate UnitsGroup
	
	//Select a worker
	UnitsGroup* ug;	
	double minDist=999999;
	double curDist=0;
	
		//Check over all the already created unitsGroup which one is near the first subgoal to accomplish
		//TODO

		
	//NO unitsgroup already found, must create a new one
	ug = new UnitsGroup();	
	for each(Unit* u in Broodwar->getAllUnits()){
		if (u->getPlayer()==Broodwar->self()&&u->getType().isWorker()&& !(u->isConstructing())){
			ug->takeControl(u);
			break;
		}
	}
	ug->addGoal(goal);
	goal->setUnitsGroup(ug);
<<<<<<< HEAD
	warManager->unitsgroups.push_front(ug);
=======
	microManager->unitsgroups.push_front(ug);
>>>>>>> essai
	return ug;
}


#ifdef BW_QT_DEBUG
QWidget* ScoutManager::createWidget(QWidget* parent) const
{
	return new QLabel(QString("createWidget and refreshWidget undefined for this component."), parent);
}

void ScoutManager::refreshWidget(QWidget* widget) const
{
// TODO update your widget after having defined it in the previous method :)
}
#endif

