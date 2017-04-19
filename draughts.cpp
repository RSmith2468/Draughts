// Console-based game of Draughts
// g++ draughts.cpp board.cpp -o draughts.exe -std=c++11


#include <iostream>
#include <limits>
#include "board.h"
#include "randomrs.h"   // Random number generator for deciding who goes first

enum commands
{
   UP,
   DOWN,
   LEFT,
   RIGHT,
   SELECT,
   EXIT,
   RESET,
   YES,
   NO,
   NUM_COMMANDS
};


char GetUserInput()
{
   char userChar;
   userChar = std::cin.get();
   std::cin.ignore( std::numeric_limits<std::streamsize>::max(), '\n' );
   //std::cin >> userChar;
   return userChar;
}

void
DrawSpacer(const unsigned int width, const bool selectedRow, const int selectedX)
{
   for(int x = 0 ; x < int(width) ; x++)
   {
      std::cout << "+" << ( (selectedRow && (x == selectedX)) ?"+":"-");
   }
   std::cout << "+";
}

void
Draw(CBoard &board)
{   
   for(unsigned int y = 0 ; y < board.Height() ; y++)
   {
      // Draw a spacer at the top of this row
      DrawSpacer( board.Width(), ((board.SelectedY() == y) || (board.SelectedY()+1 == y)), board.SelectedX() );
      std::cout << "\n";
      // Draw the squares for this row
      for(unsigned int x = 0 ; x < board.Width() ; x++)
      {
         if( board.IsQueuedForExecution(x,y) || board.IsQueuedForExecution(x-1,y) )
            std::cout << ":";
         else if( board.IsSelected(x,y) || board.IsSelected(x-1,y) )
            std::cout << "+";
         else
            std::cout << "|";
         // Work out what to draw in the square
         if( board.SquareIsEmpty(x,y) )
            std::cout << " ";
         else if( board.SquareContainsManPiece(x,y) )
            std::cout << (board.SquareContainsXPiece(x,y)?"x":"o");
         else
            std::cout << (board.SquareContainsXPiece(x,y)?"X":"O");
      }
      if( board.IsSelected(board.Width()-1,y) )
         std::cout << "+";
      else
         std::cout << "|";
      std::cout << "\n";
   }
   // Draw a spacer at the bottom of the board
   DrawSpacer(board.Width(), (board.SelectedY() == board.Height()-1), board.SelectedX() );
   std::cout << "\n";
}


int main()
{
   // random number generator to use later in the program
   CRandomRS rng;
   //// Set the range of the random number generator
   //rng.SetRange(1,10);  // will generate a number between 1 and 10 inclusive
   //// Generate a random number
   //rng.GetNumber();
   
   std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~";
   std::cout << "\n~~Welcome to Rob's Draughts!~~";
   std::cout << "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
   // Create the checkers board
   CBoard board;
   
   //bool isXTurn = true; // Initialise the game to X having the first turn
   //bool prevLoopIsXTurn = true;//false; // Used to check whether a turnover has occurred (if so, CalculateAllMoves must be called)
   
   // If the AI routine fails for any reason, then the game will exit
   bool aiSuccess = true;
   // The AI's intelligence is the depth of the search tree that it will evaluate
   int aiIntelligence = 5;
   
   // Bool to trigger asking for the game initialisation options
   bool isNewGame = true;
   bool singlePlayer = false; // bool to signal whether it is a 1-player or 2-player game
   bool aiIsX = true;         // bool to signal which side the AI is controlling (if 1-player game)
   
   // Define the letters for each command
   constexpr char userInput[NUM_COMMANDS] = {'w',  // UP,
                                             's',  // DOWN,
                                             'a',  // LEFT,
                                             'd',  // RIGHT,
                                             ' ',  // SELECT,
                                             'q',  // EXIT,
                                             'r',  // RESET,
                                             'y',  // YES,
                                             'n'}; // NO,

   char command = userInput[RESET]; // Command from the user
   do
   {
      // Draw the board
      //board.Draw();
      Draw(board);

      if( isNewGame )
      {
         // Query how many players (only affects the behaviour of the code in this function)
         std::cout << "\nPlease enter the number of players followed by return (1/2): ";
         command = GetUserInput();
         if( command == '1' )
         {
            singlePlayer = true;
            // Query which side is the AI (only affects the behaviour of the code in this function)
            std::cout << "Which side is the AI opponent (x/o)? ";
            command = GetUserInput();
            if( (command == 'x') || (command == 'X') )
            {
               aiIsX = true;
            }
            else  // Don't bother checking for 'o' being entered - just assume that it was
            {
               aiIsX = false;
            }

            // Query what the personality of the AI should be
            std::cout << "AI personality types are: Moderate(m) / Aggressive(a) / Cautious(c) / Generous(g) / Stupid(s)\n";
            std::cout << "What type of AI would you like to face (m/a/c/g/s)? ";
            command = GetUserInput();

            aiIntelligence = 5;  // Default AI intelligence is 5 - will be overridden if a stupid AI is chosen

            switch( command )
            {
               case 'm':
                  board.SetAIModerate();
                  break;
               case 'a':
                  board.SetAIAggressive();
                  break;
               case 'c':
                  board.SetAICautious();
                  break;
               case 'g':
                  board.SetAIGenerous();
                  break;
               default: // case 's'
                  // Acknowledge if an invalid type was selected, and default to a stupid AI
                  if( command != 's' )
                     std::cout << "AI type \"" << command << "\" not recognised - preparing a stupid AI for you\n";
                  board.SetAIModerate();
                  aiIntelligence = 0;
                  break;
            }
         }
         else  // Don't bother checking for '2' being entered - just assume that it was
         {
            singlePlayer = false;
         }
         
         // Randomly select which side goes first
         rng.SetRange(0,1);  // will generate a number between 0 and 1 inclusive
         bool turn = rng.GetNumber();  // Generate a random number & convert it to a bool for the turn
         board.ForceTurn( turn );
         //prevLoopIsXTurn = !turn;
         
         isNewGame = false;
      }

      // // At the beginning of each new turn, calculate all of the possible moves for all of the pieces the current side
      // if( prevLoopIsXTurn != board.IsXTurn() )
      //    board.CalculateAllMoves(/*isXTurn*/);
      
      //prevLoopIsXTurn = board.IsXTurn();

      // Check to see if any moves are left for the current player - if not, then the other player has won the game
      if( !(board.CurrentSideHasMoves()) )
         std::cout << "\n\n~~~~ " << (board.IsXTurn()?"O":"X") << " Wins!!" << " ~~~~\n";

      if( singlePlayer && (board.IsXTurn() == aiIsX) && board.CurrentSideHasMoves() )
      {
         // It is the AI's turn
         std::cout << "\n~~~~ AI\'s turn. ~~~~\n";
         
         aiSuccess = board.InvokeAI( /*isXTurn ,*/ aiIntelligence );
         board.ExecuteSelectedSquare( /*isXTurn*/ );
      }
      else
      {
         // Prompt the user for a command
         std::cout << "\n\'"<<userInput[UP]     <<"\' = up, "
                      <<"\'"<<userInput[DOWN]   <<"\' = down, "
                      <<"\'"<<userInput[LEFT]   <<"\' = left, "
                      <<"\'"<<userInput[RIGHT]  <<"\' = right, "
                      <<"\'"<<userInput[SELECT] <<"\' = select piece/square, "
                      <<"\'"<<userInput[RESET]  <<"\' = reset board, "
                      <<"\'"<<userInput[EXIT]   <<"\' = quit\n";
         std::cout << (board.IsXTurn()?"X":"O") << "\'s turn. Please enter a command, followed by return: ";
         command = GetUserInput();
         std::cout << std::endl;
         switch(command)
         {
            case userInput[UP]:
               board.MoveSelectSquareUp();
               break;

            case userInput[DOWN]:
               board.MoveSelectSquareDown();
               break;

            case userInput[LEFT]:
               board.MoveSelectSquareLeft();
               break;

            case userInput[RIGHT]:
               board.MoveSelectSquareRight();
               break;

            case userInput[SELECT]:
            {
               int returnCode = board.ExecuteSelectedSquare(/*isXTurn*/);
               switch (returnCode)
               {
                  case 1:
                     std::cout << "\n~~~~ Please select an " << (board.IsXTurn()?"\'x\' or \'X\'":"\'o\' or \'O\'") << " piece ~~~~\n\n";
                     break;
                  case 2:
                     std::cout << "\n~~~~ The attempted move is not permitted ~~~~\n\n";
                     break;
                  case 3:
                     std::cout << "\n~~~~ The attempted move is not permitted: An Aggressive/Jump moves is available and must be taken ~~~~\n\n";
                     break;
                  default: //case 0:
                     break;
               }
               break;
            }
            case userInput[RESET]:
            {
               std::cout << "Are you sure you would like to reset the board? ("<<userInput[YES]<<"/"<<userInput[NO]<<"): ";
               char confirmation = GetUserInput();
               if( confirmation == userInput[YES] )
               {
                  board.ResetBoard(true);
                  //isXTurn = true;
                  //prevLoopIsXTurn = false;
               }
               // Don't bother checking if userInput[NO] was input, or whether some other character was used
               isNewGame = true;
               break;
            }

            case userInput[EXIT]:
            {
               std::cout << "Are you sure you would like to quit? ("<<userInput[YES]<<"/"<<userInput[NO]<<"): ";
               char confirmation = GetUserInput();
               if( confirmation != userInput[YES] )
                  command = '\t';   // Just some random character to stop this while loop from exiting
               // Don't bother checking if userInput[NO] was input, or whether some other character was used
               break;
            }

            default:
               std::cout << "\n~~~~ Error: Command \""<<command<<"\" not recognised ~~~~\n\n";
               break;
         }
      }
   } while( (command != userInput[EXIT]) && aiSuccess );
   
   
   return 0;
}

