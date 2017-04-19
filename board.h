// Declaration of class for the draughts board

#ifndef _BOARD_H
#define _BOARD_H

#include <vector>

#include "randomrs.h"
#include "piece.h"


// Class for describing & controlling the board
class CBoard
{
   public:
      // Constructors
      CBoard() : CBoard(8, 8, 12) {}
      
      // Member Functions
      void ResetBoard(const bool _IsXTurn);
      void SetLayoutAndReset(const bool _layout) { boardLayout = _layout; ResetBoard(isXTurn); }
      bool Layout() const { return boardLayout; }
      
      void SelectSquare(unsigned int x, unsigned int y) { selectedSquare.x=int(x); selectedSquare.y=int(y); }
      
      // Return code: 0 = OK, 1 = invalid piece/square selected, 2 = invalid attempted move, 3 = invalid attempted move (aggressive move is available)
      int ExecuteSelectedSquare();
      
      void MoveSelectSquareUp()    { selectedSquare.y -= (selectedSquare.y>0?1:0); }
      void MoveSelectSquareDown()  { selectedSquare.y += (selectedSquare.y<int(height)-1?1:0); }
      void MoveSelectSquareLeft()  { selectedSquare.x -= (selectedSquare.x>0?1:0); }
      void MoveSelectSquareRight() { selectedSquare.x += (selectedSquare.x<int(width)-1?1:0); }
      
      bool CurrentSideHasMoves() const { return currentSideHasMoves; }
      
      // Function to invoke the AI to take a relevant action (returns a success bool in case the AI fails for some reason)
      bool InvokeAI( int depth );
      
      // Functions to query whose turn it is & the force a turnover (which results in a reset of the board)
      bool IsXTurn() const { return isXTurn; }
      
      void ForceTurn(const bool _isXTurn) { ResetBoard(_isXTurn); }

      // Functions for altering the AI's personality (MODERATE, GENEROUS, AGGRESSIVE, CAUTIOUS).
      // Level of intelligence is detemined by the "depth" argument that is passed into the InvokeAI function (0 = AI makes random moves)
      void SetAIModerate()   { aiPersonality = MODERATE; }
      void SetAIGenerous()   { aiPersonality = GENEROUS; }
      void SetAIAggressive() { aiPersonality = AGGRESSIVE; }
      void SetAICautious()   { aiPersonality = CAUTIOUS; }
      
      // Functions to get information about the piece at a given square location (GetSquare will handle x or y being out of bounds)
      bool SquareIsEmpty(unsigned int x, unsigned int y)
      {
         return GetSquare(x,y).GetPiece() == emptyPiece;
      }
      bool SquareContainsXPiece(unsigned int x, unsigned int y)
      {
         return GetSquare(x,y).GetPiece().IsX();
      }
      bool SquareContainsManPiece(unsigned int x, unsigned int y)
      {
         return GetSquare(x,y).GetPiece().IsMan();
      }
      
      // Functions to query information about the board
      unsigned int Width() const  { return width; }
      unsigned int Height() const { return height; }
      
      bool IsSelected(int x, int y) const { return (x==selectedSquare.x) && (y==selectedSquare.y); }
      bool IsQueuedForExecution(int x, int y) const { return (x==executionSquare.x) && (y==executionSquare.y); }
      
      int SelectedX() const { return selectedSquare.x; }
      int SelectedY() const { return selectedSquare.y; }
      int QueuedX() const { return executionSquare.x; }
      int QueuedY() const { return executionSquare.y; }


   private:

      // --------------------------------------------------------
      // Sub-classes utilised only by CBoard
      // --------------------------------------------------------
      // CSquare class contains a CPiece, which may be an empty piece
      class CSquare
      {
         public:
            CSquare() {}
            CSquare(CPiece &piece) : currentPiece(piece) {}

            CPiece &GetPiece() { return currentPiece; }

         private:

            CPiece currentPiece;
      };

      // Simple struct to store the x & y coordinates of a square
      class CSquareLocation
      {
         public:
            CSquareLocation() : CSquareLocation(-1,-1) {}
            CSquareLocation(const int _x, const int _y) : x(_x), y(_y) {}

            bool operator==(const CSquareLocation &rhs) const
            {
               return (x == rhs.x) && (y == rhs.y);
            }

            bool operator!=(const CSquareLocation &rhs) const
            {
               return (x != rhs.x) || (y != rhs.y);
            }

            int x;
            int y;
      };

      // Class for holding a list of passive moves, a list of aggressive moves, & the location of the moving piece
      class CMoveContainer
      {
         public:
            CMoveContainer()
            {
               // Reserve space for the passiveMoves & aggressiveMoves vectors (max = 4 moves for each)
               passiveMoves.reserve(4);
               aggressiveMoves.reserve(4);
            }

            void Clear()
            {
               //pieceLocation.x = pieceLocation.y = -1;
               passiveMoves.clear();
               aggressiveMoves.clear();
            }

            CSquareLocation pieceLocation;
            std::vector<CSquareLocation> passiveMoves;
            std::vector<CSquareLocation> aggressiveMoves;
      };


   private:

      // --------------------------------------------------------
      // CBoard private data and functions
      // --------------------------------------------------------
      // For now, this constructor is private so that you can only have a 8x8 board with 12 pieces on each side
      CBoard(unsigned int _width,
             unsigned int _height,
             unsigned int _maxPieces)
       : width(_width), height(_height),
         maxPieces(_maxPieces),
         boardLayout(1),
         squares(_width*_height),
         currentTurnAllMoves(_maxPieces),
         aiPersonality(MODERATE)
      {
         emptySquare.GetPiece() = emptyPiece;
         ResetBoard(false);
      }

      unsigned int width;
      unsigned int height;
      unsigned int maxPieces;
      bool boardLayout;

      /*const */CPiece emptyPiece;   // default constructor will make this as an empty piece (type = NONE)
      
      std::vector<CSquare> squares;
      CSquare emptySquare; // Used for out-of-bounds access attempt of GetSquare
      
      CSquare &GetSquare(unsigned int x, unsigned int y)
      {
         if( (x >= width) || (y >= height) ) // Fix for out-of-bounds access attempt
            return emptySquare;
         else
            return squares[(y*width) + x];
      }
      
      CSquare &GetSquare(const CSquareLocation &squareLoc)
      {
         if( (squareLoc.x >= int(width)) || (squareLoc.y >= int(height)) || (squareLoc.x < 0) || (squareLoc.y < 0) ) // Fix for out-of-bounds access attempt
            return emptySquare;
         else
            return squares[(squareLoc.y*width) + squareLoc.x];
      }
      
      // Variable for determining which square is currently selected
      CSquareLocation selectedSquare;
      
      // Variable for determining which square is currently queued for execution
      CSquareLocation executionSquare;
      
      // Storage for the valid moves for the currently selected/queued piece
      CMoveContainer executionSquareMoves;
      // Simple function to clear the currently stored moves
      void ResetMoves() { executionSquareMoves.Clear(); }
      // Function to populate the list of moves for the currently-selected piece
      void PopulateMoves(const CPiece &currentPiece, const bool populatePassive, CMoveContainer &moves);
      // Indicates whether a multi-turn sequence is in action (i.e. an aggressive move has been made & further aggressive moves are available)
      bool multiTurnSequence;
      
      // Function to cancel a multi-turn sequence (reset execution square, reset moves lists, turnover, set mutliTurnSequence to false)
      void CancelMultiTurn();
      
      // Vector of all of the moves for each of the current side's pieces
      std::vector<CMoveContainer> currentTurnAllMoves;
      
      // Function to populate the currentTurnAllMoves vector
      void CalculateAllMoves();
      
      // Bools to signal whether the current side has any moves left (if not, then the game is over)
      // & whether the current side has any aggressive moves available (if so, then they must make an aggressive move)
      bool currentSideHasMoves;
      bool currentSideHasAggressiveMoves;
      
      // Control variables for the AI
      bool aiIsX;
      
      enum PersonlityTypes { MODERATE, GENEROUS, AGGRESSIVE, CAUTIOUS };
      int aiPersonality;
      
      // Random number generator
      CRandomRS rng;
      
      // Recursive function that the AI uses for working out what the best of the available moves is
      int GetTreeScore( CBoard src_board, int depth );
      
      // bool to denote which side's turn is active
      bool isXTurn;

};


#endif
