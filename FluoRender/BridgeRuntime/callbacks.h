#pragma once

enum class Modifiers 
{
    NoModifier = 0,
    Shift = 1,
    Ctrl = 2,
    Alt = 4,
};

extern "C"
{
	typedef void (*PFNKEYDOWNCALLBACK)(int key, int modifiers, unsigned long window);
    typedef void (*PFNKEYUPCALLBACK)(int key, int modifiers, unsigned long window);
    typedef void (*PFNMOUSEDOWNCALLBACK)(int x, int y, unsigned long window);
    typedef void (*PFNMPOUSEUPCALLBACK)(int x, int y, unsigned long window);
    typedef void (*PFNMOVECALLBACK)(int x, int y, unsigned long window);
    typedef void (*PFNMOUSEWHEELCALLBACK)(int delta, unsigned long window);

}