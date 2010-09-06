#include <BWTA.h>
#include <ScoutManager.h>
#include "Defines.h"

using namespace BWAPI;
ScoutManager::ScoutManager( )
:
exploringEnemy(false)
{
	regions = NULL;
	warManager = NULL;
}

ScoutManager::~ScoutManager( )
{
}

void ScoutManager::setDependencies(){
	this->regions = & Regions::Instance();
	this->warManager = & WarManager::Instance();
	this->arbitrator = &  Arbitrator::Arbitrator<BWAPI::Unit*,double>::Instance();
}

void ScoutManager::update()
{
	
	if(regions->enemyFound() && ! exploringEnemy ){
		
		exploringEnemy = true;
		this->awaitingGoals.push_back(pGoal(new ExploreGoal(regions->whereIsEnemy())));
	}

	std::set<UnitsGroup *> toTrash;
	//Free units that need to be freed and add goals to other
	for(std::list<UnitsGroup *>::iterator it = this->myUnitsGroups.begin(); it != this->myUnitsGroups.end(); ++it ){
		
		if((*it)->isWaiting()){

			//If no new goal available release the ug : 
			if(this->awaitingGoals.size() <= 0){
				for(std::vector<pBayesianUnit>::iterator u = (*it)->units.begin(); u != (*it)->units.end(); ++u){
					this->arbitrator->removeBid(this, (*u)->unit);
				}
				this->warManager->promptRemove(*it);
				toTrash.insert(*it);
			} else {
			//New goal available : assign it to the ug
				(*it)->addGoal(this->awaitingGoals.front());
				this->awaitingGoals.pop_front();
			}
		}
	}

	if(this->awaitingGoals.size() > 0){
		//ask units :
		for(std::set<BWAPI::Unit *>::const_iterator it = BWAPI::Broodwar->self()->getUnits().begin(); it != BWAPI::Broodwar->self()->getUnits().end(); ++it){
			if( (*it)->getType().isWorker() || (*it)->getType() == BWAPI::UnitTypes::Protoss_Observer ){
				this->arbitrator->setBid(this, (*it),90);
			}
		}
	}
	for(std::set<UnitsGroup *>::iterator trash = toTrash.begin(); trash != toTrash.end(); ++trash){
		this->myUnitsGroups.remove(*trash);
	}
	toTrash.empty();

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

}

void ScoutManager::onOffer(std::set<BWAPI::Unit*> units){

	std::vector<pGoal> goalsDone;
	std::set<BWAPI::Unit*> remainingUnits = units;//Does it copy the set ?


	//Else grab a new unit
	//Obs are always the best
	BWAPI::Unit * bestUnit = NULL;
	int dist= 999999999;
	int test = 0;
	UnitsGroup * giveMeTheGoal = NULL;

	for(std::list<pGoal>::iterator goals = this->awaitingGoals.begin(); goals != this->awaitingGoals.end(); ++goals){
	//find the best unit for each goal
		dist = 999999999;
		bestUnit = NULL;

		for(std::set<BWAPI::Unit *>::iterator units = remainingUnits.begin(); units != remainingUnits.end(); ++units ){
					
			if(bestUnit == NULL){
				bestUnit = (*units);
			}
			test = (*goals)->estimateDistance((*units)->getPosition());
			if( test < dist ){
				bestUnit = (*units);
				dist = test;
			}
			if((*units)->getType() == BWAPI::UnitTypes::Protoss_Observer){
				bestUnit = (*units);
				break;
			}
		}
		this->arbitrator->accept(this, bestUnit, 90);
		giveMeTheGoal = new UnitsGroup();
		this->myUnitsGroups.push_back(giveMeTheGoal);
		giveMeTheGoal->takeControl(bestUnit);
		remainingUnits.erase(bestUnit);
	
		//We have a unitsGroup
		(*goals)->setUnitsGroup(giveMeTheGoal);
		giveMeTheGoal->addGoal((*goals));
	
		warManager->unitsGroups.push_back(giveMeTheGoal);
		giveMeTheGoal->switchMode(MODE_SCOUT);
		goalsDone.push_back((*goals));
	}

	for(std::set<BWAPI::Unit *>::iterator it = remainingUnits.begin(); it != remainingUnits.end(); ++it){
		this->arbitrator->decline(this,(*it), 0);
	}

	for(std::vector<pGoal>::const_iterator it = goalsDone.begin(); it != goalsDone.end(); ++it ){
		this->awaitingGoals.remove(*it);
	}

}


void ScoutManager::onRevoke(BWAPI::Unit* unit, double bid){

}

void ScoutManager::findEnemy(){

	this->awaitingGoals.push_back(pGoal(new FindEnemyGoal()));
	//Set bid on all units that can scout for this goal

	for(std::set<BWAPI::Unit *>::const_iterator it = BWAPI::Broodwar->self()->getUnits().begin(); it != BWAPI::Broodwar->self()->getUnits().end(); ++it){
		
		if( (*it)->getType().isWorker() || (*it)->getType() == BWAPI::UnitTypes::Protoss_Observer ){
			this->arbitrator->setBid(this, (*it),90);
		}
	}

}


void ScoutManager::onUnitShow(BWAPI::Unit* unit){

}

void ScoutManager::onUnitDestroy(BWAPI::Unit* unit){
	//Find if a ug is concerned
	for(std::list<UnitsGroup *>::iterator it = myUnitsGroups.begin(); it != myUnitsGroups.end(); ++it){
		if( (*it)->emptyUnits() ){
			this->myUnitsGroups.remove( (*it) );
			break;
		}
	}

}





