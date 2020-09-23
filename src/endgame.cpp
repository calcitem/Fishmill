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

#include "bitboard.h"
#include "endgame.h"
#include "movegen.h"

namespace
{

// Used to drive the king towards the edge of the board
// in KX vs K and KQ vs KR endgames.
inline int push_to_edge(Square s)
{
    int rd = edge_distance(rank_of(s)), fd = edge_distance(file_of(s));
    return 90 - (7 * fd * fd / 2 + 7 * rd * rd / 2);
}

// Used to drive the king towards A1H8 corners in KBN vs K endgames.
inline int push_to_corner(Square s)
{
    return abs(7 - rank_of(s) - file_of(s));
}

// Drive a piece close to or away from another piece
inline int push_close(Square s1, Square s2)
{
    return 140 - 20 * distance(s1, s2);
}
inline int push_away(Square s1, Square s2)
{
    return 120 - push_close(s1, s2);
}

// Map the square as if strongSide is white and strongSide's only pawn
// is on the left half of the board.
Square normalize(const Position &pos, Color strongSide, Square sq)
{
    assert(pos.count<STONE>(strongSide) == 1);

    if (file_of(pos.square<STONE>(strongSide)) >= FILE_C)
        sq = flip_file(sq);

    return strongSide == WHITE ? sq : flip_rank(sq);
}

} // namespace


namespace Endgames
{
std::pair<Map<Value>, Map<ScaleFactor>> maps;

void init()
{

}
}
