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

#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include <algorithm>
#include <array>

#include "types.h"

class Position;

enum GenType
{
    CAPTURES,
    QUIETS,
    NON_EVASIONS,
    LEGAL
};

struct ExtMove
{
    Move move;
    int value;

    operator Move() const
    {
        return move;
    }
    void operator=(Move m)
    {
        move = m;
    }

    // Inhibit unwanted implicit conversions to Move
    // with an ambiguity that yields to a compile error.
    operator float() const = delete;
};

inline bool operator<(const ExtMove &f, const ExtMove &s)
{
    return f.value < s.value;
}

template<GenType>
ExtMove *generate(const Position &pos, ExtMove *moveList);

/// The MoveList struct is a simple wrapper around generate(). It sometimes comes
/// in handy to use this class instead of the low level generate() function.
template<GenType T>
struct MoveList
{
    explicit MoveList(const Position &pos) : last(generate<T>(pos, moveList))
    {
    }
    const ExtMove *begin() const
    {
        return moveList;
    }
    const ExtMove *end() const
    {
        return last;
    }
    size_t size() const
    {
        return last - moveList;
    }
    bool contains(Move move) const
    {
        return std::find(begin(), end(), move) != end();
    }

    static void create();
    static void shuffle();

    inline static std::array<Square, FILE_NB *RANK_NB> movePriorityTable {
        SQ_8, SQ_9, SQ_10, SQ_11, SQ_12, SQ_13, SQ_14, SQ_15,
        SQ_16, SQ_17, SQ_18, SQ_19, SQ_20, SQ_21, SQ_22, SQ_23,
        SQ_24, SQ_25, SQ_26, SQ_27, SQ_28, SQ_29, SQ_30, SQ_31,
    };

    inline static Move moveTable[SQUARE_NB][MD_NB] = { {MOVE_NONE} };

private:
    ExtMove moveList[MAX_MOVES], *last;
};

#endif // #ifndef MOVEGEN_H_INCLUDED
