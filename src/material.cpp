/*
  Fishmill, a UCI Mill Game playing engine derived from Stockfish
  Copyright (C) 2004-2008 Tord Romstad (Glaurung author)
  Copyright (C) 2008-2015 Marco Costalba, Joona Kiiski, Tord Romstad (Stockfish author)
  Copyright (C) 2015-2020 Marco Costalba, Joona Kiiski, Gary Linscott, Tord Romstad (Stockfish author)
  Copyright (C) 2020 Calcitem <calcitem@outlook.com>

  Fishmill is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Fishmill is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>
#include <cstring>   // For std::memset

#include "material.h"
#include "thread.h"

using namespace std;

namespace {

  // Polynomial material imbalance parameters

  constexpr int QuadraticOurs[][PIECE_TYPE_NB] = {
    //            OUR PIECES
    // pair pawn knight bishop rook queen
    {1438                               }, // Bishop pair
    {  40,   38                         }, // Pawn
    {  32,  255, -62                    }, // Knight      OUR PIECES
    {   0,  104,   4,    0              }, // Bishop
    { -26,   -2,  47,   105,  -208      }, // Rook
    {-189,   24, 117,   133,  -134, -6  }  // Queen
  };

  constexpr int QuadraticTheirs[][PIECE_TYPE_NB] = {
    //           THEIR PIECES
    // pair pawn knight bishop rook queen
    {   0                               }, // Bishop pair
    {  36,    0                         }, // Pawn
    {   9,   63,   0                    }, // Knight      OUR PIECES
    {  59,   65,  42,     0             }, // Bishop
    {  46,   39,  24,   -24,    0       }, // Rook
    {  97,  100, -42,   137,  268,    0 }  // Queen
  };

  /// imbalance() calculates the imbalance by comparing the piece count of each
  /// piece type for both colors.
  template<Color Us>
  int imbalance(const int pieceCount[][PIECE_TYPE_NB]) {

    constexpr Color Them = ~Us;

    int bonus = 0;

    // Second-degree polynomial material imbalance, by Tord Romstad
    for (int pt1 = NO_PIECE_TYPE; pt1 <= BAN; ++pt1)
    {
        if (!pieceCount[Us][pt1])
            continue;

        int v = 0;

        for (int pt2 = NO_PIECE_TYPE; pt2 <= pt1; ++pt2)
            v +=  QuadraticOurs[pt1][pt2] * pieceCount[Us][pt2]
                + QuadraticTheirs[pt1][pt2] * pieceCount[Them][pt2];

        bonus += pieceCount[Us][pt1] * v;
    }

    return bonus;
  }

} // namespace

namespace Material {

} // namespace Material
