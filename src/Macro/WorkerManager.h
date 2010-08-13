#pragma once
#include <Arbitrator.h>
#include <BWAPI.h>
#include <BaseManager.h>
#include "CSingleton.h"
class BuildOrderManager;
class WorkerManager : public Arbitrator::Controller<BWAPI::Unit*,double>, public CSingleton<WorkerManager>
{
	friend class CSingleton<WorkerManager>;
  public:
    class WorkerData
    {
      public:
        WorkerData() {resource = NULL; lastFrameSpam = 0;}
        BWAPI::Unit* resource;
        BWAPI::Unit* newResource;
        int lastFrameSpam;
    };
	void setDependencies();

    virtual void onOffer(std::set<BWAPI::Unit*> units);
    virtual void onRevoke(BWAPI::Unit* unit, double bid);
    virtual void update();
    virtual std::string getName() const;
    virtual std::string getShortName() const;

    void onRemoveUnit(BWAPI::Unit* unit);
    void setWorkersPerGas(int count);
    double getMineralRate() const;
    double getGasRate() const;
    int getOptimalWorkerCount() const;
    void enableAutoBuild();
    void disableAutoBuild();
    void setAutoBuildPriority(int priority);
	    
	std::map<BWAPI::Unit*,WorkerData> workers;
	    bool autoBuild;
  private:

	WorkerManager();
    Arbitrator::Arbitrator<BWAPI::Unit*,double>* arbitrator;
    BaseManager* baseManager;
    BuildOrderManager* buildOrderManager;

    std::map<BWAPI::Unit*, std::set<BWAPI::Unit*> > currentWorkers;
    std::map<BWAPI::Unit*, Base*> resourceBase;
    std::map<BWAPI::Unit*, int> desiredWorkerCount;
    std::vector<std::pair<BWAPI::Unit*, int> > mineralOrder;
    int mineralOrderIndex;
    int lastSCVBalance;
    std::set<Base*> basesCache;
    int WorkersPerGas;
    
    void rebalanceWorkers();
    void updateWorkerAssignments();
    double mineralRate;
    double gasRate;

    int autoBuildPriority;
    int optimalWorkerCount;
};