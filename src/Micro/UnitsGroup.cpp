#include <PrecompiledHeader.h>
#include <Defines.h>
#include "Micro/UnitsGroup.h"
#include <Position.h>
#include "Utils/Util.h"
#include "Micro/Units/ProtossSpecial/ArbiterUnit.h"
#include "Micro/Units/ProtossSpecial/DarkArchonUnit.h"
#include "Micro/Units/ProtossSpecial/HighTemplarUnit.h"
#include "Micro/Units/ProtossFlying/CarrierUnit.h"
#include "Micro/Units/ProtossFlying/ObserverUnit.h"
#include "Micro/Units/ProtossFlying/ShuttleUnit.h"
#include "Micro/Units/ProtossFlying/CorsairUnit.h"
#include "Micro/Units/ProtossFlying/ScoutUnit.h"
#include "Micro/Units/ProtossGround/ArchonUnit.h"
#include "Micro/Units/ProtossGround/ZealotUnit.h"
#include "Micro/Units/ProtossGround/ReaverUnit.h"
#include "Micro/Units/ProtossGround/ProbeUnit.h"
#include "Micro/Units/ProtossGround/DragoonUnit.h"
#include "Micro/Units/ProtossGround/DarkTemplarUnit.h"
#include "Micro/Units/TerranGround/MarineUnit.h"
#include "Micro/Units/TerranGround/MedicUnit.h"
#include "Micro/Units/ZergFlying/ScourgeUnit.h"
#include "Micro/Units/ZergFlying/MutaliskUnit.h"
#include <stack>
#include <typeinfo>
#include "Utils/Nearby.h"

int round(double a)
{
    return int(a + 0.5);
}

using namespace BWAPI;

#define __MAX_DISTANCE_TO_GROUP__ 512

UnitsGroup::UnitsGroup()
: defaultTargetEnemy(NULL)
, nearestChoke(NULL)
, totalHP(0)
, totalMinPrice(0)
, totalGazPrice(0)
, totalSupply(0)
, _hasDetection(0)
, enemiesCenter(Position(0, 0))
{
    _eUnitsFilter = & EUnitsFilter::Instance();
}

UnitsGroup::~UnitsGroup()
{
}

bool comp_i_dist(const i_dist& l, const i_dist& r) { return (r.dist < l.dist); }

void simple_align(std::vector<Position>& from, std::vector<unsigned int>& alignment)
{
    for(unsigned int i = 0; i < from.size(); i++)
    {
        alignment.push_back(i);
    }
}

/*** 
dumb O(#units^3) keeping the relative positions w.r.t. the centers
from and to are required to be of the same size()
*/
void mid_based_align(const std::vector<Position>& from, const std::vector<Position>& to, std::vector<unsigned int>& alignment)
{
    if (!from.size())
        return;
    assert(from.size() != 0);
    assert(to.size() == from.size());
    Position from_center;
    Position to_center;
    for (unsigned int i = 0; i < to.size(); ++i)
    {
        from_center += from[i];
        to_center += to[i];
    }
    from_center.x() /= from.size();
    from_center.y() /= from.size();
    to_center.x() /= to.size();
    to_center.y() /= to.size();
    std::vector<Vec> v_from_center(from.size(), Vec(0, 0));
    std::vector<Vec> v_to_center(to.size(), Vec(0, 0));
    for (unsigned int i = 0; i < to.size(); ++i) 
    {
        v_from_center[i] = Vec(from[i] - from_center);
        v_to_center[i] = Vec(to[i] - to_center);
    }

    std::set<unsigned int> done_j;
    if (to.size() < 40) // magic number
    {
        for (unsigned int i = 0; i < to.size(); ++i)
            alignment.push_back(i);
        std::set<unsigned int> done_i;
        while (done_j.size() < to.size()) 
        {
            double max = -1000000000000000.0; // bad
            unsigned int max_i, max_j;
            for (unsigned int i = 0; i < v_from_center.size(); ++i)
            {
                for (unsigned int j = 0; j < v_to_center.size(); ++j)
                {
                    if ((done_i.find(i) == done_i.end()) && (done_j.find(j) == done_j.end()) 
                        && fabs(v_from_center[i].dot(v_to_center[j])) > max)
                    {
                        max = v_from_center[i].dot(v_to_center[j]);
                        max_i = i;
                        max_j = j;
                    }
                }
            } 
            alignment[max_i] = max_j;
            done_i.insert(max_i);
            done_j.insert(max_j);
        }
    } else 
    {
        // non optimal, should have one more loop
        for (unsigned int i = 0; i < v_from_center.size(); ++i)
        {
            double max = -1000000000000000.0; // bad
            unsigned int max_j;
            for (unsigned int j = 0; j < v_to_center.size(); ++j)
            {
                if ((done_j.find(j) == done_j.end()) && fabs(v_from_center[i].dot(v_to_center[j])) > max)
                {
                    max = v_from_center[i].dot(v_to_center[j]);
                    max_j = j;
                }
            }
            alignment.push_back(max_j);
            done_j.insert(max_j);
        } 
    }
}

/***
/!\ dumb O(#units^2) trying (w/o collisions) to minimize the ending time
*/
void align(std::vector<Position>& from, std::vector<Position>& to, std::vector<unsigned int>& alignment)
{
    std::vector<std::vector<i_dist> > distances;
    std::vector<std::set<unsigned int> > attempts;
    std::set<unsigned int> inner_set;
    for(unsigned int i = 0; i < to.size(); i++) inner_set.insert(i);
    for(std::vector<BWAPI::Position>::iterator f = from.begin();
        f != from.end(); f++)
    {
        std::vector<i_dist> temp;
        for(unsigned int ind_to = 0; ind_to < to.size(); ind_to++)
        {
            temp.push_back(i_dist(ind_to, (*f).getApproxDistance(to[ind_to])));
        }
        //temp.sort();
        sort(temp.begin(), temp.end(), comp_i_dist); // [0]: further, [size()-1]: closer

        distances.push_back(temp);
        attempts.push_back(inner_set);
    }
    for(unsigned int i = 0; i < distances.size(); i++)
    {
        for(unsigned int j = 0; j < distances[0].size(); j++)
        {
            // TODO
        }
    }
}

void UnitsGroup::displayTargets()
{
    for each(pBayesianUnit u in units)
    {
        if (u->unit->getOrderTarget())
        {
            int ux = u->unit->getPosition().x(); int uy = u->unit->getPosition().y();
            int ex = u->unit->getOrderTarget()->getPosition().x(); int ey = u->unit->getOrderTarget()->getPosition().y();
            BWAPI::Broodwar->drawLineMap(ux,uy,ex,ey,Colors::Orange);
        
            if (u->targetEnemy && u->unit->getOrderTarget() != u->targetEnemy)
            {
                int ux = u->unit->getPosition().x(); int uy = u->unit->getPosition().y();
                int ex = u->targetEnemy->getPosition().x(); int ey = u->targetEnemy->getPosition().y();
                BWAPI::Broodwar->drawLineMap(ux,uy,ex,ey,Colors::Blue);
            }
            if (u->oorTargetEnemy && u->oorTargetEnemy->exists() && u->oorTargetEnemy->isVisible())
            {
                ex = u->oorTargetEnemy->getPosition().x(); int ey = u->oorTargetEnemy->getPosition().y();
                BWAPI::Broodwar->drawLineMap(ux,uy,ex,ey,Colors::Purple);
            }
        }
    }
}

double UnitsGroup::evaluateForces()
{
    bool onlyInvisibles = true;
    int theirMinPrice = 0;
    int theirGazPrice = 0;
    int theirSupply = 0; // this is double supply: 1 zergling = 1 supply
    for (std::map<Unit*, Position>::const_iterator it = enemies.begin();
        it != enemies.end(); ++it)
    {
        UnitType ut = _eUnitsFilter->getViewedUnit(it->first).type;
        if (ut.isBuilding())
        {
            // consider ground defenses only because we have only ground atm
            if (ut == UnitTypes::Protoss_Photon_Cannon || ut == UnitTypes::Terran_Bunker)
            {
                theirMinPrice += 3*ut.mineralPrice();
            } else if (ut == UnitTypes::Zerg_Sunken_Colony)
            {
                theirMinPrice += 3*(ut.mineralPrice() + 50);
            } else if (ut == UnitTypes::Protoss_Shield_Battery)
            {
                theirMinPrice += 2*ut.mineralPrice();
            }
            continue;
        }
        if (ut != UnitTypes::Zerg_Lurker // complete when !_eUnitsFilter->getInvisibleUnits().count(it->first)
            || ut != UnitTypes::Protoss_Dark_Templar
            || ((ut != UnitTypes::Terran_Wraith && ut != UnitTypes::Terran_Ghost) 
            || (ut == UnitTypes::Terran_Wraith || ut == UnitTypes::Terran_Ghost) && !_eUnitsFilter->getInvisibleUnits().count(it->first)))
            onlyInvisibles = false;
        theirMinPrice += ut.mineralPrice();
        theirGazPrice += ut.gasPrice();
        theirSupply += ut.supplyRequired();
        if (ut == UnitTypes::Terran_Siege_Tank_Siege_Mode) // a small boost for sieged tanks
            theirSupply += ut.supplyRequired();
        if (_eUnitsFilter->getInvisibleUnits().count(it->first)) // invisibles not detected count double
        {
            theirMinPrice += ut.mineralPrice();
            theirGazPrice += ut.gasPrice();
            theirSupply += ut.supplyRequired();
        }
    }
    // trying a simple rule: 100 minerals == 4 pop == 75 gaz == 100 pts
    double ourScore = totalMinPrice + (4/3)*totalGazPrice + 25*totalSupply;
    double theirScore = theirMinPrice + (4/3)*theirGazPrice + 25*theirSupply;
    if (enemiesAltitude > groupAltitude)
        theirScore *= 1.5;
    if (onlyInvisibles && !_hasDetection)
        return 0.1;
    else
        return ourScore/theirScore;
}

void BasicUnitsGroup::update()
{
    accomplishGoal();
	for (std::vector<pBayesianUnit>::const_iterator it = units.begin();
		it != units.end(); ++it)
	    (*it)->update();
}

void UnitsGroup::update()
{
	if (this == NULL) /// WHAT
		return;  /// THE FUCK?
#ifdef __DEBUG__
    clock_t start = clock();
#endif
	if (units.empty())
    {
		accomplishGoal();
		return;
    }

	for (std::list<pBayesianUnit>::iterator it = incompleteUnits.begin();
		it != incompleteUnits.end(); )
	{
		if ((*it)->unit->isCompleted())
		{
			dispatchCompleteUnit(*it);
			incompleteUnits.erase(it++);
		}
		else
			++it;
	}

    if (!arrivingUnits.empty())
    {
        if (units.size() <= 3)
        {
            for (std::list<pBayesianUnit>::iterator it = arrivingUnits.begin();
                it != arrivingUnits.end(); )
            {
                units.push_back(*it);
                arrivingUnits.erase(it++);
            }
        }
        else
        {
            for (std::list<pBayesianUnit>::iterator it = arrivingUnits.begin();
                it != arrivingUnits.end(); )
            {
                if ((*it)->unit->getPosition().getApproxDistance(center) < __MAX_DISTANCE_TO_GROUP__)
                {
                    units.push_back(*it);
					updateGroupStrengh((*it)->unit);
                    //goals.front()->achieve();
                    arrivingUnits.erase(it++);
                }
                else
                {
                    (*it)->target = center;
                    (*it)->update();
                    ++it;
                }
            }
        }
    }

    updateCenter();
    leadingUnit = units.front();
    for(std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); ++it)
    { 
        if ((*it)->unit->getType().isFlyer())
            continue;
        if ((leadingUnit->unit->getType().size() < (*it)->unit->getType().size() 
            || ( leadingUnit->unit->getType().size() == (*it)->unit->getType().size() && 
                 leadingUnit->unit->getDistance(center) > (*it)->unit->getDistance(center) )) && (leadingUnit->unit->getHitPoints() + leadingUnit->unit->getShields() > 100)
            )
            leadingUnit = *it;
    }
    if (leadingUnit != NULL && leadingUnit->unit->exists()) // defensive prog
    {
        leadingUnit->updatePPath();
        if (!leadingUnit->getPPath().empty())
            ppath = leadingUnit->getPPath();
    }

    this->totalHP = 0;
    double maxRange = -1.0;
    /*** TODO BUG IN SquareFormation TODO TODO TODO volatile bool contactUnits = false; // why do I need volatile to make it work not erratically? */
    for (std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); ++it)
    {
        /*** TODO BUG IN SquareFormation TODO TODO TODO
        if ((*it)->unit->getType() == UnitTypes::Protoss_Zealot 
            || (*it)->unit->getType() == UnitTypes::Protoss_Archon)
            contactUnits = true;
        */
        totalHP += (*it)->unit->getHitPoints();
		if (Broodwar->self()->getRace() == Races::Protoss)
			totalHP += (*it)->unit->getShields();
        double tmp_max = max(max((*it)->unit->getType().groundWeapon().maxRange(), (*it)->unit->getType().airWeapon().maxRange()), 
            (*it)->unit->getType().sightRange()); // TODO: upgrades
        if (tmp_max > maxRange)
            maxRange = tmp_max;
    }
    /*** TODO BUG IN SquareFormation TODO TODO TODO
    if (contactUnits)
        _alignFormation = false;
    else
        _alignFormation = true;*/

    //clock_t s = clock();
    updateNearbyEnemyUnitsFromFilter(center, maxRadius + maxRange + 92); // possibly hidden units, could be taken from onUnitsShow/View asynchronously for more efficiency
    // update enemiesCenter
    if (enemies.size () != 0)
    {
        enemiesCenter = Position(0, 0);
        enemiesAltitude = 0;
        for (std::map<Unit*, Position>::const_iterator it = enemies.begin();
            it != enemies.end(); ++it)
        {
            enemiesCenter += it->second;
            if (!(it->first->getType().isFlyer()))
                enemiesAltitude += BWAPI::Broodwar->getGroundHeight(TilePosition(it->second));
        }
        enemiesCenter.x() /= enemies.size();
        enemiesCenter.y() /= enemies.size();
        if (!enemiesCenter.isValid())
            enemiesCenter.makeValid();
        enemiesAltitude = round((double)enemiesAltitude / enemies.size());
    }
    //clock_t f = clock();
    //double dur = (double)(f - s) / CLOCKS_PER_SEC;
    //Broodwar->printf( "UnitsGroup::update() took %2.5f seconds\n", dur); 
#ifdef __DEBUG__
    Broodwar->drawCircleMap((int)center.x(), (int)center.y(), (int)maxRadius + (int)maxRange +  32, Colors::Yellow);
#endif

    if (!enemies.empty()) /// We fight, we'll see later for the goals 
    {
        double force = evaluateForces();
        if (force < 0.8) // TOCHANGE 0.8
        {
            // strategic withdrawal
            for (std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); ++it)
            {
                (*it)->target = Position(Broodwar->self()->getStartLocation());
            }
        }
        else if (force > 1.5) // TOCHANGE 1.5
        {
            // we can be offensive, get our goals done
            accomplishGoal();
        }
        else // stand our ground or go up the ramp
        {
            for(std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); ++it)
            {
                if (enemiesAltitude > groupAltitude && nearestChoke)
                {
                    const std::pair<BWTA::Region*, BWTA::Region*> regions = nearestChoke->getRegions();
                    BWTA::Region* higherRegion = 
						(Broodwar->getGroundHeight(TilePosition(regions.first->getCenter()))
> Broodwar->getGroundHeight(TilePosition(regions.second->getCenter())))
                        ? regions.first : regions.second;
					(*it)->target = MapManager::Instance().regionsPFCenters[higherRegion];
                }
                else
                    (*it)->target = (*it)->unit->getPosition();
            }
        }
    }
    else /// Let's do the goals now 
    {
        defaultTargetEnemy = NULL;
        accomplishGoal();
    }

#ifdef __DEBUG__
    displayTargets();
#endif

#ifdef __DEBUG__
    clock_t finish = clock();
    double duration = (double)(finish - start) / CLOCKS_PER_SEC;
    if (duration > 0.040) 
        Broodwar->printf( "UnitsGroup::update() took %2.5f seconds\n", duration);
#endif

    for (std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); ++it)
        (*it)->update();
    templarMergingStuff();
}

void UnitsGroup::attack(int x, int y)
{
    attack(Position(x, y));
}

void UnitsGroup::attack(BWAPI::Position& p)
{
    for(std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); it++)
    {
        (*it)->target = p;
        //(*it)->attack(p); // TODO, for the moment, each unit keeps a path, needs to be 1 unit per UnitsGroup + flocking
    }
}

void UnitsGroup::move(BWAPI::Position& p)
{
    for(std::vector<pBayesianUnit>::iterator it = this->units.begin(); it != this->units.end(); it++)
    {
        (*it)->target = p;
    }
}

void UnitsGroup::formation(pFormation f)
{
    if (units.empty())
        return;
    std::vector<BWAPI::Position> from;
    for(std::vector<pBayesianUnit>::iterator it = units.begin(); it != units.end(); it++)
    {
        from.push_back((*it)->unit->getPosition());
    }

    f->computeToPositions(units);

    /*** TODO BUG IN SquareFormation TODO TODO TODOif (_alignFormation)
    {*/
        const std::vector<BWAPI::Position>& to = f->end_positions;
        std::vector<unsigned int> alignment; // alignment[from_pos] = to_pos 
        // align(from, to, alignment);// TODO min crossing || fastest
        // simple_align(from, alignment);
        mid_based_align(from, to, alignment);
        for (unsigned int i = 0; i < units.size(); i++)
        {
            units[i]->target = to[alignment[i]];
        }
    /*** TODO BUG IN SquareFormation TODO TODO TODO}
    else
    {
        for (unsigned int i = 0; i < units.size(); i++)
        {
            units[i]->target = f->end_positions[i];
        }
    }*/

}

void UnitsGroup::setGoals(std::list<pGoal>& goals)
{
    this->goals = goals;
	if(!this->goals.empty())
		this->goals.front()->achieve();
}

void UnitsGroup::addGoal(pGoal goal)
{
    this->goals.push_back(goal);
	goal->setUnitsGroup(this);
}

void UnitsGroup::addGoalFront(pGoal goal)
{
    this->goals.push_front(goal);
    goal->setUnitsGroup(this);
    goals.front()->achieve();
}


void UnitsGroup::onUnitDestroy(Unit* u)
{
    if (u->getPlayer() != Broodwar->self())
        for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
            (*it)->onUnitDestroy(u);
    else
    {
        giveUpControl(u);
    }
    unitDamages.left.erase(u);
}

void UnitsGroup::onUnitShow(Unit* u)
{
    for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
        (*it)->onUnitShow(u);
    if (u->getPlayer() == Broodwar->enemy() && u->isDetected()) //(!u->getType().isBuilding())
        unitDamages.insert(UnitDmg(u, Dmg(0, u, u->getHitPoints() + u->getShields())));
}

void UnitsGroup::onUnitHide(Unit* u)
{    
    for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
        (*it)->onUnitHide(u);
}

double UnitsGroup::getDistance(BWAPI::Unit* u) const
{
    return Vec(center - u->getPosition()).norm();
}

pBayesianUnit UnitsGroup::addUnit(Unit* u)
{
    pBayesianUnit tmp;
    if (u->getType() == BWAPI::UnitTypes::Protoss_Arbiter)
        tmp = pBayesianUnit(new ArbiterUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Archon)
        tmp = pBayesianUnit(new ArchonUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Carrier)
        tmp = pBayesianUnit(new CarrierUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Corsair)
        tmp = pBayesianUnit(new CorsairUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Dark_Archon)
        tmp = pBayesianUnit(new DarkArchonUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar)
        tmp = pBayesianUnit(new DarkTemplarUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
        tmp = pBayesianUnit(new DragoonUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_High_Templar)
        tmp = pBayesianUnit(new HighTemplarUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Observer)
        tmp = pBayesianUnit(new ObserverUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Probe)
        tmp = pBayesianUnit(new ProbeUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Reaver)
        tmp = pBayesianUnit(new ReaverUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Scout)
        tmp = pBayesianUnit(new ScoutUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Shuttle)
        tmp = pBayesianUnit(new ShuttleUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Protoss_Zealot)
        tmp = pBayesianUnit(new ZealotUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Zerg_Mutalisk)
        tmp = pBayesianUnit(new MutaliskUnit(u, this));
    else if (u->getType() == BWAPI::UnitTypes::Zerg_Scourge)
        tmp = pBayesianUnit(new ScourgeUnit(u, this));
	else if (u->getType() == BWAPI::UnitTypes::Terran_Marine)
		tmp = pBayesianUnit(new MarineUnit(u, this));
	else if (u->getType() == BWAPI::UnitTypes::Terran_Medic)
		tmp = pBayesianUnit(new MedicUnit(u, this));
    else
	{
        Broodwar->printf("This race/unit is not implemented");
		return tmp;
	}
	return tmp;
}

void UnitsGroup::dispatchCompleteUnit(pBayesianUnit bu)
{
	if (bu->unit->getPosition().getApproxDistance(center) < __MAX_DISTANCE_TO_GROUP__ || !units.size())
    {
		units.push_back(bu);
		updateGroupStrengh(bu->unit);
    }
    else
    {
        bu->unit->attack(center);
        arrivingUnits.push_back(bu);
    }
}

void UnitsGroup::takeControl(Unit* u)
{
	pBayesianUnit tmp = addUnit(u);
	if (tmp.get() == NULL)
	{
#ifdef __DEBUG__
		Broodwar->printf("Took control of a unit I can't build a BayesianUnit from: %s", u->getType().getName().c_str());
#endif
		return;
	}
	if (!u->isCompleted())
		incompleteUnits.push_back(tmp);
	else 
		dispatchCompleteUnit(tmp);
}

bool BasicUnitsGroup::removeUnit(Unit* u)
{
    for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
        if ((*it)->unit == u)
        {
            units.erase(it);
			return true;
        }
	return false;
}

void UnitsGroup::giveUpControl(Unit* u)
{
	removeUnit(u);
    if (u->getType() == UnitTypes::Protoss_Observer)
    {
        _hasDetection = false;
        for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
            if ((*it)->unit->getType() == UnitTypes::Protoss_Observer)
            {
                _hasDetection = true;
                break;
            }
    }
    totalMinPrice -= u->getType().mineralPrice();
    totalGazPrice -= u->getType().gasPrice();
    totalSupply -= u->getType().supplyRequired();
}

bool BasicUnitsGroup::emptyUnits()
{
    return units.empty();
}
bool BasicUnitsGroup::emptyGoals()
{
    return goals.empty();
}

void UnitsGroup::updateGroupStrengh(Unit* u)
{
	if (u->getType() == UnitTypes::Protoss_Observer)
		_hasDetection = true;
	totalMinPrice += u->getType().mineralPrice();
	totalGazPrice += u->getType().gasPrice();
	totalSupply += u->getType().supplyRequired();
}


int UnitsGroup::getTotalHP() const
{
    return totalHP;
}

std::vector<pBayesianUnit>* UnitsGroup::getUnits()
{
    return &units;
}

void UnitsGroup::updateNearbyEnemyUnitsFromFilter(BWAPI::Position p, double radius)
{
    enemies.clear();
    // have units that have been seek like units on cliffs or lurkers before burrowing (for instance)
    for (std::map<BWAPI::Unit*, EViewedUnit>::const_iterator it = _eUnitsFilter->getViewedUnits().begin();
        it != _eUnitsFilter->getViewedUnits().end(); ++it)
    {
        if (it->second.position.getApproxDistance(p) <= radius)
            enemies.insert(std::make_pair<Unit*, Position>(it->first, it->second.position));
    }
}

const BayesianUnit& UnitsGroup::operator[](ptrdiff_t i)
{
    return *(units[i]);
}

void UnitsGroup::display()
{
	
    Broodwar->drawCircle(CoordinateType::Map, center.x(), center.y(), 8, Colors::Green);
    for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); it++)
    {
        Unit* unit = (*it)->unit;
        Position target = (*it)->target;

        // Attack range
        //Broodwar->drawCircle( CoordinateType::Map, unit->getPosition().x(), unit->getPosition().y(), unit->getType().groundWeapon()->maxRange(), Colors::Red);

        // Target
        if( true)//unit->getPosition().getApproxDistance( target) < 200)
        {
            Broodwar->drawCircle( CoordinateType::Map, unit->getPosition().x(), unit->getPosition().y(), 5, Colors::Cyan, true);
            Broodwar->drawLine( CoordinateType::Map, unit->getPosition().x(), unit->getPosition().y(), target.x(), target.y(), Colors::Cyan);
            Broodwar->drawCircle( CoordinateType::Map, target.x(), target.y(), 5, Colors::Cyan, true);
        }

        // Goal status
        if( goals.empty() || goals.front()->getStatus() == GS_ACHIEVED)
            Broodwar->drawTextMap( unit->getPosition().x(), unit->getPosition().y(), "W");
        if( !goals.empty() && goals.front()->getStatus() == GS_IN_PROGRESS)
            Broodwar->drawTextMap( unit->getPosition().x(), unit->getPosition().y(), "P");
    }

	//TOUNCOMMENT when formation for all units group accomplished
//    for (std::list<pGoal>::iterator it = goals.begin(); it != goals.end(); it++)
//    {
//        Broodwar->drawCircle( CoordinateType::Map, (int)((*it)->formation->center.x), (int)((*it)->formation->center.y), 5, Colors::White, true);
//    }
	
}

int BasicUnitsGroup::size() const
{
    return units.size();
}

void UnitsGroup::updateCenter()
{
    if (!units.size())
        return;
    // update center
    center = Position(0, 0);
    groupAltitude = 0;
    for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
    {
        center += (*it)->unit->getPosition();
        if (!(*it)->unit->getType().isFlyer())
            groupAltitude += Broodwar->getGroundHeight(TilePosition((*it)->unit->getPosition()));
    }
    center.x() /= units.size();
    center.y() /= units.size();
    groupAltitude = round((double)groupAltitude / units.size());
    if (nearestChoke)
        distToNearestChoke = nearestChoke->getCenter().getApproxDistance(center);
    if (!nearestChoke || distToNearestChoke > 256)
    {
        nearestChoke = BWTA::getNearestChokepoint(center);
    }

    // update stdDevRadius and maxRadius
    maxRadius = -1.0;
    double sum = 0.0;
    for (std::vector<pBayesianUnit>::const_iterator it = units.begin(); it != units.end(); ++it)
    {
        double dist = center.getApproxDistance((*it)->unit->getPosition());
        if (dist > maxRadius)
            maxRadius = dist;
        sum += (dist * dist);
    }
    stdDevRadius = sqrt((1/units.size()) * sum); // 1/(units.size() - 1) for the sample std dev
}

Position UnitsGroup::getCenter() const
{
    return center;
}

#ifdef _UNITS_DEBUG
void UnitsGroup::selectedUnits(std::set<pBayesianUnit>& u)
{
    for (std::vector<pBayesianUnit>::iterator i=this->units.begin(); i!=this->units.end(); ++i)
    {
        if ((*i)->unit->isSelected())
            u.insert(*i);
    }
}
#endif


void BasicUnitsGroup::accomplishGoal()
{	
	if(goals.size() > 0){
		if (goals.front()->getStatus() != GS_ACHIEVED) {
			goals.front()->achieve();
		} else {
			if(goals.size() > 1 ){
				goals.pop_front();
			}
		}
	}
}

void UnitsGroup::switchMode(unit_mode um){
	for(std::vector<pBayesianUnit>::iterator it = getUnits()->begin(); it != getUnits()->end(); ++it){
		(*it)->switchMode(um);
	}
}

void UnitsGroup::idle(){
	for(std::vector<pBayesianUnit>::iterator it = getUnits()->begin(); it != getUnits()->end(); ++it){
		(*it)->target = (*it)->unit->getPosition();
	}
}

pGoal UnitsGroup::getLastGoal(){
	return this->goals.front();
}

bool UnitsGroup::isWaiting(){
	if(goals.size() <= 0)
		//Problematic situation
		return true;
	return goals.size() == 1 && (*goals.front()).getStatus() == GS_ACHIEVED ;
}

void UnitsGroup::signalMerge(Unit* u)
{
    if (u->getType() == UnitTypes::Protoss_High_Templar)
    {
        _mergersHT.insert(u);
    }
}

void UnitsGroup::templarMergingStuff()
{
    if (_mergersHT.size() < 2)
        return;
    Unit* tomerge = NULL;
    Unit* closer = NULL;
    double distance = 1000000.0;
    // stupid, suboptimal, heuristic
    for (std::set<Unit*>::iterator it = _mergersHT.begin();
        it != _mergersHT.end(); )
    {
        if (!(*it)->exists())
        {
            _mergersHT.erase(it++);
            continue;
        }
        else if (!tomerge)
            tomerge = *it;
        else
        {
            double tmp = (*it)->getDistance(tomerge);
            if (tmp < distance)
            {
                closer = *it;
                distance = tmp;
            }
        }
        ++it;
    }
    if (tomerge != NULL)
    {
        if (tomerge->exists())
        {
            if (closer != NULL)
            {
                if (closer->exists())
                    tomerge->useTech(TechTypes::Archon_Warp, closer);
            }
            _mergersHT.erase(closer);
        }
        _mergersHT.erase(tomerge);
    }
}
