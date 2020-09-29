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

#include <algorithm>
#include <cassert>
#include <cstring>   // For std::memset
#include <iomanip>
#include <sstream>

#include "bitboard.h"
#include "evaluate.h"
#include "material.h"
#include "thread.h"

namespace Trace
{

enum Tracing
{
    NO_TRACE, TRACE
};

enum Term
{
    // The first 8 entries are reserved for PieceType
    MATERIAL = 8, IMBALANCE, MOBILITY, THREAT, PASSED, SPACE, INITIATIVE, TOTAL, TERM_NB
};

Score scores[TERM_NB][COLOR_NB];

double to_cp(Value v)
{
    return double(v) / StoneValue;
}

void add(int idx, Color c, Score s)
{
    scores[idx][c] = s;
}

void add(int idx, Score w, Score b = SCORE_ZERO)
{
    scores[idx][WHITE] = w;
    scores[idx][BLACK] = b;
}

std::ostream &operator<<(std::ostream &os, Score s)
{
    os << std::setw(5) << to_cp(mg_value(s)) << " "
        << std::setw(5) << to_cp(eg_value(s));
    return os;
}

std::ostream &operator<<(std::ostream &os, Term t)
{
    if (t == MATERIAL || t == IMBALANCE || t == INITIATIVE || t == TOTAL)
        os << " ----  ----" << " | " << " ----  ----";
    else
        os << scores[t][WHITE] << " | " << scores[t][BLACK];

    os << " | " << scores[t][WHITE] - scores[t][BLACK] << "\n";
    return os;
}
}

using namespace Trace;

namespace
{

// Threshold for lazy and space evaluation
constexpr Value LazyThreshold = Value(1400);
constexpr Value SpaceThreshold = Value(12222);

#define S(mg, eg) make_score(mg, eg)
#undef S

// Evaluation class computes and stores attacks tables and other working data
template<Tracing T>
class Evaluation
{

public:
    Evaluation() = delete;
    explicit Evaluation(/* const */ Position &p) : pos(p)
    {
    }
    Evaluation &operator=(const Evaluation &) = delete;
    Value value();

private:
    template<Color Us> void initialize();
    template<Color Us, PieceType Pt> Score pieces();
    template<Color Us> Score threats() const;
    template<Color Us> Score space() const;
    Score initiative(Score score) const;

    /* const */ Position &pos;
    Bitboard mobilityArea[COLOR_NB];
    Score mobility[COLOR_NB] = { SCORE_ZERO, SCORE_ZERO };

    // attackedBy[color][piece type] is a bitboard representing all squares
    // attacked by a given color and piece type. Special "piece types" which
    // is also calculated is ALL_PIECES.
    Bitboard attackedBy[COLOR_NB][PIECE_TYPE_NB];

    // attackedBy2[color] are the squares attacked by at least 2 units of a given
    // color, including x-rays. But diagonal x-rays through pawns are not computed.
    Bitboard attackedBy2[COLOR_NB];

    // kingRing[color] are the squares adjacent to the king plus some other
    // very near squares, depending on king position.
    Bitboard kingRing[COLOR_NB];

    // kingAttackersCount[color] is the number of pieces of the given color
    // which attack a square in the kingRing of the enemy king.
    int kingAttackersCount[COLOR_NB];

    // kingAttackersWeight[color] is the sum of the "weights" of the pieces of
    // the given color which attack a square in the kingRing of the enemy king.
    // The weights of the individual piece types are given by the elements in
    // the KingAttackWeights array.
    int kingAttackersWeight[COLOR_NB];

    // kingAttacksCount[color] is the number of attacks by the given color to
    // squares directly adjacent to the enemy king. Pieces which attack more
    // than one square are counted multiple times. For instance, if there is
    // a white knight on g5 and black's king is on g8, this white knight adds 2
    // to kingAttacksCount[WHITE].
    int kingAttacksCount[COLOR_NB];
};


// Evaluation::initialize() computes king and pawn attacks, and the king ring
// bitboard for a given color. This is done at the beginning of the evaluation.
template<Tracing T> template<Color Us>
void Evaluation<T>::initialize()
{
    // Squares occupied by those pawns, by our king or queen, by blockers to attacks on our king
    // or controlled by enemy pawns are excluded from the mobility area.
    mobilityArea[Us] = 0;
}


// Evaluation::pieces() scores pieces of a given color and type
template<Tracing T> template<Color Us, PieceType Pt>
Score Evaluation<T>::pieces()
{
    constexpr Color     Them = ~Us;
    constexpr Bitboard OutpostRanks = (Us == WHITE ? Rank4BB | Rank5BB | Rank6BB
                                       : Rank5BB | Rank4BB | Rank3BB);
    const Square *pl = pos.squares<Pt>(Us);

    Score score = SCORE_ZERO;

    /* TODO */

    if (T)
        Trace::add(Pt, Us, score);

    return score;
}


// Evaluation::threats() assigns bonuses according to the types of the
// attacking and the attacked pieces.
template<Tracing T> template<Color Us>
Score Evaluation<T>::threats() const
{
    Score score = SCORE_ZERO;

    /* TODO */

    if (T)
        Trace::add(THREAT, Us, score);

    return score;
}


// Evaluation::space() computes the space evaluation for a given side. The
// space evaluation is a simple bonus based on the number of safe squares
// available for minor pieces on the central four files on ranks 2--4. Safe
// squares one, two or three squares behind a friendly pawn are counted
// twice. Finally, the space bonus is multiplied by a weight. The aim is to
// improve play on game opening.

template<Tracing T> template<Color Us>
Score Evaluation<T>::space() const
{
    Score score = SCORE_ZERO;

    /* TODO */

    if (T)
        Trace::add(SPACE, Us, score);

    return score;
}


// Evaluation::initiative() computes the initiative correction value
// for the position. It is a second order bonus/malus based on the
// known attacking/defending status of the players.

template<Tracing T>
Score Evaluation<T>::initiative(Score score) const
{
    /* TODO */

    if (T)
        Trace::add(INITIATIVE, score);

    return score;
}


// Evaluation::value() is the main function of the class. It computes the various
// parts of the evaluation and returns the value of the position from the point
// of view of the side to move.

template<Tracing T>
Value Evaluation<T>::value()
{
    Value value = VALUE_ZERO;

    int nPiecesInHandDiff;
    int nPiecesOnBoardDiff;
    int pieceCountNeedRemove;

    switch (pos.get_phase()) {
    case PHASE_READY:
        break;

    case PHASE_PLACING:
        nPiecesInHandDiff = pos.pieces_count_in_hand(BLACK) - pos.pieces_count_in_hand(WHITE);
        value += nPiecesInHandDiff * VALUE_EACH_PIECE_INHAND;

        nPiecesOnBoardDiff = pos.pieces_count_on_board(BLACK) - pos.pieces_count_on_board(WHITE);
        value += nPiecesOnBoardDiff * VALUE_EACH_PIECE_ONBOARD;

        switch (pos.get_action()) {
        case ACTION_SELECT:
        case ACTION_PLACE:
            break;

        case ACTION_REMOVE:
            pieceCountNeedRemove = (pos.side_to_move() == BLACK) ?
                pos.piece_count_need_remove() : -(pos.piece_count_need_remove());
            value += pieceCountNeedRemove * VALUE_EACH_PIECE_PLACING_NEEDREMOVE;
            break;
        default:
            break;
        }

        break;

    case PHASE_MOVING:
        value = pos.pieces_count_on_board(BLACK) * VALUE_EACH_PIECE_ONBOARD -
            pos.pieces_count_on_board(WHITE) * VALUE_EACH_PIECE_ONBOARD;

#ifdef EVALUATE_MOBILITY
        value += pos.get_mobility_diff(position->turn, position->pieceCountInHand[BLACK], position->pieceCountInHand[WHITE], false) * 10;
#endif  /* EVALUATE_MOBILITY */

        switch (pos.get_action()) {
        case ACTION_SELECT:
        case ACTION_PLACE:
            break;

        case ACTION_REMOVE:
            pieceCountNeedRemove = (pos.side_to_move() == BLACK) ?
                pos.piece_count_need_remove() : -(pos.piece_count_need_remove());
            value += pieceCountNeedRemove * VALUE_EACH_PIECE_MOVING_NEEDREMOVE;
            break;
        default:
            break;
        }

        break;

    case PHASE_GAMEOVER:
        if (pos.pieces_count_on_board(BLACK) + pos.pieces_count_on_board(WHITE) >=
            RANK_NB * FILE_NB) {
            if (rule.isBlackLosebutNotDrawWhenBoardFull) {
                value -= VALUE_MATE;
            } else {
                value = VALUE_DRAW;
            }
        } else if (pos.get_action() == ACTION_SELECT &&
                   pos.is_all_surrounded() &&
                   rule.isLoseButNotChangeSideWhenNoWay) {
            Value delta = pos.side_to_move() == BLACK ? -VALUE_MATE : VALUE_MATE;
            value += delta;
        }

        else if (pos.pieces_count_on_board(BLACK) < rule.nPiecesAtLeast) {
            value -= VALUE_MATE;
        } else if (pos.pieces_count_on_board(WHITE) < rule.nPiecesAtLeast) {
            value += VALUE_MATE;
        }

        break;

    default:
        break;
    }

    if (pos.side_to_move() == WHITE) {
        value = -value;
    }

    return value;
}

} // namespace


/// evaluate() is the evaluator for the outer world. It returns a static
/// evaluation of the position from the point of view of the side to move.

Value Eval::evaluate(/* const */ Position &pos)
{
    return Evaluation<NO_TRACE>(pos).value();
}


/// trace() is like evaluate(), but instead of returning a value, it returns
/// a string (suitable for outputting to stdout) that contains the detailed
/// descriptions and values of each evaluation term. Useful for debugging.

std::string Eval::trace(/* const */ Position &pos)
{
    std::memset(scores, 0, sizeof(scores));

    pos.this_thread()->contempt = SCORE_ZERO; // Reset any dynamic contempt

    Value v = Evaluation<TRACE>(pos).value();

    v = pos.side_to_move() == WHITE ? v : -v; // Trace scores are from white's point of view

    std::stringstream ss;
    ss << std::showpoint << std::noshowpos << std::fixed << std::setprecision(2)
        << "     Term    |    White    |    Black    |    Total   \n"
        << "             |   MG    EG  |   MG    EG  |   MG    EG \n"
        << " ------------+-------------+-------------+------------\n"
        << "    Material | " << Term(MATERIAL)
        << "   Imbalance | " << Term(IMBALANCE)
        << "    Mobility | " << Term(MOBILITY)
        << "     Threats | " << Term(THREAT)
        << "      Passed | " << Term(PASSED)
        << "       Space | " << Term(SPACE)
        << "  Initiative | " << Term(INITIATIVE)
        << " ------------+-------------+-------------+------------\n"
        << "       Total | " << Term(TOTAL);

    ss << "\nTotal evaluation: " << to_cp(v) << " (white side)\n";

    return ss.str();
}
