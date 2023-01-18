/*#include <libndls.h>

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

    if (isKeyPressed(KEY_NSPIRE_4))
        pad->stick_x = -128;
    if (isKeyPressed(KEY_NSPIRE_6))
        pad->stick_x = 127;
    if (isKeyPressed(KEY_NSPIRE_8))
        pad->stick_y = 127;
    if (isKeyPressed(KEY_NSPIRE_2))
        pad->stick_y = -128;
}
*/