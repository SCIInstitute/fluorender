# This variable will help provide a master list of all the sources.
# Add new source files here.
SET(BIFF_SOURCES
  privateBiff.h
  biff.h
  biffmsg.c
  biffbiff.c
  )

ADD_TEEM_LIBRARY(biff ${BIFF_SOURCES})
