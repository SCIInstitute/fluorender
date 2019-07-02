# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = teem

TEMPLATE = lib
CONFIG += staticlib c++17

HEADERS = \
   $$PWD/air/air.h \
   $$PWD/air/privateAir.h \
   $$PWD/biff/biff.h \
   $$PWD/biff/privateBiff.h \
   $$PWD/hest/hest.h \
   $$PWD/hest/privateHest.h \
   $$PWD/nrrd/nrrd.h \
   $$PWD/nrrd/nrrdDefines.h \
   $$PWD/nrrd/nrrdEnums.h \
   $$PWD/nrrd/nrrdMacros.h \
   $$PWD/nrrd/privateNrrd.h \
   $$PWD/teemPng.h

SOURCES = \
   $$PWD/air/test/bessy.c \
   $$PWD/air/test/doubleprint.c \
   $$PWD/air/test/floatprint.c \
   $$PWD/air/test/fp.c \
   $$PWD/air/test/logrice.c \
   $$PWD/air/test/tarr.c \
   $$PWD/air/test/tdio.c \
   $$PWD/air/test/texp.c \
   $$PWD/air/test/tline.c \
   $$PWD/air/test/tmop.c \
   $$PWD/air/test/tok.c \
   $$PWD/air/test/tprint.c \
   $$PWD/air/test/trand.c \
   $$PWD/air/754.c \
   $$PWD/air/array.c \
   $$PWD/air/dio.c \
   $$PWD/air/endianAir.c \
   $$PWD/air/enum.c \
   $$PWD/air/heap.c \
   $$PWD/air/math.c \
   $$PWD/air/miscAir.c \
   $$PWD/air/mop.c \
   $$PWD/air/parseAir.c \
   $$PWD/air/randMT.c \
   $$PWD/air/sane.c \
   $$PWD/air/string.c \
   $$PWD/air/threadAir.c \
   $$PWD/biff/test/test.c \
   $$PWD/biff/biffbiff.c \
   $$PWD/biff/biffmsg.c \
   $$PWD/hest/test/bday.c \
   $$PWD/hest/test/ex0.c \
   $$PWD/hest/test/ex1.c \
   $$PWD/hest/test/ex2.c \
   $$PWD/hest/test/ex3.c \
   $$PWD/hest/test/ex4.c \
   $$PWD/hest/test/ex5.c \
   $$PWD/hest/test/strings.c \
   $$PWD/hest/test/tmpl.c \
   $$PWD/hest/defaultsHest.c \
   $$PWD/hest/methodsHest.c \
   $$PWD/hest/parseHest.c \
   $$PWD/hest/usage.c \
   $$PWD/nrrd/test/ax.c \
   $$PWD/nrrd/test/convo.c \
   $$PWD/nrrd/test/dnorm.c \
   $$PWD/nrrd/test/genvol.c \
   $$PWD/nrrd/test/histrad.c \
   $$PWD/nrrd/test/io.c \
   $$PWD/nrrd/test/kv.c \
   $$PWD/nrrd/test/minmax.c \
   $$PWD/nrrd/test/morph.c \
   $$PWD/nrrd/test/otsu.c \
   $$PWD/nrrd/test/quadvol.c \
   $$PWD/nrrd/test/reuse.c \
   $$PWD/nrrd/test/strio.c \
   $$PWD/nrrd/test/texp.c \
   $$PWD/nrrd/test/tkernel.c \
   $$PWD/nrrd/test/tline.c \
   $$PWD/nrrd/test/trand.c \
   $$PWD/nrrd/test/tread.c \
   $$PWD/nrrd/test/typestest.c \
   $$PWD/nrrd/tmf/tmFilters_raw.c \
   $$PWD/nrrd/accessors.c \
   $$PWD/nrrd/apply1D.c \
   $$PWD/nrrd/apply2D.c \
   $$PWD/nrrd/arith.c \
   $$PWD/nrrd/arraysNrrd.c \
   $$PWD/nrrd/axis.c \
   $$PWD/nrrd/bsplKernel.c \
   $$PWD/nrrd/cc.c \
   $$PWD/nrrd/ccmethods.c \
   $$PWD/nrrd/comment.c \
   $$PWD/nrrd/convertNrrd.c \
   $$PWD/nrrd/defaultsNrrd.c \
   $$PWD/nrrd/deringNrrd.c \
   $$PWD/nrrd/encoding.c \
   $$PWD/nrrd/encodingAscii.c \
   $$PWD/nrrd/encodingBzip2.c \
   $$PWD/nrrd/encodingGzip.c \
   $$PWD/nrrd/encodingHex.c \
   $$PWD/nrrd/encodingRaw.c \
   $$PWD/nrrd/endianNrrd.c \
   $$PWD/nrrd/enumsNrrd.c \
   $$PWD/nrrd/fftNrrd.c \
   $$PWD/nrrd/filt.c \
   $$PWD/nrrd/format.c \
   $$PWD/nrrd/formatEPS.c \
   $$PWD/nrrd/formatNRRD.c \
   $$PWD/nrrd/formatPNG.c \
   $$PWD/nrrd/formatPNM.c \
   $$PWD/nrrd/formatText.c \
   $$PWD/nrrd/formatVTK.c \
   $$PWD/nrrd/gzio.c \
   $$PWD/nrrd/hestNrrd.c \
   $$PWD/nrrd/histogram.c \
   $$PWD/nrrd/iter.c \
   $$PWD/nrrd/kernel.c \
   $$PWD/nrrd/keyvalue.c \
   $$PWD/nrrd/map.c \
   $$PWD/nrrd/measure.c \
   $$PWD/nrrd/methodsNrrd.c \
   $$PWD/nrrd/parseNrrd.c \
   $$PWD/nrrd/range.c \
   $$PWD/nrrd/read.c \
   $$PWD/nrrd/reorder.c \
   $$PWD/nrrd/resampleContext.c \
   $$PWD/nrrd/resampleNrrd.c \
   $$PWD/nrrd/simple.c \
   $$PWD/nrrd/subset.c \
   $$PWD/nrrd/superset.c \
   $$PWD/nrrd/tmfKernel.c \
   $$PWD/nrrd/winKernel.c \
   $$PWD/nrrd/write.c

INCLUDEPATH = \
    $$PWD/. \
    $$PWD/air \
    $$PWD/biff \
    $$PWD/hest \
    $$PWD/nrrd

#DEFINES = 

