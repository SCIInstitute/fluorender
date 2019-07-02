#!/usr/bin/perl 
#
# Teem: Tools to process and visualize scientific data and images              
# Copyright (C) 2012, 2011, 2010, 2009  University of Chicago
# Copyright (C) 2008, 2007, 2006, 2005  Gordon Kindlmann
# Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public License
# (LGPL) as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The terms of redistributing and/or modifying this software also
# include exceptions to the LGPL that facilitate static linking.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
#


$emit = 1;
while (<>) {

    # nix carraige returns
    s///g;
    
    # nix pragmas
    s/\#pragma warning.+$//g;

    if (/w[pm][0-9][a-z] = /) {
	chomp;         # nix newline
	s/ //g;        # nix spaces 
	s/;$//g;       # nix last semicolon
	foreach $pair (split /;/) {
	    ($var, $val) = split /=/, $pair;
	    $$var = $val;
	}
	next;
    }

    # nix the minimal function body before and after the switch lines
    s/  float result;//g;
    s/  int i;//g;
    s/  i = \(t\<0\) \? \(int\)t-1:\(int\)t;//g;
    s/  t = t - i;//g;
    s/  switch \(i\) \{//g;
    s/  \}//g;
    s/  return result;//g;

    # print lines that aren't all whitespace
    s/^ +$//g;
    if (!m/^$/) {
	print;
    }
}
