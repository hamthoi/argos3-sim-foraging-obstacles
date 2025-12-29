#ifndef FORAGING_OBSTACLES_LOOP_FUNCTIONS_H
#define FORAGING_OBSTACLES_LOOP_FUNCTIONS_H

#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/core/utility/math/range.h>
#include <argos3/core/utility/math/rng.h>

#include <vector>
#include <string>
#include <fstream>

using namespace argos;

class CForagingObstaclesLoopFunctions : public CLoopFunctions {

public:

   CForagingObstaclesLoopFunctions();
   virtual ~CForagingObstaclesLoopFunctions() {}

   virtual void Init(TConfigurationNode& t_tree);
   virtual void Reset();
   virtual void Destroy();
   virtual CColor GetFloorColor(const CVector2& c_position_on_plane);
   virtual void PreStep();

private:

   /* Food parameters */
   Real m_fFoodSquareRadius;
   CRange<Real> m_cForagingArenaSideX;
   CRange<Real> m_cForagingArenaSideY;
   std::vector<CVector2> m_cFoodPos;
   std::vector<UInt32>  m_cFoodAge;
   UInt32 m_unFoodDespawnTicks;

   /* ARGoS utilities */
   CFloorEntity* m_pcFloor;
   CRandom::CRNG* m_pcRNG;

   /* Logging */
   std::string m_strOutput;
   std::ofstream m_cOutput;

   /* Statistics */
   UInt32 m_unCollectedFood;
   SInt64 m_nEnergy;
   UInt32 m_unEnergyPerFoodItem;
   UInt32 m_unEnergyPerWalkingRobot;
};

#endif
