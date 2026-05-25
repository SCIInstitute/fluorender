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

#include <GLAttribProvider.h>
#include <Global.h>
#include <MainSettings.h>

const wxGLAttributes& GLAttribProvider::CanvasAttribs()
{
	static wxGLAttributes attribs;
	static bool initialized = false;

	if (!initialized)
	{
		initialized = true;

		attribs.PlatformDefaults();

		// Request high‑precision presentation
		// On supporting systems this selects:
		//   - 10‑10‑10‑2 (HDR / deep color)
		// On others it will gracefully fall back to 8‑8‑8‑8
		attribs.MinRGBA(
			glbin_settings.m_red_bit,    // 8 or 10
			glbin_settings.m_green_bit,  // 8 or 10
			glbin_settings.m_blue_bit,   // 8 or 10
			glbin_settings.m_alpha_bit   // often 2 or 8
		);

		// Depth / stencil
		attribs.Depth(glbin_settings.m_depth_bit); // typically 24
		attribs.Stencil(8);

		// Multisampling (optional)
		if (glbin_settings.m_samples > 0)
		{
			attribs.SampleBuffers(1);
			attribs.Samplers(glbin_settings.m_samples);
		}

		attribs.DoubleBuffer();
		attribs.EndList();
	}

	return attribs;
}

wxGLContextAttrs GLAttribProvider::MakeContextAttrs()
{
    wxGLContextAttrs ctx;

#ifdef __APPLE__
    // macOS: only valid modern profile is 3.2 core
    ctx.PlatformDefaults()
       .CoreProfile()
       .MajorVersion(3)
       .MinorVersion(2)
       .Robust()
       .ResetIsolation()
       .EndList();
#else
    // Windows / Linux: use your config
    switch (glbin_settings.m_gl_profile_mask)
    {
        case 1: ctx.CoreProfile(); break;
        case 2: ctx.CompatibilityProfile(); break;
    }

    ctx.MajorVersion(glbin_settings.m_gl_major_ver)
       .MinorVersion(glbin_settings.m_gl_minor_ver)
       .Robust()
       .ResetIsolation()
       .EndList();
#endif

    return ctx;
}
