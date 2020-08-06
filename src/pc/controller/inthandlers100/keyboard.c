/****************************************************************************/
 ////////////A FEW ROUTINES FOR READING THE STATE OF THE KEYBOARD////////////
 ///////////////////////AS IF IT WERE A SET OF BUTTONS///////////////////////
 ////////////////////////////////VERSION 2.30////////////////////////////////
/****************************************************************************/
/*
 ----------------------------------------------------------------------------
|                Copyright 1998-99-2000 Frederico Jeronimo                   |
 ----------------------------------------------------------------------------
                                                                            */

//NOTES///////////////////////////////////////////////////////////////////////


/* Some of the functions that appear in this module are not used in any of
 * these tutorials. They are only here because they are part of my keyboard
 * handling routines. Since the keyboard handler doesn't need to be reentrant
 * we can use the traditional way to set it. But, more on that on my future
 * to be keyboard tutorial. Stay tuned. */

//HISTORY/////////////////////////////////////////////////////////////////////


/* -----------------08-09-98 16:39-----------------
 V1.00 : First working version in real mode.
--------------------------------------------------*/

/* -----------------10-12-98 10:15-----------------
 V2.00 : The keyboard handler now works in protected
 mode.
--------------------------------------------------*/

/* -----------------10-12-98 12:16-----------------
 V2.05 : Added the locking of variables and
 functions. Saved registers and disabled
 interruptions in the handle itself.
--------------------------------------------------*/

/* -----------------12-12-98 15:33-----------------
 V2.10 : Freed the allocated IRET wrapper after
 removing the new keyboard handler. Added a few
 comments.
--------------------------------------------------*/

/* -----------------25-12-99 13:06-----------------
 V2.11 : Added the copyright notice.
--------------------------------------------------*/

/* -----------------18-03-00 07:13PM---------------
 V2.16 : Moved all global data and defines to the
 header file where they belong.
 -------------------------------------------------*/

/* -----------------21-11-00 1:18------------------
 V2.25 : Added a return zero to the handler function,
 marking it as a no chaining handler. The lack of this
 return value caused a bug if this handler was used
 with an assembly wrapper. Other minor adjustments.
--------------------------------------------------*/

/* -----------------27-11-00 3:16------------------
 V2.26 : Corrected the messy presentation of this
 file. Added extra comments more fitting for a
 tutorial.
--------------------------------------------------*/

/* -----------------27-11-00 3:51------------------
 V2.30 : Added error checking to all relevant
 functions.
--------------------------------------------------*/

//INCLUDES////////////////////////////////////////////////////////////////////


#include <assert.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
// #include "const.h"
#include "keyboard.h"

_go32_dpmi_seginfo old_key_handler, new_key_handler;

int KeyboardStateFlag;                  /* Current keybd handler */
UINT8 KeyStateArray[MAX_SCANCODES];     /* Current key state */
UINT8 PressedArray[MAX_SCANCODES];      /* Flags set if key hit */

//GLOBALS/////////////////////////////////////////////////////////////////////


int KeyboardStateFlag = OLD_HANDLER;    /* Current keybd handler */

//FUNCTIONS///////////////////////////////////////////////////////////////////


/*****************************************************************************
 * Function KeyIntHandler :
 *
 * Replacement for the BIOS int 9 handler. Detects when a key is pressed or
 * released. Updtades KeyStateArray to reflect the state of each key. Sets
 * KeyStateArray to 1 if key is currently being held down, 0 if released.
 * When a key is released, PressedArray is set to 1. This allows a program
 * to detect that a key was pressed and then released between checks of the
 * array. PressedArray is cleared by the program reading the state of a key
 * and not by this routine. KeyStateArray and PressedArray are indexed by
 * the keyboard scan code returned from the keyboard controller.
 ****************************************************************************/

int KeyIntHandler(void)
{
    UINT8 ScanCode;
	UINT8 Temp;

    /* Clear interrupts and save registers */
	asm("cli ; pusha");

    /* Let's read the Scancode */
	ScanCode = inportb(KEYBOARD_INPUT);

    /* Clear Keyboard Controller on XT machines */
	Temp = inportb(KEYBOARD_XT_CLEAR);
    outportb(KEYBOARD_XT_CLEAR, Temp | 0x80); /* Sets the highbit to 1 */
    outportb(KEYBOARD_XT_CLEAR, Temp & 0x7F); /* Resets the highbit */

    /* Key is up if the highbit of ScanCode is set to 1. We use 0x80 because
     * it corresponds to 10000000 in binary and by ADDing it to ScanCode, the
     * conditional if expression is set to 1 if the ScanCode's highbit is also
     * 1 and set to 0 otherwise */
    if (ScanCode & 0x80)
    {
        /*Key is up*/
        ScanCode &= 0x7F;
        KeyStateArray[ScanCode] = 0;
    }

    else
	{
		/*Key is down*/
        KeyStateArray[ScanCode] = 1; /* Reflects that the key is being pressed */
		PressedArray[ScanCode] = 1;
	}

    /* This command tells the PIC (programmable interrupt controller) that the
     * highest priority interrupt has been serviced and clears the interrupt */
    outportb(PIC, NONSPECIFIC_EOI);

    /* Re-enable interruptions and restore the registers */
	asm("popa; sti");
    return RET_SUCCESS;
}

END_OF_FUNCTION(KeyIntHandler);  /* Used for locking */

//////////////////////////////////////////////////////////////////////////////


/*****************************************************************************
 * Function SetKb :
 *
 * Sets up the keyboard as a set of buttons. To do this SetKb() initializes
 * the Key State and Pressed arrays and installs the INT 9 handler,
 * KeyIntHandler().
 ****************************************************************************/

int SetKb(void)
{
	int i;

    /* If the next expression is not true, the calls to SetKb() and ResetKb()
     * have not been balanced and trouble could arise when you quit the program
     * and try to restore the original INT 9 handler */
    assert(KeyboardStateFlag == OLD_HANDLER);

    /* Initialize state arrays */
	for( i=0; i<128; i++)
        KeyStateArray[i] = PressedArray[i] = 0;

    /* If an interrupt handler, any other functions it invokes, or any
     * variables it touches, is 'paged out', the code will bomb out with
     * a page fault. The solution is to 'lock' the memory regions that must
     * be available, telling the DPMI host to keep them in active memory at
     * all times. This is done with the following lines. */
	LOCK_VARIABLE(old_key_handler);
	LOCK_VARIABLE(new_key_handler);
	LOCK_VARIABLE(KeyStateArray);
	LOCK_VARIABLE(PressedArray);
	LOCK_FUNCTION(KeyIntHandler);


    /* Load the address of the function that defines the new interrupt handler
     * in the 'pm_offset' field of the new handler structure */
	new_key_handler.pm_offset   = (int)KeyIntHandler;
	new_key_handler.pm_selector = _go32_my_cs();

    /* old_key_handle stores the original INT9 handler so that it can be
     * restored later on when the new handle is no longer needed */
	_go32_dpmi_get_protected_mode_interrupt_vector(0x9, &old_key_handler);

    /* This function creates a small assembly routine to handle the overhead
     * of servicing an interrupt (adds a IRET at the end). The function we
     * wish to execute is passed in the 'pm_offset' field of the structure it
     * receives as an argument. This produces a similar effect to the
     * 'interrupt' keyword found in other compilers */
    if(_go32_dpmi_allocate_iret_wrapper(&new_key_handler) != 0)
        return RET_FAILURE;

    /* This function replaces the old handler with the new one */
    if(_go32_dpmi_set_protected_mode_interrupt_vector(0x9,&new_key_handler) != 0)
    {
        _go32_dpmi_free_iret_wrapper(&new_key_handler);
        return RET_FAILURE;
    }

    /* Mark the new handler as being installed */
    KeyboardStateFlag = NEW_HANDLER;

    return RET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////


/*****************************************************************************
 * Function ResetKb :
 *
 * The ResetKb function sets the INT9 handler back to the routine saved in
 * OldInt9handler. This routine should only be called after SetKb() has been
 * called. If it is called before SetKb is called at least once, the INT 9
 * vector will be set to garbage.
 ****************************************************************************/

int ResetKb(void)
{
    /* If the next expression doesn't evaluate to true, there could be some
     * deep shit at program termination */
    assert(KeyboardStateFlag == NEW_HANDLER);

    /* Reinstall the original handler */
    if(_go32_dpmi_set_protected_mode_interrupt_vector(0x9,&old_key_handler) !=0)
        return RET_FAILURE;

    /* Free the allocated IRET wrapper */
    if(_go32_dpmi_free_iret_wrapper(&new_key_handler) != 0)
        return RET_FAILURE;

    /* Indicate that the old handler is current */
    KeyboardStateFlag = OLD_HANDLER;

    return RET_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////

/*****************************************************************************
 * Function KeyState :
 *
 * This routine returns 1 if the key is currently down or was pressed since
 * this function was last called for the key. 0 otherwise. The function
 * continues to return 1 as long as the key is being held down. KeyState()
 * should only be called while in SetKb() mode
 ****************************************************************************/

int KeyState(int ScanCode)
{
	int result;

    /* Scan codes should only be between 0 and 127 */
	assert(ScanCode < 128);
    assert(KeyboardStateFlag == NEW_HANDLER); /* Check if SetKb() mode is on */

	result = KeyStateArray[ScanCode] | PressedArray[ScanCode];
    PressedArray[ScanCode] = 0; /* Clear PressedArray */

    /* Returns 1 if either the key is being pressed , or has been pressed
     * since the routine was last called for that scancode */
    return result;
}

//////////////////////////////////////////////////////////////////////////////


/*****************************************************************************
 * Function TrueKeyState :
 *
 * Returns 1 if the key is currently down. The function continues to return
 * 1 as long as the key is held down. This routine differs from KeyState()
 * in that it does not check if the key has been pressed and then released
 * before this function was called. Note that KeyState() will still point
 * out that this has happened, even if this function is called first. This
 * function should only be called while in SetKb() mode.
 ****************************************************************************/

int TrueKeyState(int ScanCode)
{
	int result;

    /* Scan codes should only be between 0 and 127 */
	assert(ScanCode < 128);
    assert(KeyboardStateFlag == NEW_HANDLER); /* Check if Setkb() mode is on */

    result = KeyStateArray[ScanCode];   /* Retrieve key status */
	return result;
}

/*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*|*/
