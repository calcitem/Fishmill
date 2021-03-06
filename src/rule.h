﻿/*
  Sanmill, a mill game playing engine derived from NineChess 1.5
  Copyright (C) 2015-2018 liuweilhy (NineChess author)
  Copyright (C) 2019-2020 Calcitem <calcitem@outlook.com>

  Sanmill is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Sanmill is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RULE_H
#define RULE_H

#include "types.h"

struct Rule
{
    const char *name;
    const char *description;
    int nTotalPiecesEachSide;   // 9 or 12
    int nPiecesAtLeast; // Default is 3
    bool hasObliqueLines;
    bool hasBannedLocations;
    bool isDefenderMoveFirst;
    bool allowRemoveMultiPiecesWhenCloseMultiMill;
    bool allowRemovePieceInMill;
    bool isBlackLosebutNotDrawWhenBoardFull;
    bool isLoseButNotChangeSideWhenNoWay;
    bool allowFlyWhenRemainThreePieces;
    int maxStepsLedToDraw;
};

#define N_RULES 4
extern const struct Rule RULES[N_RULES];
extern const struct Rule *rule;

#endif /* RULE_H */

