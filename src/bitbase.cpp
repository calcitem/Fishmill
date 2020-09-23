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
#include <vector>
#include <bitset>

#include "bitboard.h"
#include "types.h"

namespace
{

// There are 24 possible pawn squares: files A to D and ranks from 2 to 7.
// Positions with the pawn on files E to H will be mirrored before probing.
constexpr unsigned MAX_INDEX = 2 * 24 * 64 * 64; // stm * psq * wksq * bksq = 196608

std::bitset<MAX_INDEX> KPKBitbase;

// A KPK bitbase index is an integer in [0, IndexMax] range
//
// Information is mapped in a way that minimizes the number of iterations:
//
// bit  0- 5: white king square (from SQ_A1 to SQ_H8)
// bit  6-11: black king square (from SQ_A1 to SQ_H8)
// bit    12: side to move (WHITE or BLACK)
// bit 13-14: white pawn file (from FILE_A to FILE_D)
// bit 15-17: white pawn RANK_7 - rank (from RANK_7 - RANK_7 to RANK_7 - RANK_2)
unsigned index(Color stm, Square bksq, Square wksq, Square psq)
{
    return int(wksq) | (bksq << 6) | (stm << 12) | (file_of(psq) << 13) | ((RANK_7 - rank_of(psq)) << 15);
}

enum Result
{
    INVALID = 0,
    UNKNOWN = 1,
    DRAW = 2,
    WIN = 4
};

Result &operator|=(Result &r, Result v)
{
    return r = Result(r | v);
}

} // namespace


bool Bitbases::probe(Square wksq, Square wpsq, Square bksq, Color stm)
{
    return KPKBitbase[index(stm, bksq, wksq, wpsq)];
}


void Bitbases::init()
{
    /* TODO */
}
