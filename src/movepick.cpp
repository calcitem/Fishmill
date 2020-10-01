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

#include "movepick.h"

namespace
{

enum Stages
{
    MAIN_TT, CAPTURE_INIT, GOOD_CAPTURE, REFUTATION, QUIET_INIT, QUIET, BAD_CAPTURE,
    PROBCUT_TT, PROBCUT_INIT, PROBCUT,
    QSEARCH_TT, QCAPTURE_INIT, QCAPTURE
};

// partial_insertion_sort() sorts moves in descending order up to and including
// a given limit. The order of moves smaller than the limit is left unspecified.
void partial_insertion_sort(ExtMove *begin, ExtMove *end, int limit)
{
    for (ExtMove *sortedEnd = begin, *p = begin + 1; p < end; ++p)
        if (p->value >= limit) {
            ExtMove tmp = *p, *q;
            *p = *++sortedEnd;
            for (q = sortedEnd; q != begin && *(q - 1) < tmp; --q)
                *q = *(q - 1);
            *q = tmp;
        }
}

} // namespace


/// Constructors of the MovePicker class. As arguments we pass information
/// to help it to return the (presumably) good moves first, to decide which
/// moves to return (in the quiescence search, for instance, we only want to
/// search captures, promotions, and some checks) and how important good move
/// ordering is at the current node.

/// MovePicker constructor for the main search
MovePicker::MovePicker(/* const */ Position &p, Move ttm, Depth d, Move *killers, int pl)
    : pos(p),
    ttMove(ttm), refutations{ {killers[0], 0}, {killers[1], 0}}, depth(d), ply(pl) {

    assert(d > 0);

    stage = (MAIN_TT)+
        !(ttm && pos.pseudo_legal(ttm));
}

/// MovePicker constructor for quiescence search
MovePicker::MovePicker(/* const */ Position &p, Move ttm, Depth d, Square rs)
    : pos(p), ttMove(ttm), recaptureSquare(rs), depth(d)
{
    assert(d <= 0);

    stage = (QSEARCH_TT)+
        !(ttm && (depth > DEPTH_QS_RECAPTURES || to_sq(ttm) == recaptureSquare)
          && pos.pseudo_legal(ttm));
}

/// MovePicker constructor for ProbCut: we generate captures with SEE greater
/// than or equal to the given threshold.
MovePicker::MovePicker(/* const */ Position &p, Move ttm, Value th)
    : pos(p), ttMove(ttm), threshold(th)
{
    stage = PROBCUT_TT + !(ttm && pos.capture(ttm)
                           && pos.pseudo_legal(ttm)
                           && pos.see_ge(ttm, threshold));
}

/// MovePicker::score() assigns a numerical value to each move in a list, used
/// for sorting. Captures are ordered by Most Valuable Victim (MVV), preferring
/// captures with a good history. Quiets moves are ordered using the histories.
template<GenType Type>
void MovePicker::score()
{
    cur = moves;

    while (cur++->move != MOVE_NONE) {
        Move m = cur->move;

        Square sq = to_sq(m);
        Square sqsrc = from_sq(m);

        // if stat before moving, moving phrase maybe from @-0-@ to 0-@-@, but no mill, so need sqsrc to judge
        int nOurMills = pos.in_how_many_mills(sq, pos.side_to_move(), sqsrc);
        int nTheirMills = 0;

#ifndef SORT_MOVE_WITHOUT_HUMAN_KNOWLEDGES
        // TODO: rule.allowRemoveMultiPiecesWhenCloseMultiMill adapt other rules
        if (type_of(m) != MOVETYPE_REMOVE) {
            // all phrase, check if place sq can close mill
            if (nOurMills > 0) {
                cur->value += RATING_ONE_MILL * nOurMills;
            } else if (pos.get_phase() == PHASE_PLACING) {
                // placing phrase, check if place sq can block their close mill
                nTheirMills = pos.in_how_many_mills(sq, ~pos.side_to_move());
                cur->value += RATING_BLOCK_ONE_MILL * nTheirMills;
            }
#if 1
            else if (pos.get_phase() == PHASE_MOVING) {
                // moving phrase, check if place sq can block their close mill
                nTheirMills = pos.in_how_many_mills(sq, ~pos.side_to_move());

                if (nTheirMills) {
                    int nOurPieces = 0;
                    int nTheirPieces = 0;
                    int nBanned = 0;
                    int nEmpty = 0;

                    pos.surrounded_pieces_count(sq, nOurPieces, nTheirPieces, nBanned, nEmpty);

                    if (sq % 2 == 0 && nTheirPieces == 3) {
                        cur->value += RATING_BLOCK_ONE_MILL * nTheirMills;
                    } else if (sq % 2 == 1 && nTheirPieces == 2 && rule.nTotalPiecesEachSide == 12) {
                        cur->value += RATING_BLOCK_ONE_MILL * nTheirMills;
                    }
                }
            }
#endif

            //cur->value += nBanned;  // placing phrase, place nearby ban point

            // for 12 men, white 's 2nd move place star point is as important as close mill (TODO)   
            if (rule.nTotalPiecesEachSide == 12 &&
                pos.count<ON_BOARD>(WHITE) < 2 &&    // patch: only when white's 2nd move
                Position::is_star_square(static_cast<Square>(m))) {
                cur->value += RATING_STAR_SQUARE;
            }
        } else { // Remove
            int nOurPieces = 0;
            int nTheirPieces = 0;
            int nBanned = 0;
            int nEmpty = 0;

            pos.surrounded_pieces_count(sq, nOurPieces, nTheirPieces, nBanned, nEmpty);

               if (nOurMills > 0) {
                // remove point is in our mill
                //cur->value += RATING_REMOVE_ONE_MILL * nOurMills;

                if (nTheirPieces == 0) {
                    // if remove point nearby has no their stone, preferred.
                    cur->value += 1;
                    if (nOurPieces > 0) {
                        // if remove point nearby our stone, preferred
                        cur->value += nOurPieces;
                    }
                }
            }

            // remove point is in their mill
            nTheirMills = pos.in_how_many_mills(sq, ~pos.side_to_move());
            if (nTheirMills) {
                if (nTheirPieces >= 2) {
                    // if nearby their piece, prefer do not remove
                    cur->value -= nTheirPieces;

                    if (nOurPieces == 0) {
                        // if nearby has no our piece, more prefer do not remove
                        cur->value -= 1;
                    }
                }
            }

            // prefer remove piece that mobility is strong 
            cur->value += nEmpty;
        }
#endif // !SORT_MOVE_WITHOUT_HUMAN_KNOWLEDGES
        }
}

/// MovePicker::select() returns the next move satisfying a predicate function.
/// It never returns the TT move.
template<MovePicker::PickType T, typename Pred>
Move MovePicker::select(Pred filter)
{
    while (cur < endMoves) {
        if (T == Best)
            std::swap(*cur, *std::max_element(cur, endMoves));

        if (*cur != ttMove && filter())
            return *cur++;

        cur++;
    }
    return MOVE_NONE;
}

/// MovePicker::next_move() is the most important method of the MovePicker class. It
/// returns a new pseudo legal move every time it is called until there are no more
/// moves left, picking the move with the highest score from a list of generated moves.
Move MovePicker::next_move(bool skipQuiets)
{
    endMoves = generate<LEGAL>(pos, moves);
    moveCount = endMoves - moves;

    score<LEGAL>();
    partial_insertion_sort(moves, endMoves, -100);    // TODO: limit = -3000 * depth

    return *moves;
}
