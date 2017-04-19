// Definition of class functions for the draughts board

#include "board.h"

#include <algorithm> // std::find

//#define _DEBUG
// --------------------------------------------------------------------------- //
// Function to reset the pieces to their original positions
// --------------------------------------------------------------------------- //

void
CBoard::ResetBoard(const bool _IsXTurn)
{
   // Create template pieces for X & O that are men
   CPiece pieceX;
   CPiece pieceO;
   pieceX.SetTypeMan();
   pieceX.SetSideX();
   pieceO.SetTypeMan();
   pieceO.SetSideO();
   
   // Assign each piece to a square on the board
   // Start by clearing the board (set all squares to empty)
   for( unsigned int y = 0 ; y < height ; y ++ )
      for( unsigned int x = 0 ; x < width ; x++ )
      {
         GetSquare(x,y).GetPiece() = emptyPiece;
      }
   // Simply assign each piece to its correct starting square
   // (the following is only valid for an 8x8 board with 12 pieces per side)
   // xs
   for( unsigned int x = (boardLayout?1:0) ; x < width ; x+=2 )
      GetSquare(x,0).GetPiece() = pieceX;
   for( unsigned int x = (boardLayout?0:1) ; x < width ; x+=2 )
      GetSquare(x,1).GetPiece() = pieceX;
   for( unsigned int x = (boardLayout?1:0) ; x < width ; x+=2 )
      GetSquare(x,2).GetPiece() = pieceX;
   // os
   for( unsigned int x = (boardLayout?0:1) ; x < width ; x+=2 )
      GetSquare(x,height-1).GetPiece() = pieceO;
   for( unsigned int x = (boardLayout?1:0) ; x < width ; x+=2 )
      GetSquare(x,height-2).GetPiece() = pieceO;
   for( unsigned int x = (boardLayout?0:1) ; x < width ; x+=2 )
      GetSquare(x,height-3).GetPiece() = pieceO;
   
   // Reset the currently-selected square
   SelectSquare(width/2, height/2);
   
   // Reset the square set for execution to be none
   executionSquare.x = -1;
   executionSquare.y = -1;
   
   // Reset the vectors of moves
   ResetMoves();
   
   // Reset multiTurnSequence
   multiTurnSequence = false;
   
   isXTurn = _IsXTurn;
   
   // Finally calculate what all the moves are for the current (1st) player
   CalculateAllMoves();
}


// --------------------------------------------------------------------------- //
// Function to execute on a selected square
//   - If the square is occupied by a piece belonging to the player whose turn it is, it queues that piece for execution & populates the list of valid moves
//   - If the square is empty & a piece has been queued for execution, then it moves the queued piece to the selected square (if the move is valid)
//   - If the square is empty & no piece is queued for execution, it does nothing
//   - Takes an indicator of whose turn it is as an argument, and modifies this if the current action ends the current player's turn
// --------------------------------------------------------------------------- //
// Return code: 0 = OK, 1 = invalid piece/square selected, 2 = invalid attempted move, 3 = invalid attempted move (aggressive move is available)
int
CBoard::ExecuteSelectedSquare()
{
   int returnCode = 0;

   // For convenience, get a reference to the piece in the currently-selected square (which may be an "empty" piece)
   const CPiece &currentPiece = GetSquare(selectedSquare).GetPiece();

   // If the current square has already been queued for execution, then de-select it (reset the execution square)
   //    (Cannot change/deselect the currently selected piece if we're in a mutli-turn sequence)
   if( (executionSquare == selectedSquare) && !multiTurnSequence )
   {
      executionSquare.x = -1;
      executionSquare.y = -1;
      ResetMoves();
   }
   // If the current square contains a piece that belongs to the player whose turn it is, then queue this square for execution
   //   (overwriting any previous square that was set for execution)
   //    (Cannot change the currently selected piece if we're in a mutli-turn sequence)
   else if( (currentPiece != emptyPiece) && (isXTurn == currentPiece.IsX()) && !multiTurnSequence )
   {
      executionSquare = selectedSquare;
      executionSquareMoves.pieceLocation = executionSquare;
      // Populate the agressive & passive moves
      PopulateMoves(currentPiece, true, executionSquareMoves);
      
      multiTurnSequence = false;  // Not yet a multi-turn sequence (should not be necessary)
   }
   // If a different square has already been queued for execution,
   //   and the current selected square square a valid square to move into, //is empty,
   //   then move the piece from the execution square into the selected square
   else if( ( (executionSquare.x >= 0) && (executionSquare.x < int(width)) && (executionSquare.y >= 0) && (executionSquare.y < int(height)) ) )
//            && (currentPiece == emptyPiece) )
   {
      // Try to find the current move/destination square (selectedSquare) in the passive & aggressive moves vectors
      // Note: it should be IMPOSSIBLE for the move to appear in both the passive & aggressive lists
      std::vector<CSquareLocation>::iterator findPassiveIt , findAggressiveIt;
      findPassiveIt = std::find( executionSquareMoves.passiveMoves.begin() , executionSquareMoves.passiveMoves.end() , selectedSquare );
      findAggressiveIt = std::find( executionSquareMoves.aggressiveMoves.begin() , executionSquareMoves.aggressiveMoves.end() , selectedSquare );
      
      // If the attempted move was passive, but there are aggressive moves available, then reject the attempted passive move
      if( currentSideHasAggressiveMoves && findPassiveIt != executionSquareMoves.passiveMoves.end() )
      {
         //std::cout << "\n~~~~ The attempted move is not permitted: An Aggressive/Jump moves is available and must be taken ~~~~\n\n";
         returnCode = 3;
      }
      // If the selectedSquare is one of the valid moves, then move the piece & remove an opposing piece, if necessary
      //   Also, if the pieceToMove was a man piece then transform it into a king if it has reached the end row
      else if( (findPassiveIt != executionSquareMoves.passiveMoves.end()) || (findAggressiveIt != executionSquareMoves.aggressiveMoves.end()) )
      {
         CPiece pieceToMove = GetSquare(executionSquare).GetPiece();
         GetSquare(selectedSquare).GetPiece() = pieceToMove;
         GetSquare(executionSquare).GetPiece() = emptyPiece;
         
         // Check whether the piece should be transformed into a king (crowned)
         bool pieceCrowned = false;
         if( pieceToMove.IsMan() &&
             ( (  pieceToMove.IsX()  && (selectedSquare.y == int(height)-1) ) ||  // x-man that has reached the bottom row
               (!(pieceToMove.IsX()) && (selectedSquare.y == 0       ) ) ) ) // o-man that has reached the top row
         {
            GetSquare(selectedSquare).GetPiece().SetTypeKing();
            pieceCrowned = true;
         }
         
         // If the move was aggressive, then empty the opposing piece from the "skipped" square & check whether we are now in a multi-turn sequence
         if( findAggressiveIt != executionSquareMoves.aggressiveMoves.end() )
         {
            GetSquare((selectedSquare.x+executionSquare.x)/2 , (selectedSquare.y+executionSquare.y)/2).GetPiece() = emptyPiece;

            // If the move was aggressive, then check whether more aggressive moves are possible for this piece
            executionSquare = selectedSquare;
            executionSquareMoves.pieceLocation = executionSquare;
            PopulateMoves(pieceToMove, false, executionSquareMoves); // This will reset the moves vectors & just repopulate the aggressive moves vector
            // If there are more aggressive moves, then signal that we're in a multi-turn sequence, but ONLY if the piece has NOT been crowned
            if( executionSquareMoves.aggressiveMoves.size() > 0 && !pieceCrowned)
            {
               multiTurnSequence = true;
            }
            // If there are no more aggressive moves or the piece has been crowned, then reset the executionSquare
            else
            {
               // Reset the passive & aggressive moves (should not be necessary)
               // There are no more aggressive moves to make so signal the end of the current player's turn
               CancelMultiTurn();
            }
         }
         // Move was passive
         else
         {
            // Set the executionSquare to none
            CancelMultiTurn();
         }
      }
      else
      {
         // Could print out an error message here (i.e. the move is not permitted)
         //std::cout << "\n~~~~ The attempted move is not permitted ~~~~\n\n";
         returnCode = 2;
#ifdef _DEBUG
         std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
         Draw();
         std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
#endif
      }
      
   }
   else
   {
      // Otherwise an empty square or the wrong player's piece is being queued for execution, which does nothing
      //std::cout << "\n~~~~ Please select an " << (isXTurn?"\'x\' or \'X\'":"\'o\' or \'O\'") << " piece ~~~~\n\n";
      returnCode = 1;
      multiTurnSequence = false;  // Not a multi-turn sequence (should not be necessary)
   }

   return returnCode;
}


// --------------------------------------------------------------------------- //
// Function to populate the lists of possible moves
// Assumes that currentPiece is in executionSquare - if that is not the case, then the resultant moves lists will be incorrect
// --------------------------------------------------------------------------- //

void
CBoard::PopulateMoves(const CPiece &currentPiece, const bool populatePassive, CMoveContainer &moves)
{
   // Perhaps need a check here that currentPiece is not empty
   
   // Clear the lists of possible moves before adding to them
   moves.Clear();
   //ResetMoves();

   const int xm1 = moves.pieceLocation.x-1;
   const int xp1 = moves.pieceLocation.x+1;
   const int ym1 = moves.pieceLocation.y-1;
   const int yp1 = moves.pieceLocation.y+1;
   // Passive moves - only if the square to move into is unoccupied):
   //   If piece is an 'o' then it can move into a square offset by coordinate (+/-1 , -1) (diagonally 'up' the board)
   //   If piece is an 'x' then it can move into a square offset by coordinate (+/-1 , +1) (diagonally 'down' the board)
   //   If piece is a king, then it can move into a square offset by coordinate (+/-1 , +/-1) (diagonally 'up' or 'down')
   // Aggressive moves
   //   If one of the above passive moves is blocked by an opposing piece, then check to see whether 
   //     the square diagonally behind the opposing piece is empty - if it is, then that square is 
   //     added to the list of aggressive moves

   if( !(currentPiece.IsMan()) || currentPiece.IsX() )   // King or X-man can move +ve y
   {
      if( xm1>=0 && yp1<int(height) ) //(-1,+1)
      {
         // Add (-1,+1) to passive moves if square is empty
         if( GetSquare(xm1,yp1).GetPiece() == emptyPiece )
         {
            if( populatePassive )
               moves.passiveMoves.push_back(CSquareLocation(xm1,yp1));
         }
         // Add (-2,+2) to aggressive moves if (-1,+1) contains an opposition piece, and (-2,+2) is in bounds, and (-2,+2) is empty
         else if( ( GetSquare(xm1,yp1).GetPiece().IsX() != currentPiece.IsX() ) && (xm1>0 && yp1<(int(height)-1)) && (GetSquare(xm1-1,yp1+1).GetPiece() == emptyPiece) )
            moves.aggressiveMoves.push_back(CSquareLocation(xm1-1,yp1+1));
      }
      if( xp1<width && yp1<int(height) ) //(+1,+1)
      {
         // Add (+1,+1) to passive moves if square is empty
         if( GetSquare(xp1,yp1).GetPiece() == emptyPiece )
         {
            if( populatePassive )
               moves.passiveMoves.push_back(CSquareLocation(xp1,yp1));
         }
         // Add (+2,+2) to aggressive moves if (+1,+1) contains an opposition piece, and (+2,+2) is in bounds, and (+2,+2) is empty
         else if( ( GetSquare(xp1,yp1).GetPiece().IsX() != currentPiece.IsX() ) && (xp1<(int(width)-1) && yp1<(int(height)-1)) && (GetSquare(xp1+1,yp1+1).GetPiece() == emptyPiece) )
            moves.aggressiveMoves.push_back(CSquareLocation(xp1+1,yp1+1));
      }
   }

   if( !(currentPiece.IsMan()) || !(currentPiece.IsX()) )   // King or O-man can move -ve y
   {
      if( xm1>=0 && ym1>=0 )     //(-1,-1)
      {
         // Add (-1,-1) to passive moves if square is empty
         if( GetSquare(xm1,ym1).GetPiece() == emptyPiece )
         {
            if( populatePassive )
               moves.passiveMoves.push_back(CSquareLocation(xm1,ym1));
         }
         // Add (-2,-2) to aggressive moves if (-1,-1) contains an opposition piece, and (-2,-2) is in bounds, and (-2,2) is empty
         else if( ( GetSquare(xm1,ym1).GetPiece().IsX() != currentPiece.IsX() ) && (xm1>0 && ym1>0) && (GetSquare(xm1-1,ym1-1).GetPiece() == emptyPiece) )
            moves.aggressiveMoves.push_back(CSquareLocation(xm1-1,ym1-1));
      }
      if( xp1<int(width) && ym1>=0 )     //(+1,-1)
      {
         // Add (+1,-1) to passive moves if square is empty
         if( GetSquare(xp1,ym1).GetPiece() == emptyPiece )
         {
            if( populatePassive )
               moves.passiveMoves.push_back(CSquareLocation(xp1,ym1));
         }
         // Add (+2,-2) to aggressive moves if (+1,-1) contains an opposition piece, and (+2,-2) is in bounds, and (+2,-2) is empty
         else if( ( GetSquare(xp1,ym1).GetPiece().IsX() != currentPiece.IsX() ) && (xp1<(int(width)-1) && ym1>0) && (GetSquare(xp1+1,ym1-1).GetPiece() == emptyPiece) )
            moves.aggressiveMoves.push_back(CSquareLocation(xp1+1,ym1-1));
      }
   }

}


// --------------------------------------------------------------------------- //
// Function to cancel a multi-turn sequence
// --------------------------------------------------------------------------- //

void
CBoard::CancelMultiTurn()
{
   executionSquare.x = executionSquare.y = -1;
   // Reset the passive & aggressive moves (should not be necessary)
   ResetMoves();
   // The multi-turn sequence has ended so signal the end of the current player's turn
   isXTurn = !isXTurn;
   // Not a multi-turn sequence
   multiTurnSequence = false;
   // Re-calculate all moves for the next turn
   CalculateAllMoves();
}


// --------------------------------------------------------------------------- //
// Function to populate the currentTurnAllMoves vector
// --------------------------------------------------------------------------- //

void
CBoard::CalculateAllMoves()
{
   // Clear all of the existing moves & piece locations in the currentTurnAllMoves vector
   for( unsigned int pieceIndex = 0 ; pieceIndex < currentTurnAllMoves.size() ; pieceIndex++ )
   {
      currentTurnAllMoves[pieceIndex].Clear();
      currentTurnAllMoves[pieceIndex].pieceLocation = CSquareLocation(-1,-1);
   }
   
   unsigned int pieceIndex = 0;
   unsigned int passiveMovesAvailable = 0;
   unsigned int aggressiveMovesAvailable = 0;
   // Loop through all squares in the board
   // If a square contains a piece whose side is the same as isXTurn, then populate the moves for that piece
   
   for(unsigned int y = 0 ; y < height ; y++)
   {
      for(unsigned int x = 0 ; x < width ; x++)
      {
         const CPiece &currentPiece = GetSquare(x,y).GetPiece();
         // If the current piece is not empty and it is the turn of the current piece's side, then populate the moves & increment the pieceIndex
         if( (currentPiece != emptyPiece) &&
             (currentPiece.IsX() == isXTurn) )
         {
            currentTurnAllMoves[pieceIndex].pieceLocation.x = x;
            currentTurnAllMoves[pieceIndex].pieceLocation.y = y;

            PopulateMoves(currentPiece, true, currentTurnAllMoves[pieceIndex]);
            
            // Update the number of available moves
            passiveMovesAvailable += currentTurnAllMoves[pieceIndex].passiveMoves.size();
            aggressiveMovesAvailable += currentTurnAllMoves[pieceIndex].aggressiveMoves.size();
            
            // Increment the index
            pieceIndex++;
         }
      }
   }

   currentSideHasMoves = (passiveMovesAvailable + aggressiveMovesAvailable) > 0;
   currentSideHasAggressiveMoves = aggressiveMovesAvailable > 0;
}


// --------------------------------------------------------------------------- //
// Function to invoke the AI to take a relevant action
// --------------------------------------------------------------------------- //

bool
CBoard::InvokeAI( int depth )
{
   // Signal for success or failure of the AI routine
   bool aiSuccess = true;
   
   // Signal which pieces the AI is controlling, so that the "score" calculation at each AI depth can be estimated
   aiIsX = isXTurn;
   
   // We can only decide on a move if there are moves available to make
   if( (!multiTurnSequence && CurrentSideHasMoves()) || (multiTurnSequence && executionSquareMoves.aggressiveMoves.size()>0) )
   {
      // - If any aggressive moves exist, then the options to explore are only the aggressive moves
      // - Else only passive moves exist, so the options to explore are only the passive moves
      // - Now create a vector of scores, the size of which is equal to the number of options that are to be explored
      unsigned int numMoves = 0;
      // If we're in a multi-turn sequence then the piece to move is fixed & the possible destination squares are in executionSquareMoves
      if( multiTurnSequence )
      {
         numMoves = executionSquareMoves.aggressiveMoves.size();
      }
      else if( currentSideHasAggressiveMoves )
      {
         for( unsigned int piece = 0 ; piece < currentTurnAllMoves.size() ; piece++ )
            numMoves += currentTurnAllMoves[piece].aggressiveMoves.size();
      }
      else
      {
         for( unsigned int piece = 0 ; piece < currentTurnAllMoves.size() ; piece++ )
            numMoves += currentTurnAllMoves[piece].passiveMoves.size();
      }
      std::vector<int> scores(numMoves);
      // - Create a vector of CBoards of the same size (each of which is a copy of this CBoard), and set the executionSquare 
      //     to the piece that is to be moved and the selectedSquare to the move that is to be made
      std::vector<CBoard> boards(numMoves, *this);
      if( multiTurnSequence )
      {
         unsigned int boardIndex = 0;
         for( unsigned int move = 0 ; move < executionSquareMoves.aggressiveMoves.size() ; move++ )
         {
            boards[boardIndex].selectedSquare = executionSquareMoves.aggressiveMoves[move];
            boardIndex++;
         }
      }
      else if( currentSideHasAggressiveMoves )
      {
         //std::cout << "(InvokeAI, aggressive board pop.) numMoves = " << numMoves << ", " << std::endl;
         unsigned int boardIndex = 0;
         for( unsigned int piece = 0 ; piece < currentTurnAllMoves.size() ; piece++ )
            for( unsigned int move = 0 ; move < currentTurnAllMoves[piece].aggressiveMoves.size() ; move++ )
            {
               boards[boardIndex].selectedSquare = currentTurnAllMoves[piece].pieceLocation;
               boards[boardIndex].ExecuteSelectedSquare();
               boards[boardIndex].selectedSquare = currentTurnAllMoves[piece].aggressiveMoves[move];
               boardIndex++;
            }
      }
      else
      {
         unsigned int boardIndex = 0;
         for( unsigned int piece = 0 ; piece < currentTurnAllMoves.size() ; piece++ )
            for( unsigned int move = 0 ; move < currentTurnAllMoves[piece].passiveMoves.size() ; move++ )
            {
               boards[boardIndex].selectedSquare = currentTurnAllMoves[piece].pieceLocation;
               boards[boardIndex].ExecuteSelectedSquare();
               boards[boardIndex].selectedSquare = currentTurnAllMoves[piece].passiveMoves[move];
               boardIndex++;
            }
      }
      // - For each option, pass the CBoard into the GetTreeScore function, and assign the function's return value to the 
      //     relevant score vector entry
      std::vector<int> maxScore;
      maxScore.reserve(numMoves);
      maxScore.push_back(-1024);

      std::vector<unsigned int> maxScoreIndex;
      maxScoreIndex.reserve(numMoves);
      maxScoreIndex.push_back(0);

      for( unsigned int option = 0 ; option < numMoves ; option++ )
      {
#ifdef _DEBUG
         std::cout << "(InvokeAI) Evaluating option " << option << std::endl;
         boards[option].Draw();
#endif
         scores[option] = GetTreeScore( boards[option] , /*isXTurn ,*/ depth );
         //exit(0);
         if( scores[option] > maxScore[0] )
         {
            // Clear any previous max scores (needed in case there were multiple options with the same max score)
            maxScore.clear();
            maxScoreIndex.clear();
            // Update the max score & index
            maxScore.push_back(scores[option]);
            maxScoreIndex.push_back(option);
         }
         else if( scores[option] == maxScore[0] )  // multiple options with the same max score
         {
            maxScore.push_back(scores[option]);
            maxScoreIndex.push_back(option);
         }
      }
      // Test which score is the highest - in the case of a draw, select a random one from amongst the best.
      unsigned int optionToSelect = 0;
      if( maxScore.size() == 0 ) // Something has gone very wrong...
      {
         aiSuccess = false;
      }
      else if( maxScore.size() > 1 )   // Need to randomly pick an option
      {
         // Set the range of the random number generator
         rng.SetRange(0,maxScore.size()-1);
         // Select a random index
         optionToSelect = maxScoreIndex[rng.GetNumber()];
      }
      else
      {
         optionToSelect = maxScoreIndex[0];
      }
      // Set this version of the CBoard's executionSquare & selectedSquare to those of the highest score, and call 
      //   ExecuteSelectedSquare( isXTurn )
      if( aiSuccess )
      {
#ifdef _DEBUG
         std::cout << "(InvokeAI) Selected option " << optionToSelect << ": ("
                   <<boards[optionToSelect].executionSquare.x<<","<<boards[optionToSelect].executionSquare.y<<") -> ("
                   <<boards[optionToSelect].selectedSquare.x <<","<<boards[optionToSelect].selectedSquare.y <<")" << std::endl;
#endif

         // Only need to select the piece to move if we are not in a multi-turn sequence
         if( !multiTurnSequence )
         {
            selectedSquare = boards[optionToSelect].executionSquare;
            ExecuteSelectedSquare( /*isXTurn*/ );
         }
         selectedSquare = boards[optionToSelect].selectedSquare;
         //ExecuteSelectedSquare( /*isXTurn*/ );
      }
      
   }
   // No moves are available, which is a failure state for the AI
   else
   {
      aiSuccess = false;
   }
   
   return aiSuccess;
   
   // This function will replace InvokeAI

   // This function wants to look at all the possible moves for the current board configuration & choose one of them to execute
   // It does this by calling GetTreeScore for all of either the aggressive or passive moves in the currentTurnAllMoves vector
   //   and then setting the executionSquare and selectSquare of the move with the best resultant score & executing
   
   // The aim is to create a list of CMoveContainers, each of which contains the CSquareLocation of the piece to move & 
   //   either a list of aggressiveMoves or a list of passiveMoves (the other list must be empty)

   // The number of possible options can be obtained by iterating through currentTurnAllMoves & summing the size of the 
   //   aggressiveMoves & passiveMoves vectors for each piece
   // If any aggressive moves exist, then the options to explore are only the aggressive moves
   // Else If any passive moves exist, then the options to explore are only the passive moves
   // Else no moves exist, which is a failure state
   // Now create a vector of scores, the size of which is equal to the number of options that are to be explored
   // Create a vector of CBoards of the same size (each of which is a copy of this CBoard), and set the executionSquare 
   //   to the piece that is to be moved and the selectedSquare to the move that is to be made
   // For each option, pass the CBoard into the GetTreeScore function, and assign the function's return value to the 
   //   relevant score vector entry
   // Test which score is the highest - in the case of a draw, select a random one from amongst the best.
   // Set this version of the CBoard's executionSquare & selectedSquare to those of the highest score, and call 
   //   ExecuteSelectedSquare( isXTurn )

}


// --------------------------------------------------------------------------- //
// Function to evaluate all of the possible moves for the input board configuration
// and return the score of
// --------------------------------------------------------------------------- //

int
CBoard::GetTreeScore( CBoard src_board , int depth )
{
   //std::cout << "(GetTreeScore, depth " << depth << ") Executing move (" 
   //          << src_board.executionSquare.x << "," << src_board.executionSquare.y << ") -> ("
   //          << src_boardsrc_board.selectedSquare.x  << "," << src_board.selectedSquare.y  << ")"<< std::endl;
   // src_board will have the executionSquare and selectedSquare already set
   // Call src_board.ExecuteSelectedSquare(isXTurn) on the src_board
   src_board.ExecuteSelectedSquare();
   // If we've entered into a multi-turn sequence with the above move, then don't calculate all moves (it's already been done)
   //if( !src_board.multiTurnSequence )
   //{
   //   // Call src_board.CalculateAllMoves(isXTurn) (this will populate all possible moves for the current src_board)
   //   src_board.CalculateAllMoves(/*isXTurn*/);
   //}
   // If we have reached the maximum search depth, or there are no more moves left (i.e. someone has won the game),
   //   then calculate the score & return it
   if( (--depth < 0) || !(src_board.CurrentSideHasMoves()) )
   {
      int score = 0;
      // If src_board.CurrentSideHasMoves() == false, then +/- 100 depending on which side has won
      if( !(src_board.CurrentSideHasMoves()) )
      {
         // If it is the AI's turn & there are no moves left, then this is a bad move, so -100
         score = ( (src_board.IsXTurn() == aiIsX) ? -100 : 100 );
      }
      else
      {
         // Go through all of the squares in the src_board & +1 if it is the ai's piece, & -1 if it is the opponent's piece (& +0 if it is empty)
         for(unsigned int y = 0 ; y < height ; y++)
         {
            for(unsigned int x = 0 ; x < width ; x++)
            {
               const CPiece currentPiece = src_board.GetSquare(x,y).GetPiece();
               if( currentPiece != emptyPiece )
               {
                  if( (currentPiece.IsX() && aiIsX) || (!(currentPiece.IsX()) && !aiIsX) )
                     score += 1;
                  else
                     score -= 1;
               }
            }
         }
      }
      // If the AI personality type is generous, then invert the score, so that the AI makes the worst possible move...
      if( aiPersonality == GENEROUS )
         score = -score;

      return score;
   }
   // The src_board has some moves available & we have not reached the final depth
   else
   {
      // - If any aggressive moves exist, then the options to explore are only the aggressive moves
      // - Else only passive moves exist, so the options to explore are only the passive moves
      // - Now create a vector of scores, the size of which is equal to the number of options that are to be explored
      unsigned int numMoves = 0;
      // If we're in a multi-turn sequence then the piece to move is fixed & the possible destination squares are in executionSquareMoves
      if( src_board.multiTurnSequence )
      {
         numMoves = src_board.executionSquareMoves.aggressiveMoves.size();
      }
      else if( src_board.currentSideHasAggressiveMoves )
      {
         for( unsigned int piece = 0 ; piece < src_board.currentTurnAllMoves.size() ; piece++ )
            numMoves += src_board.currentTurnAllMoves[piece].aggressiveMoves.size();
      }
      else
      {
         for( unsigned int piece = 0 ; piece < src_board.currentTurnAllMoves.size() ; piece++ )
            numMoves += src_board.currentTurnAllMoves[piece].passiveMoves.size();
      }
      //std::vector<int> scores(numMoves);
      // - Create a vector of CBoards of the same size (one for each move, each of which is a copy of this CBoard), and set
      //     the executionSquare to the piece that is to be moved and the selectedSquare to the move that is to be made
      std::vector<CBoard> boards(numMoves, src_board);
      if( src_board.multiTurnSequence )
      {
         // Multi-turn sequence means that the piece to move is already selected & only the square to move to needs to be set
         unsigned int boardIndex = 0;
         for( unsigned int move = 0 ; move < src_board.executionSquareMoves.aggressiveMoves.size() ; move++ )
         {
            boards[boardIndex].selectedSquare = src_board.executionSquareMoves.aggressiveMoves[move];
            boardIndex++;
         }
      }
      else if( src_board.currentSideHasAggressiveMoves )
      {
         unsigned int boardIndex = 0;
         for( unsigned int piece = 0 ; piece < src_board.currentTurnAllMoves.size() ; piece++ )
            for( unsigned int move = 0 ; move < src_board.currentTurnAllMoves[piece].aggressiveMoves.size() ; move++ )
            {
               boards[boardIndex].selectedSquare = src_board.currentTurnAllMoves[piece].pieceLocation;
               boards[boardIndex].ExecuteSelectedSquare();
               boards[boardIndex].selectedSquare = src_board.currentTurnAllMoves[piece].aggressiveMoves[move];
               boardIndex++;
            }
      }
      else
      {
         unsigned int boardIndex = 0;
         for( unsigned int piece = 0 ; piece < src_board.currentTurnAllMoves.size() ; piece++ )
            for( unsigned int move = 0 ; move < src_board.currentTurnAllMoves[piece].passiveMoves.size() ; move++ )
            {
               boards[boardIndex].selectedSquare = src_board.currentTurnAllMoves[piece].pieceLocation;
               boards[boardIndex].ExecuteSelectedSquare();
               boards[boardIndex].selectedSquare = src_board.currentTurnAllMoves[piece].passiveMoves[move];
               boardIndex++;
            }
      }
      
      // - For each option, pass the CBoard into the GetTreeScore function, and assign the function's return value to the 
      //     relevant score vector entry
      
      // Evaluate the board & come up with a score based on the AI's personality type: MODERATE, GENEROUS, AGGRESSIVE, CAUTIOUS
      if( aiPersonality == CAUTIOUS )
      {
         int minScore = 1024;
         for( unsigned int option = 0 ; option < numMoves ; option++ )
         {
            int score = GetTreeScore( boards[option] , depth );
            if( score > minScore )
               minScore = score;
         }
         // - Return the lowest score (it does not matter which move it came from)
         return minScore;
      }
      else if( aiPersonality == AGGRESSIVE )
      {
         // Return the sum of the scores (i.e. overall quality of this path)
         int sumOfScores = 0;
         for( unsigned int option = 0 ; option < numMoves ; option++ )
            sumOfScores += GetTreeScore( boards[option] , depth );
   
         return sumOfScores; //sumOfScores/numMoves;
      }
      else //if( aiPersonality == MODERATE ) // || (aiPersonality == GENEROUS)
      {
         int maxScore = -1024;
         for( unsigned int option = 0 ; option < numMoves ; option++ )
         {
#ifdef _DEBUG
            std::cout << "(GetTreeScore, depth " << depth << ") evaluating option " << option << std::endl;
            boards[option].Draw();
#endif
            int score = GetTreeScore( boards[option] , depth );
#ifdef _DEBUG
            std::cout << " ^^ score = " << score << std::endl;
#endif
            if( score > maxScore )
               maxScore = score;
         }
         // - Return the highest score (it does not matter which move it came from)
         return maxScore;
      }
   }
   
   // src_board will have the executionSquare and selectedSquare already set
   // Call src_board.ExecuteSelectedSquare(isXTurn) on the src_board
   // Call src_board.CalculateAllMoves(isXTurn) (this will populate all possible moves for the current src_board)
   // If depth < 0 or src_board.CurrentSideHasMoves() == false, then calculate the score & return it:
   //   - Count the number of non-empty pieces in piecesX
   //   - Count the number of non-empty pieces in piecesO
   //   - score = (no. non-empty piecesX) - (no. non-empty piecesO)
   // Else
   //   - If any aggressive moves exist, then the options to explore are only the aggressive moves
   //   - Else only passive moves exist, so the options to explore are only the passive moves
   //   - Now create a vector of scores, the size of which is equal to the number of options that are to be explored
   //   - Create a vector of CBoards of the same size (each of which is a copy of this CBoard), and set the executionSquare 
   //       to the piece that is to be moved and the selectedSquare to the move that is to be made
   //   - For each option, pass the CBoard into the GetTreeScore function, and assign the function's return value to the 
   //       relevant score vector entry
   //   - Return the highest score (it does not matter which move it came from)
   
}




