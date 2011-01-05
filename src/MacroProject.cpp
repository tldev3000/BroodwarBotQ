#include "MacroProject.h"
#include <Util.h>
#include <time.h>
#include <UnitsGroup.h>
#ifdef BW_QT_DEBUG
#include <QtGui/QApplication.h>
#endif

#define __POOL_TIME_RUSH__ 130 // seconds, 3 workers + 1 pool + 11 seconds
#define __BBS_TIME_RUSH__ 230 // seconds, 4 workers + 2 barracks + 18 seconds
#define __GATES_TIME_RUSH__ 190 // seconds, 4 workers + 2 gateways + 18 seconds

using namespace BWAPI;

// TODO TODO TODO: work with accessibility while making it false when onUnitEvade

BattleBroodAI::BattleBroodAI()
{
    /*std::set<UnitType>& aut = BWAPI::UnitTypes::allUnitTypes();
    for (std::set<UnitType>::iterator wu = aut.begin(); wu != aut.end(); wu++)
        log( "%s requieres %i %s", wu->getName().c_str(), wu->whatBuilds().second, wu->whatBuilds().first.getName().c_str());

    log( "\n\n\n\n\n");

    for (std::set<UnitType>::iterator wu = aut.begin(); wu != aut.end(); wu++)
        for(std::map<UnitType, int >::const_iterator it = wu->requiredUnits().begin(); it != wu->requiredUnits().end(); it++)
            log("create %s requieres %s", wu->getName().c_str(), it->first.getName().c_str());

    log( "\n\n\n\n\n");

    for (std::set<UnitType>::iterator wu = aut.begin(); wu != aut.end(); wu++)
        if( wu->isBuilding() && !wu->isResourceDepot() && wu->canProduce())
            log( "%s is a prod building.", wu->getName().c_str());

    log( "\n\n\n\n\n");

    for (std::set<UnitType>::iterator wu = aut.begin(); wu != aut.end(); wu++)
        if( wu->isBuilding() && !wu->isResourceDepot() && !wu->canProduce() && wu->canAttack())
            log( "%s is a def building.", wu->getName().c_str());

    log( "\n\n\n\n\n");

    for (std::set<UnitType>::iterator wu = aut.begin(); wu != aut.end(); wu++)
        if( !wu->isBuilding() && !wu->isWorker())
            log( "%s is a unit.", wu->getName().c_str());

    log( "\n\n\n\n\n");

    for (std::set<UnitType>::iterator wu = aut.begin(); wu != aut.end(); wu++)
        if( !wu->isBuilding() && !wu->isWorker())
            log( "%s has a range of %i.", wu->getName().c_str(), wu->seekRange());*/

}

BattleBroodAI::~BattleBroodAI()
{
#ifdef BW_QT_DEBUG
    if (qapplication)
        qapplication->quit();
#endif
    if( Broodwar->self()->getRace() == Races::Protoss)
        ProtossStrat::Destroy();
    else if( Broodwar->self()->getRace() == Races::Terran)
        TerranStrat::Destroy();
    else if( Broodwar->self()->getRace() == Races::Zerg)
        ZergStrat::Destroy();

    // reverse order ;)
#ifdef __DEBUG__
    EnhancedUI::Destroy();
#endif
    GoalManager::Destroy();
    TimeManager::Destroy();
    ETechEstimator::Destroy();
    EEcoEstimator::Destroy();
    EUnitsFilter::Destroy();
    WarManager::Destroy();
    WorkerManager::Destroy();
    MapManager::Destroy();
    ScoutManager::Destroy();
    InformationManager::Destroy();
    UnitGroupManager::Destroy();
    UpgradeManager::Destroy();
    TechManager::Destroy();
    SupplyManager::Destroy();
    BuildOrderManager::Destroy();
    ProductionManager::Destroy();
    MorphManager::Destroy();
    ConstructionManager::Destroy();
    BuildManager::Destroy();
    BuildingPlacer::Destroy();
    BorderManager::Destroy();
    BaseManager::Destroy();
    Arbitrator::Arbitrator<BWAPI::Unit*,double>::Destroy();
    ObjectManager::Destroy();
    delete this->workerSelfDefenseManager;
}

void BattleBroodAI::onStart()
{
    //Broodwar->printf("The map is %s, a %d player map",Broodwar->mapName().c_str(),Broodwar->getStartLocations().size());
    // Enable some cheat flags
#ifdef __DEBUG__
    Broodwar->enableFlag(Flag::UserInput);
    Broodwar->setLocalSpeed(0);
#endif
    // Uncomment to enable complete map information
    //Broodwar->enableFlag(Flag::CompleteMapInformation);

    // Speed up the game to the maximum

    this->showManagerAssignments=false;

    if (Broodwar->isReplay())
    {
        //	Broodwar->printf("The following players are in this replay:");
        for(std::set<Player*>::iterator p=Broodwar->getPlayers().begin();p!=Broodwar->getPlayers().end();p++)
        {
            if (!(*p)->getUnits().empty() && !(*p)->isNeutral())
            {
                //	Broodwar->printf("%s, playing as a %s",(*p)->getName().c_str(),(*p)->getRace().getName().c_str());
            }
        }
        return;
    }

    // Enable some cheat flags
    //Broodwar->enableFlag(Flag::UserInput);
    //Broodwar->enableFlag(Flag::CompleteMapInformation);
    BWTA::readMap();
    BWTA::analyze();
    this->analyzed=true;

    this->objManager = & ObjectManager::Instance();
    this->arbitrator = static_cast< Arbitrator::Arbitrator<BWAPI::Unit*,double>* >(& Arbitrator::Arbitrator<BWAPI::Unit*,double>::Instance());
    this->baseManager = & BaseManager::Instance();
    this->borderManager = & BorderManager::Instance();
    this->buildingPlacer = & BuildingPlacer::Instance();
    this->buildManager = & BuildManager::Instance();
    this->constructionManager = & ConstructionManager::Instance();
    this->morphManager = & MorphManager::Instance();
    this->productionManager = & ProductionManager::Instance();
    this->buildOrderManager = & BuildOrderManager::Instance();
    this->supplyManager = & SupplyManager::Instance();
    this->techManager = & TechManager::Instance();
    this->upgradeManager = & UpgradeManager::Instance();
    this->unitGroupManager = & UnitGroupManager::Instance();
    this->informationManager = & InformationManager::Instance();
    this->scoutManager = & ScoutManager::Instance();
    this->mapManager = & MapManager::Instance();
    this->workerManager = & WorkerManager::Instance();
    this->warManager = & WarManager::Instance();
    this->eUnitsFilter = & EUnitsFilter::Instance();
    this->eEcoEstimator = & EEcoEstimator::Instance();
    this->eTechEstimator = & ETechEstimator::Instance();
    this->timeManager = & TimeManager::Instance();
    this->goalManager = & GoalManager::Instance();
    this->defenseManager = & DefenseManager::Instance();
    this->workerSelfDefenseManager = new WorkerSelfDefenseManager(static_cast< Arbitrator::Arbitrator<BWAPI::Unit*,double>* >(& Arbitrator::Arbitrator<BWAPI::Unit*,double>::Instance()));
#ifdef __DEBUG__
    this->enhancedUI = & EnhancedUI::Instance();
#endif
    if( Broodwar->self()->getRace() == Races::Protoss)
        this->macroManager = & ProtossStrat::Instance();
    else if( Broodwar->self()->getRace() == Races::Terran)
        this->macroManager = & TerranStrat::Instance();
    else if( Broodwar->self()->getRace() == Races::Zerg)
        this->macroManager = & ZergStrat::Instance();

    //Set dependencies
    this->baseManager->setDependencies();
    this->borderManager->setDependencies();
    this->buildManager->setDependencies();
    this->constructionManager->setDependencies();
    this->morphManager->setDependencies();
    this->productionManager->setDependencies();
    this->buildOrderManager->setDependencies();
    this->supplyManager->setDependencies();
    this->techManager->setDependencies();
    this->upgradeManager->setDependencies();
    this->scoutManager->setDependencies();
    this->mapManager->setDependencies();
    this->workerManager->setDependencies();
    this->warManager->setDependencies();
    this->eEcoEstimator->setDependencies();
    this->goalManager->setDependencies();
    this->macroManager->setDependencies();
    this->defenseManager->setDependencies();

    this->baseManager->update();
#ifdef BW_QT_DEBUG
    g_onStartDone = true;
#endif

    //Call on start functions
    this->macroManager->onStart();
    this->warManager->onStart();
}

void BattleBroodAI::onEnd(bool isWinner)
{
    if (isWinner)
    {
        //log win to file
    }
}

void BattleBroodAI::onFrame()
{
#ifdef __DEBUG__
    clock_t start = clock();
#endif
    if (Broodwar->isReplay()) return;
    if (!this->analyzed) return;
#ifdef __DEBUG__
    if (Broodwar->getLastError() != BWAPI::Errors::None)
        Broodwar->printf("LAST ERROR: %s", Broodwar->getLastError().toString().c_str());
#endif

#ifdef BW_POS_MOUSE
    char mousePos[100];
    sprintf_s(mousePos, "%d, %d", Broodwar->getMousePosition().x(), Broodwar->getMousePosition().y());
    Broodwar->drawTextMouse(12, 0, mousePos);
#endif
   
    this->objManager->onFrame();
#ifdef __DEBUG__
    clock_t end = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("onFrame() took: %2.5f seconds\n", duration);
    clock_t end2 = clock();
#endif

    this->baseManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("BaseManager took: %2.5f seconds\n", duration);
#endif
    this->borderManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("BorderManager took: %2.5f seconds\n", duration);
#endif
    this->buildManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("BuildManager took: %2.5f seconds\n", duration);
#endif
    this->constructionManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("ConstructionManager took: %2.5f seconds\n", duration);
#endif
    this->morphManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("MorphManager took: %2.5f seconds\n", duration);
#endif
    this->productionManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("ProductionManager took: %2.5f seconds\n", duration);
#endif
    this->buildOrderManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("BuildOrderManager took: %2.5f seconds\n", duration);
#endif
    this->supplyManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("SupplyManager took: %2.5f seconds\n", duration);
#endif
    this->techManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("TechManager took: %2.5f seconds\n", duration);
#endif
    this->upgradeManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("UpgradeManager took: %2.5f seconds\n", duration);
#endif
    this->scoutManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("ScoutManager took: %2.5f seconds\n", duration);
#endif
    this->mapManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("mapManager took: %2.5f seconds\n", duration);
#endif
    this->workerManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("workerManager took: %2.5f seconds\n", duration);
#endif
    this->warManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("WarManager took: %2.5f seconds\n", duration);
#endif
    this->eUnitsFilter->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("EUnitsFilter took: %2.5f seconds\n", duration);
#endif
    this->eEcoEstimator->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("EEcoEstimator took: %2.5f seconds\n", duration);
#endif
    this->timeManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("TimeManager took: %2.5f seconds\n", duration);
#endif
    this->macroManager->update();
#ifdef __DEBUG__
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("MacroManager took: %2.5f seconds\n", duration);
#endif
    this->defenseManager->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("DefenseManager took: %2.5f seconds\n", duration);
#endif
    this->workerSelfDefenseManager->update();
#ifdef __DEBUG__
    //this->enhancedUI->update();
    end2 = clock();
    duration = (double)(end2 - end) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("EnhancedUI took: %2.5f seconds\n", duration);
#endif

    static_cast< Arbitrator::Arbitrator<BWAPI::Unit*,double>* >(arbitrator)->update();
#ifdef __DEBUG__
    end = clock();
    duration = (double)(end - end2) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("Arbitrator took: %2.5f seconds\n", duration);
#endif

    std::set<Unit*> units=Broodwar->self()->getUnits();
    if (this->showManagerAssignments)
    {
        for(std::set<Unit*>::iterator i=units.begin();i!=units.end();i++)
        {
            if (this->arbitrator->hasBid(*i))
            {
                int x=(*i)->getPosition().x();
                int y=(*i)->getPosition().y();
                std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> > bids=this->arbitrator->getAllBidders(*i);
                int y_off=0;
                bool first = false;
                const char activeColor = '\x07', inactiveColor = '\x16';
                char color = activeColor;
                for(std::list< std::pair< Arbitrator::Controller<BWAPI::Unit*,double>*, double> >::iterator j=bids.begin();j!=bids.end();j++)
                {
                    Broodwar->drawTextMap(x,y+y_off,"%c%s: %d",color,j->first->getShortName().c_str(),(int)j->second);
                    y_off+=15;
                    color = inactiveColor;
                }
            }
        }
    }

#ifdef __DEBUG__
    display();
    end = clock();
    duration = (double)(end - start) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf("onFrame() took: %2.5f seconds\n", duration);
#endif
    //clock_t end = clock();
    //Broodwar->printf("Iterations took %f", (double)(end-start)/CLOCKS_PER_SEC);
}

void BattleBroodAI::onUnitCreate(BWAPI::Unit* unit)
{
    this->scoutManager->onUnitCreate(unit);
    this->mapManager->onUnitCreate(unit);
    this->warManager->onUnitCreate(unit);
    this->macroManager->onUnitCreate(unit);

    if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus) // provisory
        macroManager->nexuses.insert(unit);
    else if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway)
        macroManager->gateways.insert(unit);
    else if (unit->getType() == BWAPI::UnitTypes::Protoss_Forge)
        macroManager->forges.insert(unit);
}

void BattleBroodAI::onUnitDestroy(BWAPI::Unit* unit)
{
    this->arbitrator->onRemoveObject(unit);
    this->buildManager->onRemoveUnit(unit);
    this->baseManager->onRemoveUnit(unit);
    this->techManager->onRemoveUnit(unit);
    this->upgradeManager->onRemoveUnit(unit);
    this->workerManager->onRemoveUnit(unit);
    this->macroManager->onUnitDestroy( unit);
    this->warManager->onUnitDestroy(unit);
    this->mapManager->onUnitDestroy(unit);
    this->eUnitsFilter->onUnitDestroy(unit);
    this->informationManager->onUnitDestroy(unit);
    this->defenseManager->onRemoveUnit(unit);
    this->workerSelfDefenseManager->onRemoveUnit(unit);
    this->scoutManager->onUnitDestroy(unit);

    if (unit->getType() == BWAPI::UnitTypes::Protoss_Nexus) // provisory
        macroManager->nexuses.erase(unit);
    else if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway)
        macroManager->gateways.erase(unit);
    else if (unit->getType() == BWAPI::UnitTypes::Protoss_Forge)
        macroManager->forges.erase(unit);
}

void BattleBroodAI::onUnitShow(BWAPI::Unit* unit)
{
    mapManager->onUnitShow(unit);
    eUnitsFilter->update(unit);
    scoutManager->onUnitShow(unit);
    if (unit->getPlayer() == BWAPI::Broodwar->enemy())
    {
        if ((unit->getType() == UnitTypes::Zerg_Spawning_Pool && Broodwar->getFrameCount() < __POOL_TIME_RUSH__ && !unit->getRemainingBuildTime())
            || (unit->getType() == UnitTypes::Protoss_Gateway && Broodwar->getFrameCount() < __GATES_TIME_RUSH__ && !unit->getRemainingBuildTime() && eUnitsFilter->getNumbersType(unit->getType()) > 1)
            || (unit->getType() == UnitTypes::Terran_Barracks && Broodwar->getFrameCount() < __BBS_TIME_RUSH__ && !unit->getRemainingBuildTime() && eUnitsFilter->getNumbersType(unit->getType()) > 1))
            this->macroManager->eRush();
        if ((unit->getType() == UnitTypes::Zerg_Spire) || (unit->getType() == UnitTypes::Terran_Control_Tower) || (unit->getType() == UnitTypes::Protoss_Stargate))
            this->macroManager->someAir();
    }
    if (!unit->isDetected())
    {
        this->macroManager->needed[Observer] = 3;
        this->macroManager->priority[Observer] = 90;
        this->buildOrderManager->build(3, UnitTypes::Protoss_Observer, 100);
    }
}

void BattleBroodAI::onUnitHide(BWAPI::Unit* unit)
{
    mapManager->onUnitHide(unit);
    eUnitsFilter->update(unit);
}

void BattleBroodAI::onUnitDiscover(BWAPI::Unit* unit)
{
    this->informationManager->onUnitDiscover(unit);
    this->unitGroupManager->onUnitDiscover(unit);
}

void BattleBroodAI::onUnitEvade(BWAPI::Unit* unit){
    this->informationManager->onUnitEvade(unit);
    this->unitGroupManager->onUnitEvade(unit);
    this->arbitrator->onRemoveObject(unit);
}

void BattleBroodAI::onUnitMorph(BWAPI::Unit* unit)
{
    unitGroupManager->onUnitMorph(unit);
    eUnitsFilter->update(unit);
}

void BattleBroodAI::onUnitRenegade(BWAPI::Unit* unit)
{
    unitGroupManager->onUnitRenegade(unit);
    eUnitsFilter->update(unit);
}

void BattleBroodAI::onPlayerLeft(BWAPI::Player* player)
{
    //Broodwar->sendText("%s left the game.",player->getName().c_str());
}

void BattleBroodAI::onNukeDetect(BWAPI::Position target)
{
    //if (target!=Positions::Unknown)
    //	Broodwar->printf("Nuclear Launch Detected at (%d,%d)",target.x(),target.y());
    //else
    //	Broodwar->printf("Nuclear Launch Detected");
}

void BattleBroodAI::onSendText(std::string text)
{
    if (text=="/show players")
    {
        showPlayers();
        return;
    } else if (text=="/show forces")
    {
        showForces();
        return;
    } else if (text=="/show visibility")
    {
        show_visibility_data=true;
    } else if (text=="/analyze")
    {
        if (analyzed == false)
        {
            // Broodwar->printf("Analyzing map... this may take a minute");
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)AnalyzeThread, NULL, 0, NULL);
        }
        return;
    }
    else if (text.substr(0,9) == "/setspeed")
    {
        int x = 0;
        if (text[9] != ' ')
            Broodwar->printf("Usage: /setspeed SPEEDVALUE");
        std::string xx = text.substr(10);
        x = atoi(xx.c_str());
        //  Broodwar->printf("Set Speed %i", x);
        Broodwar->setLocalSpeed(x);
    }
    else if (text=="/target")
    {
        set<pBayesianUnit> tmp;
        // for (std::list<UnitsGroup*>::iterator it = this->warManager->unitsGroups.begin(); it != this->warManager->unitsGroups.end(); it++) // why HEAD does have a different warManager->unitsGroups? @merge
        for (std::list<UnitsGroup*>::iterator it = this->warManager->unitsGroups.begin(); it != this->warManager->unitsGroups.end(); it++)
        {
            set<pBayesianUnit> tmp;
            (*it)->selectedUnits(tmp);
            if (tmp.size() == 0)
                //    Broodwar->printf("No selected units");
                for (std::set<pBayesianUnit>::const_iterator i = tmp.begin(); i != tmp.end(); ++i)
                {
                    //  Broodwar->printf("Target de l'unit� : (%i, %i)", (*i)->target.x(), (*i)->target.y());
                    Position pos(Broodwar->getScreenPosition() + Broodwar->getMousePosition());
                    // Broodwar->printf("Curseur : (%i, %i)", pos.x(), pos.y());
                }
        }
    }

    UnitType type=UnitTypes::getUnitType(text);
    if (text=="debug")
    {
        this->showManagerAssignments=true;
        return;
    }
    if (type!=UnitTypes::Unknown)
    {
        this->buildManager->build(type);
    }
    else
    {
        TechType type=TechTypes::getTechType(text);
        if (type!=TechTypes::Unknown)
        {
            this->techManager->research(type);
        }
        else
        {
            UpgradeType type=UpgradeTypes::getUpgradeType(text);
            if (type!=UpgradeTypes::Unknown)
            {
                this->upgradeManager->upgrade(type);
            }
            else
                Broodwar->printf("You typed '%s'!",text.c_str());
        }
    }
    return;
}

DWORD WINAPI AnalyzeThread()
{
    BWTA::analyze();
    analyzed   = true;
    analysis_just_finished = true;

    //self start location only available if the map has base locations
    if (BWTA::getStartLocation(BWAPI::Broodwar->self())!=NULL)
    {
        home = BWTA::getStartLocation(BWAPI::Broodwar->self())->getRegion();
    }
    //enemy start location only available if Complete Map Information is enabled.
    if (BWTA::getStartLocation(BWAPI::Broodwar->enemy())!=NULL)
    {
        enemy_base = BWTA::getStartLocation(BWAPI::Broodwar->enemy())->getRegion();
    }
    return 0;
}

void BattleBroodAI::drawStats()
{
    std::set<Unit*> myUnits = Broodwar->self()->getUnits();
    Broodwar->drawTextScreen(5,0,"I have %d units:",myUnits.size());
    std::map<UnitType, int> unitTypeCounts;
    for(std::set<Unit*>::iterator i=myUnits.begin();i!=myUnits.end();i++)
    {
        if (unitTypeCounts.find((*i)->getType())==unitTypeCounts.end())
        {
            unitTypeCounts.insert(std::make_pair((*i)->getType(),0));
        }
        unitTypeCounts.find((*i)->getType())->second++;
    }
    int line=1;
    for(std::map<UnitType,int>::iterator i=unitTypeCounts.begin();i!=unitTypeCounts.end();i++)
    {
        Broodwar->drawTextScreen(5,16*line,"- %d %ss",(*i).second, (*i).first.getName().c_str());
        line++;
    }
}

void BattleBroodAI::showPlayers()
{
    std::set<Player*> players=Broodwar->getPlayers();
    for(std::set<Player*>::iterator i=players.begin();i!=players.end();i++)
    {
        //   Broodwar->printf("Player [%d]: %s is in force: %s",(*i)->getID(),(*i)->getName().c_str(), (*i)->getForce()->getName().c_str());
    }
}
void BattleBroodAI::showForces()
{
    std::set<Force*> forces=Broodwar->getForces();
    for(std::set<Force*>::iterator i=forces.begin();i!=forces.end();i++)
    {
        std::set<Player*> players=(*i)->getPlayers();
        //  Broodwar->printf("Force %s has the following players:",(*i)->getName().c_str());
        for(std::set<Player*>::iterator j=players.begin();j!=players.end();j++)
        {
            //      Broodwar->printf("  - Player [%d]: %s",(*j)->getID(),(*j)->getName().c_str());
        }
    }
}

// This method is called from onFrame() and allow to display graphical debug informations on each component.
void BattleBroodAI::display()
{
    warManager->display();
}
