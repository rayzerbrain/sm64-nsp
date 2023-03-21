#include <libndls.h>

#include "macros.h"

#include "lib/src/libultra_internal.h"
#include "lib/src/osContInternal.h"


s32 osContInit(UNUSED OSMesgQueue *mq, u8 *controllerBits, UNUSED OSContStatus *status) {
    *controllerBits = 1;
    return 0;
}

s32 osContStartReadData(UNUSED OSMesgQueue *mesg) {
    return 0;
}

void osContGetReadData(OSContPad *pad) {
    pad->button = 0;
    pad->stick_x = 0;
    pad->stick_y = 0;
    pad->errnum = 0;
    
    if (isKeyPressed(KEY_NSPIRE_ENTER))
        pad->button |= START_BUTTON;
    if (isKeyPressed(KEY_NSPIRE_MENU))
        pad->button |= A_BUTTON;
    if (isKeyPressed(KEY_NSPIRE_DEL))
        pad->button |= B_BUTTON;
    if (isKeyPressed(KEY_NSPIRE_DOC))
        pad->button |= Z_TRIG;

    if (isKeyPressed(KEY_NSPIRE_LEFT) || isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_DOWNLEFT))
        pad->stick_x = -128;
    if (isKeyPressed(KEY_NSPIRE_RIGHT) || isKeyPressed(KEY_NSPIRE_UPRIGHT) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN))
        pad->stick_x = 127;
    if (isKeyPressed(KEY_NSPIRE_UP) || isKeyPressed(KEY_NSPIRE_LEFTUP) || isKeyPressed(KEY_NSPIRE_UPRIGHT))
        pad->stick_y = 127;
    if (isKeyPressed(KEY_NSPIRE_DOWN) || isKeyPressed(KEY_NSPIRE_RIGHTDOWN) || isKeyPressed(KEY_NSPIRE_DOWNLEFT))
        pad->stick_y = -128;

    if (isKeyPressed(KEY_NSPIRE_8))
        pad->button |= U_CBUTTONS;
    if (isKeyPressed(KEY_NSPIRE_2))
        pad->button |= D_CBUTTONS;
    if (isKeyPressed(KEY_NSPIRE_6))
        pad->button |= R_CBUTTONS;
    if (isKeyPressed(KEY_NSPIRE_4))
        pad->button |= L_CBUTTONS;
}