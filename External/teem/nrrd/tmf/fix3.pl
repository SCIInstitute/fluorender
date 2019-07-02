#!/usr/bin/perl -w
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

# for most of the TMF filters, the parm vector is not used, and
# fix2.pl assumes as much by putting in an AIR_UNUSED declaration.
# This takes out the AIR_UNUSED for the filters that do actually
# use the parm vector (thanks to do the nearby kernel identification)

while (<>) {
    s|AIR_UNUSED\(parm\); /\* TMF_dn_cn_3ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_dn_c1_4ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_cn_2ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_cn_4ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_c0_3ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d1_c0_4ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d2_cn_3ef \*/||g;
    s|AIR_UNUSED\(parm\); /\* TMF_d2_c1_4ef \*/||g;
    print;
}
