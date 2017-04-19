// Simple Random Number Generator Class

#ifndef _RANDOMRS_H
#define _RANDOMRS_H

#include <random>
#include <chrono>
#include <algorithm>


class CRandomRS
{
   public:
      // Constructors
      CRandomRS() { e1 = initialize_twister(); }

      // Function to set the range of random numbers to generate
      void SetRange(const int lower_incl, const int upper_incl)
      {
         uniform_dist = std::uniform_int_distribution<int>(lower_incl, upper_incl);
      }

      // Function to get a random number within the specified range
      int GetNumber()
      {
         return uniform_dist(e1);
      }
      
      CRandomRS &operator=(const CRandomRS &rhs)
      {
         uniform_dist = rhs.uniform_dist;
         return *this;
      }
      
   private:
      
      std::mt19937 e1;
      std::uniform_int_distribution<int> uniform_dist;

      // Function to set up the random number generator with a random seeding
      std::mt19937 initialize_twister( std::size_t seed = std::time(nullptr) )
      {
         static constexpr std::size_t NUM_DISCARD = 1024;

         std::minstd_rand lcg(seed);
         lcg.discard(NUM_DISCARD);

         std::size_t seeds[ std::mt19937::state_size ];
         std::generate_n( seeds , std::mt19937::state_size , lcg );

         try
         {
            // check if there is a random_device available
            seeds[0] = std::random_device {}();
         }
         catch (const std::exception&)
         {
            /* ignore */
         }

         std::seed_seq seed_sequence( std::begin(seeds) , std::end(seeds) );
         return std::mt19937 { seed_sequence }; // warm-up with seed seed_sequence.generate()
      }
};

#endif
