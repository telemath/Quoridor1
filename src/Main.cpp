#include <algorithm>
#include <iostream>
#include <string>
#ifndef __BIGUNSIGNED_HPP__
#include "BigUnsigned.hpp"
#endif
#ifndef __BIGINTEGERUTILS_HPP__
#include "BigIntegerUtils.hpp"
#endif
using namespace std;

// Define the parameters of the game.
const unsigned __int32 COLUMN_COUNT = 8;
const unsigned __int32 ROW_COUNT = 8;
const unsigned __int32 MAX_FENCE_COUNT = 20;

// Other constants, pre-calculated for convenience.
const unsigned __int32 SIGNATURE_DIMENSION = (1 << COLUMN_COUNT);
const unsigned __int32 MAX_VERTICAL_SIGNATURE = SIGNATURE_DIMENSION - 1;
const unsigned __int32 FENCE_DIMENSION = MAX_FENCE_COUNT + 1;

// rows [vertical_signature][fence_count] = number of ways to get that vertical signature with that fence count.
// The vertical signature is the binary value of the fence arrangement, using 1 for a vertical fence and 0 for a horizontal fence or no fence.
// Let . denote an empty intersection, | denote a vertical fence, and - denote a horizontal fence.
// With 8 columns, rows [17][4] = 11, because there are 11 ways to place 4 fences with a vertical signature of 17:
// -.-|...|,  -..|-..|,  -..|.-.|,  -..|..-|,  .-.|-..|,  .-.|.-.|,  .-.|..-|,  ..-|-..|,  ..-|.-.|,  ..-|..-|, and ...|-.-|
// Note that two adjacent columns can not both have horizontal fences.
unsigned __int32 rows [SIGNATURE_DIMENSION][FENCE_DIMENSION];

// For counting the ways to fill an entire board, filling one row at a time.
// grid [vertical_signature][fence_count] stores the number of ways to fill
// the board with some number of (unspecified) rows, and the given fence_count,
// where the last row has the given vertical_signature.
// (Global because VS complained about putting so much memory on the stack.)
BigUnsigned grid_a[SIGNATURE_DIMENSION][FENCE_DIMENSION];
BigUnsigned grid_b[SIGNATURE_DIMENSION][FENCE_DIMENSION];


// Depth-first search for all possible arrangements of fences.
void countNewRows (unsigned __int32 vertical_signature,
                   unsigned __int32 columns_filled,
                   unsigned __int32 fence_count,
                   bool             last_column_was_horizontal)
{
   if (columns_filled == COLUMN_COUNT)
   {
      ++rows [vertical_signature][fence_count];
   }
   else
   {
      // Regardless of the previous column, the current column can always be empty.
      countNewRows (vertical_signature, // Empty column doesn't change vertical signature.
                    columns_filled + 1, // Filled one more column.
                    fence_count,        // Empty column doesn't change fence_count.
                    false);             // Last column was empty, not horizontal.

      // Regardless of the previous column, the current column can have a vertical fence.
      countNewRows (vertical_signature | (1 << columns_filled), // Set the associated bit in the vertical signature.
                    columns_filled + 1,                         // Filled one more column.
                    fence_count + 1,                            // Added a fence.
                    false);                                     // Last column was vertical, not horizontal.

      // Only allow adding a horizontal fence if the previous column doesn't have a horizontal fence.
      if (!last_column_was_horizontal)
      {
         countNewRows (vertical_signature, // Horizontal fence doesn't change vertical signature.
                       columns_filled + 1, // Filled one more column.
                       fence_count + 1,    // Added a fence.
                       true);              // Last column was horizontal.
      }
   }
}


// Add a row to the current grid calculations.
void addRow (BigUnsigned previous_grid[][FENCE_DIMENSION], BigUnsigned current_grid[][FENCE_DIMENSION], unsigned __int32 row)
{
   unsigned __int32 max_prev_fence_count = row * COLUMN_COUNT;

   // Clear the current grid.
   for (unsigned __int32 vertical_signature = 0; vertical_signature <= MAX_VERTICAL_SIGNATURE; ++vertical_signature)
   {
      for (unsigned __int32 fence_count = 0; fence_count <= MAX_FENCE_COUNT; ++fence_count)
      {
         current_grid [vertical_signature][fence_count] = 0;
      }
   }

   // To create the current grid, add the number of ways to add each row to the previous grid.
   for (unsigned __int32 new_vertical_signature = 0; new_vertical_signature <= MAX_VERTICAL_SIGNATURE; ++new_vertical_signature)
   {
      // Vertical signature of previous_grid - only selects values that are compatible with the new vertical signature.
      for (unsigned __int32 prev_vertical_signature = 0;
           prev_vertical_signature <= MAX_VERTICAL_SIGNATURE;
           prev_vertical_signature = ((prev_vertical_signature | new_vertical_signature) + 1) & ~new_vertical_signature)
      {
         // Consider all possible previous fence counts.
         for (unsigned __int32 prev_fence_count = 0; prev_fence_count <= max_prev_fence_count; ++prev_fence_count)
         {
            if (previous_grid [prev_vertical_signature][prev_fence_count] != 0)
            {
               // Consider all possible new fence counts, up to the given MAX_FENCE_COUNT.
               for (unsigned __int32 added_fence_count = 0;
                    (added_fence_count <= COLUMN_COUNT) && (added_fence_count + prev_fence_count <= MAX_FENCE_COUNT);
                   ++added_fence_count)
               {
                  if (rows [new_vertical_signature][added_fence_count] != 0)
                  {
                     current_grid [new_vertical_signature][prev_fence_count + added_fence_count] +=
                         previous_grid [prev_vertical_signature][prev_fence_count] * rows [new_vertical_signature][added_fence_count];
                  }
               }
            }
         }
      }
   }
}


// Output the results.
// The output is formatted to be saved to a .csv file and read in Excel.
void outputResults (BigUnsigned final_grid[][FENCE_DIMENSION])
{
    cout << "For a " << COLUMN_COUNT << "x" << ROW_COUNT << " Quoridor board with up to " << MAX_FENCE_COUNT << " fences." << endl;
    cout << "Fences,Ways" << endl;

    BigUnsigned total = 0;

    for (unsigned __int32 fence_count = 0; fence_count <= MAX_FENCE_COUNT; ++fence_count)
    {
        BigUnsigned count = 0;

        for (unsigned vertical_signature = 0; vertical_signature <= MAX_VERTICAL_SIGNATURE; ++vertical_signature)
        {
            count += final_grid [vertical_signature][fence_count];
            total += final_grid [vertical_signature][fence_count];
        }

        // Use ="#" or Excel will only show the first 15 significant digits.  Stupid Excel.
        cout << fence_count << ",=\"" << bigUnsignedToString (count).c_str () << "\"" << endl;
    }

    cout << "Total,=\"" << bigUnsignedToString (total).c_str () << "\"" << endl;
}


// Calculate the number of ways to place each number of fences in a Quoridor game.
void quoridorCalc (void)
{
    memset (rows, 0, sizeof(unsigned __int32) * (SIGNATURE_DIMENSION) * (FENCE_DIMENSION));
    countNewRows(0, 0, 0, false);
  
    // Initialize to no rows filled: 1 way to have no fences and no vertical signature.
    grid_a [0][0] = 1;

    // Fill each subsequent row from the previous row.
    for (unsigned __int32 row = 0; row < ROW_COUNT; ++row)
    {
        if ((row & 1) == 0)
        {
            addRow(grid_a, grid_b, row);
        }
        else
        {
            addRow(grid_b, grid_a, row);
        }
    }

    // Output final results.
    outputResults(((ROW_COUNT & 1) == 0) ? grid_a : grid_b);
}


// Parse command line arguments, then call quoridorCalc to do the work.
int main (int argc, char **argv)
{
    quoridorCalc (); 
    return 0;
}
