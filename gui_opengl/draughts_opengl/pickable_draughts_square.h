// Class for defining the action to take when a square on the draughts board is picked

#ifndef _PICKABLE_DRAUGHTS_SQUARE_H_
#define _PICKABLE_DRAUGHTS_SQUARE_H_

#include "ipickable.h"
#include "../../board.h"

class CPickableDraughtsSquare : public IPickable
{
   public:
      // Constructor
      CPickableDraughtsSquare(CBoard &_board, const int _xCoord, const int _yCoord)
      :  board(_board), xCoord(_xCoord), yCoord(_yCoord)
      {}

      // Function to execute upon the square that this object corresponds to
      void Execute()
      {
         board.SelectSquare( xCoord ,yCoord );
         board.ExecuteSelectedSquare();
      }

      // Copying/assignment
      CPickableDraughtsSquare(const CPickableDraughtsSquare &other)
      :  board(other.board), xCoord(other.xCoord), yCoord(other.yCoord)
      {}

      const CPickableDraughtsSquare &operator=(const CPickableDraughtsSquare &rhs)
      {
         board = rhs.board;
         xCoord = rhs.xCoord;
         yCoord = rhs.yCoord;
         return *this;
      }

   private:
      // Default constructor is not possible - must have a reference to a CBoard
      CPickableDraughtsSquare();

      // Reference to the draughts board
      CBoard &board;

      // The location of the square to execute upon
      int xCoord;
      int yCoord;
};


#endif // _PICKABLE_DRAUGHTS_SQUARE_H_
