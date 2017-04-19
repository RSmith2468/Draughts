// Declaration of class for the draughts pieces

#ifndef _PIECE_H
#define _PIECE_H

#include <iostream>  // std::cout

class CPiece
{
   public:
      // Constructor
      CPiece() : type(NONE), side(X) {}
      
      // Functions for modifying the type
      void SetTypeMan()  { type = MAN; }
      void SetTypeKing() { type = KING; }
      void SetSideX()    { side = X; }
      void SetSideO()    { side = O; }
      void SetTypeEmpty() { type = NONE; }
      
      // Functions for querying the piece
      bool IsMan() const { return (type == MAN); }
      bool IsX()   const { return (side == X);   }
      
      // Operators
      bool operator==(const CPiece &rhs) const { return (type == rhs.type) && (side == rhs.side); }
      bool operator!=(const CPiece &rhs) const { return (type != rhs.type) || (side != rhs.side); }
      
   private:
      // Enumerated type for each piece (man or king)
      enum PieceType
      {
         NONE,
         MAN,
         KING
      };
      // Enumerated side for each piece ('x' or 'o')
      enum PieceSide
      {
         X,
         O
      };
      
      PieceType type;
      PieceSide side;
};


#endif
