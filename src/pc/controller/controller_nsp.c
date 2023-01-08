

#include <libndls.h>


#include "controller_api.h"

static void controller_nsp_init(void) {

}

static void controller_nsp_read(OSContPad *pad) {
    if (isKeyPressed(KEY_NSPIRE_ENTER))
        pad->button |= START_BUTTON;
    if (isKeyPressed(KEY_NSPIRE_MENU))
        pad->button |= A_BUTTON;
    if (isKeyPressed(KEY_NSPIRE_DEL))
        pad->button |= B_BUTTON;
    if (isKeyPressed(KEY_NSPIRE_DOC))
        pad->button |= Z_TRIG;
    
    if (isKeyPressed(KEY_NSPIRE_RIGHT))
        pad->stick_x = 127;
    if (isKeyPressed(KEY_NSPIRE_LEFT))
        pad->stick_x = -128;
    if (isKeyPressed(KEY_NSPIRE_UP))
        pad->stick_y = 127;
    if (isKeyPressed(KEY_NSPIRE_DOWN))
        pad->stick_y = -128;
}

struct ControllerAPI controller_nsp = {
	controller_nsp_init,
	controller_nsp_read
};

//enter = enter
//up = 8
//down = 2
//left = 4
//right = 6
//jump = del
//hit = menu
//crouch = doc