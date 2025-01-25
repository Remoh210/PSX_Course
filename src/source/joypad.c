#include "joypad.h"
#include <libetc.h>
#include <sys/types.h>


// 32-bit number that holds the state of the joypad
static u_long padstate;             

void JoyPadInit(void) 
{
    // Initializes the joypad
    PadInit(0);
}

void JoyPadReset(void) 
{
    padstate = 0;
}

void JoyPadUpdate(void) 
{
    // Update the state of the controller
    u_long pad = PadRead(0);
    padstate = pad;
}

int JoyPadCheck(int p)
{
    return padstate & p;
}