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

#ifndef Layout_h
#define Layout_h

enum class RenderViewLayout
{
	Horizontal,        // 1xN
	Vertical,          // Nx1

	Grid2x2,           // up to 4 views

	ThreeTopBottom,    // 2 top, 1 bottom
	ThreeLeftRight,    // 2 left, 1 right

	QuadTopLarge,      // 1 top, 3 bottom
	QuadLeftLarge,     // 1 left, 3 right

	Auto               // choose based on count
};

struct ViewLayoutItem
{
	int row;
	int col;
	int rowSpan;
	int colSpan;
};

struct LayoutSpec
{
	int rows;
	int cols;
	std::vector<ViewLayoutItem> items;
};

static inline LayoutSpec GetLayoutSpec(
    RenderViewLayout layout,
    int count)
{
    LayoutSpec spec{};

    if (layout == RenderViewLayout::Auto)
    {
        switch (count)
        {
        case 1:
            layout = RenderViewLayout::Horizontal;
            break;
        case 2:
            layout = RenderViewLayout::Vertical;
            break;
        case 3:
            layout = RenderViewLayout::ThreeTopBottom;
            break;
        case 4:
            layout = RenderViewLayout::Grid2x2;
            break;
        default:
            layout = RenderViewLayout::Grid2x2;
            break;
        }
    }

    switch (layout)
    {
    case RenderViewLayout::Horizontal:
        spec.rows = 1;
        spec.cols = count;
        for (int i = 0; i < count; ++i)
            spec.items.push_back({ 0, i, 1, 1 });
        break;

    case RenderViewLayout::Vertical:
        spec.rows = count;
        spec.cols = 1;
        for (int i = 0; i < count; ++i)
            spec.items.push_back({ i, 0, 1, 1 });
        break;

    case RenderViewLayout::ThreeTopBottom:
        spec.rows = 2;
        spec.cols = 2;
        spec.items =
        {
            {0,0,1,1},
            {0,1,1,1},
            {1,0,1,2}
        };
        break;

    case RenderViewLayout::ThreeLeftRight:
        spec.rows = 2;
        spec.cols = 2;
        spec.items =
        {
            {0,0,2,1},
            {0,1,1,1},
            {1,1,1,1}
        };
        break;

    case RenderViewLayout::Grid2x2:
        spec.rows = 2;
        spec.cols = 2;
        spec.items =
        {
            {0,0,1,1},
            {0,1,1,1},
            {1,0,1,1},
            {1,1,1,1}
        };
        break;

    case RenderViewLayout::QuadTopLarge:
        spec.rows = 2;
        spec.cols = 3;
        spec.items =
        {
            {0,0,1,3}, // large top
            {1,0,1,1},
            {1,1,1,1},
            {1,2,1,1}
        };
        break;

    case RenderViewLayout::QuadLeftLarge:
        spec.rows = 3;
        spec.cols = 2;
        spec.items =
        {
            {0,0,3,1}, // large left
            {0,1,1,1},
            {1,1,1,1},
            {2,1,1,1}
        };
        break;
    }

    return spec;
}

#endif//Layout_h