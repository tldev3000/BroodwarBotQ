#include <PrecompiledHeader.h>
#include "FindSubgoal.h"
#include "Defines.h"
#include "ScoutManager.h"

FindSubgoal::FindSubgoal(SubgoalLogic l)
: Subgoal(l)
, achieved(false)
{
}

bool FindSubgoal::isRealized()
{
	if (achieved)
		return true;
	else
		achieved = check();
		return achieved;
}

bool FindSubgoal::check()
{
    return ScoutManager::Instance().enemyFound; // quick fix
}

void FindSubgoal::tryToRealize()
{
}

double FindSubgoal::distanceToRealize()
{
	return -1;
}