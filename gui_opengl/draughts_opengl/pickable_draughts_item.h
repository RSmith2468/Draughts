// Class for defining the action to take when a text pertaining to the draughts board is picked

#ifndef _PICKABLE_DRAUGHTS_ITEM_H_
#define _PICKABLE_DRAUGHTS_ITEM_H_

#include "ipickable.h"
#include "scene.h"

namespace NSPickableDraughtsItem
{
   // enum to define what action this object will take upon the board
   enum ActionOptions
   {
      RESET,
      OPEN_MENU,
      CLOSE_MENU,
      AIP_MODERATE,     // AIP = AI Personality
      AIP_AGGRESSIVE,
      AIP_GENEROUS,
      AIP_STUPID,
      AIP_RANDOM,
      AI_SIDE,
      FIRST_TURN,
      LAYOUT
   };

   class CPickableDraughtsItem : public IPickable
   {
      public:
         // Constructor
         CPickableDraughtsItem(Scene &_scene, const unsigned int _action)
         :  scene(_scene), action(_action)
         {}

         // Function to execute upon the square that this object corresponds to
         void Execute()
         {
            switch(action)
            {
               case RESET:
                  scene.ResetBoard();
                  break;
               case OPEN_MENU:
                  scene.OpenMenu();
                  break;
               case CLOSE_MENU:
                  scene.CloseMenu();
                  break;
               case AIP_MODERATE:
               case AIP_AGGRESSIVE:
               case AIP_GENEROUS:
               case AIP_STUPID:
               case AIP_RANDOM:
                  scene.SetPersonality(action - AIP_MODERATE);
                  break;
               case AI_SIDE:
                  scene.ToggleAISide();
                  break;
               case FIRST_TURN:
                  scene.ToggleFirstTurn();
                  break;
               default: //case LAYOUT:
                  scene.ToggleLayout();
                  break;
            }
         }

         // Copying/assignment
         CPickableDraughtsItem(const CPickableDraughtsItem &other)
         :  scene(other.scene), action(other.action)
         {}

         const CPickableDraughtsItem &operator=(const CPickableDraughtsItem &rhs)
         {
            scene = rhs.scene;
            action = rhs.action;
            return *this;
         }

      private:
         // Default constructor is not possible - must have a reference to a CBoard
         CPickableDraughtsItem();

         // Reference to the draughts board
         Scene &scene;

         unsigned int action;
   };
}

#endif // _PICKABLE_DRAUGHTS_ITEM_H_
