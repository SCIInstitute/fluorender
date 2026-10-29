/*
For more information, please see: http://software.sci.utah.edu

The MIT License

Copyright (c) 2026 Scientific Computing and Imaging Institute,
University of Utah.


Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/

#ifndef UiText_h
#define UiText_h

#include <Version.h>

inline const std::string VERSION_CONTACT =
"https://sciinstitute.github.io/fluorender/";

inline const std::string VERSION_AUTHORS =
"YONG WAN       \tHIDEO OTSUNA   \n"
"CHUCK HANSEN   \tCHI-BIN CHIEN  \n"
"BRIG BAGLEY    \tTAKASHI KAWASE \n"
"REMALDEEP SINGH\tKEI ITO        \n"
"HOLLY A. HOLMAN\n";

inline const std::string VERSION_UPDATES =
"https://sciinstitute.github.io/fluorender/Releases/fluorender_v" +
std::string(fluo::VersionString) +
"/";

inline const std::string HELP_MANUAL =
"https://github.com/SCIInstitute/fluorender/releases/download/v" +
std::string(fluo::VersionString) +
"/FluoRender" +
std::string(fluo::VersionString) +
"_Manual.pdf";

inline const std::string HELP_TUTORIAL =
"https://github.com/SCIInstitute/fluorender/releases/download/v" +
std::string(fluo::VersionString) +
"/FluoRender" +
std::string(fluo::VersionString) +
"/FluoRender" +
std::string(fluo::VersionString) +
"_Tutorials.pdf";

inline const std::string YOUTUBE_URL =
"https://www.youtube.com/playlist?list=PLSBz7un7RyZhO51UWhmGiusaJebmaWPbc";

inline const std::string TWITTER_URL =
"https://twitter.com/FluoRender";

inline const std::string FACEBOOK_URL =
"https://www.facebook.com/fluorender";

// Aliases
inline const std::string& BATCH_INFO = HELP_MANUAL;
inline const std::string& HELP_PAINT = HELP_MANUAL;

// -------------------- UI Text --------------------

inline const std::string UITEXT_PROJECT = "Project";
inline const std::string UITEXT_DATAVIEW = "Data Sets";
inline const std::string UITEXT_TREEVIEW = "Workspace";
inline const std::string UITEXT_MAKEMOVIE = "Movie Making";
inline const std::string UITEXT_ADJUST = "Output Adjustments";
inline const std::string UITEXT_CLIPPING = "Clipping Planes";
inline const std::string UITEXT_PROPERTIES = "Properties";

#endif//UiText_h