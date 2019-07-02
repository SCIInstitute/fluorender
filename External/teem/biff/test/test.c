/*
  Teem: Tools to process and visualize scientific data and images             .
  Copyright (C) 2011, 2010, 2009  University of Chicago
  Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this library; if not, write to Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "../biff.h"

int
main() {
  char *tmp, *s1, *s2;
  biffMsg *msg1, *msg2;

  /*
  biffAdd("axis", "the first error axis");
  biffAdd("axis", "the second error axis");
  biffAdd("axis", "the third error axis");
  biffAdd("chard", "the first error chard");
  biffAdd("chard", "the second error chard");
  biffAdd("chard", "the third error chard");
  biffAdd("bingo", "zero-eth bingo message");
  biffMove("bingo", NULL, "chard");
  biffAdd("bingo", "the first error bingo");
  biffAdd("bingo", "the second bll boo boo boo error bingo");
  biffAdd("bingo", "the third error bingo");
  printf("%s\n", (tmp = biffGet("bingo")));
  free(tmp);
  biffDone("bingo");
  printf("%s\n", (tmp = biffGet("chard")));
  free(tmp);
  biffDone("chard");
  printf("%s\n", (tmp = biffGet("axis")));
  free(tmp);
  biffDone("axis");

  biffAdd("harold", "the first error harold");
  biffAdd("harold", "the second error harold");
  biffAdd("harold", "the third error harold");
  printf("%s\n", (tmp = biffGet("harold")));
  free(tmp);
  */

  biffAdd("axis", "the first error axis");
  biffAdd("axis", "the second error axis");
  biffAdd("axis", "the third error axis");
  biffAdd("axis", "the fourth error axis");
  biffAdd("axis", "the fifth error axis");
  printf("%s", (tmp = biffGet("axis")));
  free(tmp);
  biffDone("axis");

  biffAdd("axo", "the first error axis");
  biffAdd("axo", "the second error axis");
  biffAdd("axo", "the third error axis");
  biffAdd("axo", "the fourth error axis");
  biffAdd("axo", "the fifth error axis");
  printf("%s", (tmp = biffGetDone("axo")));
  free(tmp);

  printf("=================================\n");
  msg1 = biffMsgNew("roberts");
  biffMsgAdd(msg1, "biffMsgAdd hello, said roberts");
  biffMsgAddf(msg1, "biffMsgAddf: there's an int %d and a float %g",
              42, AIR_PI);
  s1 = biffMsgStrGet(msg1);
  printf("from msg1:\n%s", s1);
  s1 = airFree(s1);
  msg2 = biffMsgNew("sue");
  biffMsgAdd(msg2, "biffMsgAdd hi from sue");
  biffMsgAddf(msg2, "biffMsgAddf: another float %g", AIR_PI*AIR_PI);
  s2 = biffMsgStrGet(msg2);
  printf("from msg2:\n%s", s2);
  s2 = airFree(s2);
  biffMsgMovef(msg1, msg2, "biffMsgMovef: good int %d", 10);
  s1 = biffMsgStrGet(msg1);
  printf("from msg1:\n%s", s1);
  s1 = airFree(s1);
  printf("=================================\n");
  msg1 = biffMsgNix(msg1);
  msg2 = biffMsgNix(msg2);

  /*
  biffAddf("test", "%s: this is a test %d %f", "me", 1, 2.0);
  printf("%s\n", (tmp = biffGet("test")));
  free(tmp);
  biffDone("test");
  */

  exit(0);
}



