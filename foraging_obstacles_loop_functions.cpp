#include "foraging_obstacles_loop_functions.h"

#include <argos3/core/simulator/simulator.h>
#include <argos3/core/utility/configuration/argos_configuration.h>
#include <argos3/plugins/robots/foot-bot/simulator/footbot_entity.h>
#include <controllers/footbot_foraging/footbot_foraging.h>

/****************************************/
/****************************************/

CForagingObstaclesLoopFunctions::CForagingObstaclesLoopFunctions() :
   m_cForagingArenaSideX(-0.9f, 1.7f),
   m_cForagingArenaSideY(-1.7f, 1.7f),
   m_pcFloor(NULL),
   m_pcRNG(NULL),
   m_unCollectedFood(0),
   m_nEnergy(0),
   m_unEnergyPerFoodItem(1),
   m_unEnergyPerWalkingRobot(1),
   m_unFoodDespawnTicks(600) {
}

/****************************************/
/****************************************/

void CForagingObstaclesLoopFunctions::Init(TConfigurationNode& t_node) {
   try {
      TConfigurationNode& tForaging = GetNode(t_node, "foraging");

      /* Get floor */
      m_pcFloor = &GetSpace().GetFloorEntity();

      /* Food parameters */
      UInt32 unFoodItems;
      GetNodeAttribute(tForaging, "items", unFoodItems);
      GetNodeAttribute(tForaging, "radius", m_fFoodSquareRadius);
      m_fFoodSquareRadius *= m_fFoodSquareRadius;

      /* Optional despawn time */
      GetNodeAttributeOrDefault(tForaging,
                               "food_despawn_ticks",
                               m_unFoodDespawnTicks,
                               m_unFoodDespawnTicks);

      /* RNG */
      m_pcRNG = CRandom::CreateRNG("argos");

      /* Create food items */
      m_cFoodPos.clear();
      m_cFoodAge.clear();

      for(UInt32 i = 0; i < unFoodItems; ++i) {
         m_cFoodPos.push_back(
            CVector2(m_pcRNG->Uniform(m_cForagingArenaSideX),
                     m_pcRNG->Uniform(m_cForagingArenaSideY)));
         m_cFoodAge.push_back(0);
      }

      /* Output */
      GetNodeAttribute(tForaging, "output", m_strOutput);
      m_cOutput.open(m_strOutput.c_str(),
                     std::ios_base::trunc | std::ios_base::out);
      m_cOutput << "# clock\twalking\tresting\tcollected_food\tenergy"
                << std::endl;

      /* Energy parameters */
      GetNodeAttribute(tForaging, "energy_per_item",
                       m_unEnergyPerFoodItem);
      GetNodeAttribute(tForaging, "energy_per_walking_robot",
                       m_unEnergyPerWalkingRobot);
   }
   catch(CARGoSException& ex) {
      THROW_ARGOSEXCEPTION_NESTED(
         "Error parsing foraging obstacles loop functions!", ex);
   }
}

/****************************************/
/****************************************/

void CForagingObstaclesLoopFunctions::Reset() {
   m_unCollectedFood = 0;
   m_nEnergy = 0;

   m_cOutput.close();
   m_cOutput.open(m_strOutput.c_str(),
                  std::ios_base::trunc | std::ios_base::out);
   m_cOutput << "# clock\twalking\tresting\tcollected_food\tenergy"
             << std::endl;

   for(size_t i = 0; i < m_cFoodPos.size(); ++i) {
      m_cFoodPos[i].Set(m_pcRNG->Uniform(m_cForagingArenaSideX),
                        m_pcRNG->Uniform(m_cForagingArenaSideY));
      m_cFoodAge[i] = 0;
   }

   m_pcFloor->SetChanged();
}

/****************************************/
/****************************************/

void CForagingObstaclesLoopFunctions::Destroy() {
   m_cOutput.close();
}

/****************************************/
/****************************************/

CColor CForagingObstaclesLoopFunctions::GetFloorColor(
   const CVector2& c_position_on_plane) {

   if(c_position_on_plane.GetX() < -1.0f) {
      return CColor::GRAY50;
   }

   for(size_t i = 0; i < m_cFoodPos.size(); ++i) {
      if((c_position_on_plane - m_cFoodPos[i]).SquareLength()
         < m_fFoodSquareRadius) {
         return CColor::BLACK;
      }
   }

   return CColor::WHITE;
}

/****************************************/
/****************************************/

void CForagingObstaclesLoopFunctions::PreStep() {

   /* Increase food age */
   for(size_t i = 0; i < m_cFoodAge.size(); ++i) {
      ++m_cFoodAge[i];
   }

   /* Despawn old food */
   for(size_t i = 0; i < m_cFoodPos.size(); ++i) {
      if(m_cFoodAge[i] >= m_unFoodDespawnTicks &&
         m_cFoodPos[i].GetX() < 50.0f) {

         m_cFoodPos[i].Set(
            m_pcRNG->Uniform(m_cForagingArenaSideX),
            m_pcRNG->Uniform(m_cForagingArenaSideY));

         m_cFoodAge[i] = 0;
         m_pcFloor->SetChanged();
      }
   }

   UInt32 unWalkingFBs = 0;
   UInt32 unRestingFBs = 0;

   CSpace::TMapPerType& cFootbots =
      GetSpace().GetEntitiesByType("foot-bot");

   for(auto it = cFootbots.begin();
       it != cFootbots.end();
       ++it) {

      CFootBotEntity& cFootBot =
         *any_cast<CFootBotEntity*>(it->second);

      CFootBotForaging& cController =
         dynamic_cast<CFootBotForaging&>(
            cFootBot.GetControllableEntity().GetController());

      if(!cController.IsResting()) ++unWalkingFBs;
      else ++unRestingFBs;

      CVector2 cPos(
         cFootBot.GetEmbodiedEntity()
            .GetOriginAnchor().Position.GetX(),
         cFootBot.GetEmbodiedEntity()
            .GetOriginAnchor().Position.GetY());

      CFootBotForaging::SFoodData& sFoodData =
         cController.GetFoodData();

      /* Carrying food */
      if(sFoodData.HasFoodItem) {
         if(cPos.GetX() < -1.0f) {
            UInt32 idx = sFoodData.FoodItemIdx;

            m_cFoodPos[idx].Set(
               m_pcRNG->Uniform(m_cForagingArenaSideX),
               m_pcRNG->Uniform(m_cForagingArenaSideY));

            m_cFoodAge[idx] = 0;

            sFoodData.HasFoodItem = false;
            sFoodData.FoodItemIdx = 0;
            ++sFoodData.TotalFoodItems;

            m_nEnergy += m_unEnergyPerFoodItem;
            ++m_unCollectedFood;

            m_pcFloor->SetChanged();
         }
      }
      /* Not carrying food */
      else {
         if(cPos.GetX() > -1.0f) {
            for(size_t i = 0; i < m_cFoodPos.size(); ++i) {
               if((cPos - m_cFoodPos[i]).SquareLength()
                  < m_fFoodSquareRadius) {

                  m_cFoodPos[i].Set(100.0f, 100.0f);
                  m_cFoodAge[i] = 0;

                  sFoodData.HasFoodItem = true;
                  sFoodData.FoodItemIdx = i;

                  m_pcFloor->SetChanged();
                  break;
               }
            }
         }
      }
   }

   /* Energy loss */
   m_nEnergy -= unWalkingFBs * m_unEnergyPerWalkingRobot;

   /* Log */
   m_cOutput << GetSpace().GetSimulationClock() << "\t"
             << unWalkingFBs << "\t"
             << unRestingFBs << "\t"
             << m_unCollectedFood << "\t"
             << m_nEnergy << std::endl;
}

/****************************************/
/****************************************/

REGISTER_LOOP_FUNCTIONS(
   CForagingObstaclesLoopFunctions,
   "foraging_obstacles_loop_functions")
