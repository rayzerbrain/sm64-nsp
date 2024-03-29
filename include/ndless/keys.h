#ifndef KEYS_H
#define KEYS_H

/***********************************
 * Keys (key=(offset, 2^bit #)
 ***********************************/

typedef enum tpad_arrow {
    TPAD_ARROW_NONE,
    TPAD_ARROW_UP,
    TPAD_ARROW_UPRIGHT,
    TPAD_ARROW_RIGHT,
    TPAD_ARROW_RIGHTDOWN,
    TPAD_ARROW_DOWN,
    TPAD_ARROW_DOWNLEFT,
    TPAD_ARROW_LEFT,
    TPAD_ARROW_LEFTUP,
    TPAD_ARROW_CLICK
} tpad_arrow_t;

typedef struct {
    int row, col, tpad_row, tpad_col;
    tpad_arrow_t tpad_arrow;
} t_key;

/* Used when the row and column are the same for both models */
#define KEY_(row, col)                                                                                 \
    { row, col, row, col, TPAD_ARROW_NONE }
#define KEYTPAD_(row, col, tpad_row, tpad_col)                                                         \
    { row, col, tpad_row, tpad_col, TPAD_ARROW_NONE }
#define KEYTPAD_ARROW_(row, col, tpad_arrow)                                                           \
    { row, col, row, col, tpad_arrow }

/* Used to fill up a nonexistent key on a model */
#define _KEY_DUMMY_ROW 0x1C
#define _KEY_DUMMY_COL 0x400

static const t_key KEY_NSPIRE_RET = KEY_(0x10, 0x001);
static const t_key KEY_NSPIRE_ENTER = KEY_(0x10, 0x002);
static const t_key KEY_NSPIRE_SPACE = KEYTPAD_(0x10, 0x004, 0x10, 0x10);
static const t_key KEY_NSPIRE_NEGATIVE = KEY_(0x10, 0x008);
static const t_key KEY_NSPIRE_Z = KEYTPAD_(0x10, 0x010, 0x10, 0x20);
static const t_key KEY_NSPIRE_PERIOD = KEYTPAD_(0x10, 0x20, 0x1A, 0x010);
static const t_key KEY_NSPIRE_Y = KEY_(0x10, 0x040);
static const t_key KEY_NSPIRE_0 = KEY_(0x10, 0x080);
static const t_key KEY_NSPIRE_X = KEYTPAD_(0x10, 0x100, 0x12, 0x001);
static const t_key KEY_NSPIRE_THETA = KEYTPAD_(0x10, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_COMMA = KEYTPAD_(0x12, 0x001, 0x1E, 0x400);
static const t_key KEY_NSPIRE_PLUS = KEYTPAD_(0x12, 0x002, 0x1C, 0x004);
static const t_key KEY_NSPIRE_W = KEYTPAD_(0x12, 0x004, 0x12, 0x002);
static const t_key KEY_NSPIRE_3 = KEY_(0x12, 0x008);
static const t_key KEY_NSPIRE_V = KEYTPAD_(0x12, 0x010, 0x12, 0x004);
static const t_key KEY_NSPIRE_2 = KEYTPAD_(0x12, 0x020, 0x1C, 0x010);
static const t_key KEY_NSPIRE_U = KEYTPAD_(0x12, 0x040, 0x12, 0x010);
static const t_key KEY_NSPIRE_1 = KEY_(0x12, 0x080);
static const t_key KEY_NSPIRE_T = KEYTPAD_(0x12, 0x100, 0x12, 0x020);
static const t_key KEY_NSPIRE_eEXP = KEYTPAD_(0x12, 0x200, 0x16, 0x200);
static const t_key KEY_NSPIRE_PI = KEYTPAD_(0x12, 0x400, 0x12, 0x100);
static const t_key KEY_NSPIRE_QUES = KEYTPAD_(0x14, 0x001, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_QUESEXCL = KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x10, 0x100);
static const t_key KEY_NSPIRE_MINUS = KEYTPAD_(0x14, 0x002, 0x1A, 0x004);
static const t_key KEY_NSPIRE_S = KEYTPAD_(0x14, 0x004, 0x12, 0x040);
static const t_key KEY_NSPIRE_6 = KEY_(0x14, 0x008);
static const t_key KEY_NSPIRE_R = KEYTPAD_(0x14, 0x010, 0x14, 0x001);
static const t_key KEY_NSPIRE_5 = KEYTPAD_(0x14, 0x020, 0x1A, 0x040);
static const t_key KEY_NSPIRE_Q = KEYTPAD_(0x14, 0x040, 0x14, 0x002);
static const t_key KEY_NSPIRE_4 = KEY_(0x14, 0x080);
static const t_key KEY_NSPIRE_P = KEYTPAD_(0x14, 0x100, 0x14, 0x004);
static const t_key KEY_NSPIRE_TENX = KEYTPAD_(0x14, 0x200, 0x12, 0x400);
static const t_key KEY_NSPIRE_EE = KEYTPAD_(0x14, 0x400, 0x14, 0x100);
static const t_key KEY_NSPIRE_COLON = KEYTPAD_(0x16, 0x001, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_MULTIPLY = KEYTPAD_(0x16, 0x002, 0x18, 0x100);
static const t_key KEY_NSPIRE_O = KEYTPAD_(0x16, 0x004, 0x14, 0x010);
static const t_key KEY_NSPIRE_9 = KEY_(0x16, 0x008);
static const t_key KEY_NSPIRE_N = KEYTPAD_(0x16, 0x010, 0x14, 0x020);
static const t_key KEY_NSPIRE_8 = KEYTPAD_(0x16, 0x020, 0x1C, 0x040);
static const t_key KEY_NSPIRE_M = KEYTPAD_(0x16, 0x040, 0x14, 0x040);
static const t_key KEY_NSPIRE_7 = KEY_(0x16, 0x080);
static const t_key KEY_NSPIRE_L = KEYTPAD_(0x16, 0x100, 0x16, 0x001);
static const t_key KEY_NSPIRE_SQU = KEYTPAD_(0x16, 0x200, 0x14, 0x200);
static const t_key KEY_NSPIRE_II = KEYTPAD_(0x16, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_QUOTE = KEYTPAD_(0x18, 0x001, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_DIVIDE = KEYTPAD_(0x18, 0x002, 0x16, 0x100);
static const t_key KEY_NSPIRE_K = KEYTPAD_(0x18, 0x004, 0x16, 0x002);
static const t_key KEY_NSPIRE_TAN = KEY_(0x18, 0x008);
static const t_key KEY_NSPIRE_J = KEYTPAD_(0x18, 0x010, 0x16, 0x004);
static const t_key KEY_NSPIRE_COS = KEYTPAD_(0x18, 0x020, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_I = KEYTPAD_(0x18, 0x040, 0x16, 0x010);
static const t_key KEY_NSPIRE_SIN = KEYTPAD_(0x18, 0x080, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_H = KEYTPAD_(0x18, 0x100, 0x16, 0x020);
static const t_key KEY_NSPIRE_EXP = KEYTPAD_(0x18, 0x200, 0x18, 0x200);
static const t_key KEY_NSPIRE_GTHAN = KEYTPAD_(0x18, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_APOSTROPHE = KEY_(0x1A, 0x001);
static const t_key KEY_NSPIRE_CAT = KEYTPAD_(0x1A, 0x002, 0x1A, 0x080);
static const t_key KEY_NSPIRE_FRAC = KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x1A, 0x100);
static const t_key KEY_NSPIRE_G = KEYTPAD_(0x1A, 0x004, 0x16, 0x040);
static const t_key KEY_NSPIRE_RP = KEYTPAD_(0x1A, 0x008, 0x1A, 0x008);
static const t_key KEY_NSPIRE_F = KEYTPAD_(0x1A, 0x010, 0x18, 0x001);
static const t_key KEY_NSPIRE_LP = KEYTPAD_(0x1A, 0x020, 0x1A, 0x020);
static const t_key KEY_NSPIRE_E = KEYTPAD_(0x1A, 0x040, 0x18, 0x002);
static const t_key KEY_NSPIRE_VAR = KEYTPAD_(0x1A, 0x080, 0x1A, 0x002);
static const t_key KEY_NSPIRE_D = KEYTPAD_(0x1A, 0x100, 0x18, 0x004);
static const t_key KEY_NSPIRE_DEL = KEYTPAD_(0x1E, 0x100, 0x1A, 0x200);
static const t_key KEY_NSPIRE_LTHAN = KEYTPAD_(0x1A, 0x400, _KEY_DUMMY_ROW, _KEY_DUMMY_COL);
static const t_key KEY_NSPIRE_FLAG = KEY_(0x1C, 0x001);
static const t_key KEY_NSPIRE_CLICK = KEYTPAD_ARROW_(0x1C, 0x002, TPAD_ARROW_CLICK);
static const t_key KEY_NSPIRE_C = KEYTPAD_(0x1C, 0x004, 0x18, 0x010);
static const t_key KEY_NSPIRE_HOME = KEYTPAD_(0x1C, 0x008, 0x10, 0x200);
static const t_key KEY_NSPIRE_B = KEYTPAD_(0x1C, 0x010, 0x18, 0x020);
static const t_key KEY_NSPIRE_MENU = KEY_(0x1C, 0x020);
static const t_key KEY_NSPIRE_A = KEYTPAD_(0x1C, 0x040, 0x18, 0x040);
static const t_key KEY_NSPIRE_ESC = KEY_(0x1C, 0x080);
static const t_key KEY_NSPIRE_BAR = KEY_(0x1C, 0x100);
static const t_key KEY_NSPIRE_TAB = KEY_(0x1C, 0x200);
static const t_key KEY_NSPIRE_EQU = KEYTPAD_(0x1E, 0x400, 0x18, 0x080);
static const t_key KEY_NSPIRE_UP = KEYTPAD_ARROW_(0x1E, 0x001, TPAD_ARROW_UP);
static const t_key KEY_NSPIRE_UPRIGHT = KEYTPAD_ARROW_(0x1E, 0x002, TPAD_ARROW_UPRIGHT);
static const t_key KEY_NSPIRE_RIGHT = KEYTPAD_ARROW_(0x1E, 0x004, TPAD_ARROW_RIGHT);
static const t_key KEY_NSPIRE_RIGHTDOWN = KEYTPAD_ARROW_(0x1E, 0x008, TPAD_ARROW_RIGHTDOWN);
static const t_key KEY_NSPIRE_DOWN = KEYTPAD_ARROW_(0x1E, 0x010, TPAD_ARROW_DOWN);
static const t_key KEY_NSPIRE_DOWNLEFT = KEYTPAD_ARROW_(0x1E, 0x020, TPAD_ARROW_DOWNLEFT);
static const t_key KEY_NSPIRE_LEFT = KEYTPAD_ARROW_(0x1E, 0x040, TPAD_ARROW_LEFT);
static const t_key KEY_NSPIRE_LEFTUP = KEYTPAD_ARROW_(0x1E, 0x080, TPAD_ARROW_LEFTUP);
static const t_key KEY_NSPIRE_SHIFT = KEYTPAD_(0x1A, 0x200, 0x1E, 0x100);
static const t_key KEY_NSPIRE_CTRL = KEY_(0x1E, 0x200);
static const t_key KEY_NSPIRE_DOC = KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x1C, 0x008);
static const t_key KEY_NSPIRE_TRIG = KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x12, 0x200);
static const t_key KEY_NSPIRE_SCRATCHPAD = KEYTPAD_(_KEY_DUMMY_ROW, _KEY_DUMMY_COL, 0x1A, 0x400);

/* TI-84+ Keypad Mappings */
static const t_key KEY_84_DOWN = KEY_(0x10, 0x001);
static const t_key KEY_84_LEFT = KEY_(0x10, 0x002);
static const t_key KEY_84_RIGHT = KEY_(0x10, 0x004);
static const t_key KEY_84_UP = KEY_(0x10, 0x008);
static const t_key KEY_84_ENTER = KEY_(0x12, 0x001);
static const t_key KEY_84_PLUS = KEY_(0x12, 0x002);
static const t_key KEY_84_MINUS = KEY_(0x12, 0x004);
static const t_key KEY_84_MULTIPLY = KEY_(0x12, 0x008);
static const t_key KEY_84_DIVIDE = KEY_(0x12, 0x010);
static const t_key KEY_84_EXP = KEY_(0x12, 0x020);
static const t_key KEY_84_CLEAR = KEY_(0x12, 0x040);
static const t_key KEY_84_NEGATIVE = KEY_(0x14, 0x001);
static const t_key KEY_84_3 = KEY_(0x14, 0x002);
static const t_key KEY_84_6 = KEY_(0x14, 0x004);
static const t_key KEY_84_9 = KEY_(0x14, 0x008);
static const t_key KEY_84_RP = KEY_(0x14, 0x010);
static const t_key KEY_84_TAN = KEY_(0x14, 0x020);
static const t_key KEY_84_VARS = KEY_(0x14, 0x040);
static const t_key KEY_84_PERIOD = KEY_(0x16, 0x001);
static const t_key KEY_84_2 = KEY_(0x16, 0x002);
static const t_key KEY_84_5 = KEY_(0x16, 0x004);
static const t_key KEY_84_8 = KEY_(0x16, 0x008);
static const t_key KEY_84_LP = KEY_(0x16, 0x010);
static const t_key KEY_84_COS = KEY_(0x16, 0x020);
static const t_key KEY_84_PRGM = KEY_(0x16, 0x040);
static const t_key KEY_84_STAT = KEY_(0x16, 0x080);
static const t_key KEY_84_0 = KEY_(0x18, 0x001);
static const t_key KEY_84_1 = KEY_(0x18, 0x002);
static const t_key KEY_84_4 = KEY_(0x18, 0x004);
static const t_key KEY_84_7 = KEY_(0x18, 0x008);
static const t_key KEY_84_COMMA = KEY_(0x18, 0x010);
static const t_key KEY_84_SIN = KEY_(0x18, 0x020);
static const t_key KEY_84_APPS = KEY_(0x18, 0x040);
static const t_key KEY_84_X = KEY_(0x18, 0x080);
static const t_key KEY_84_STO = KEY_(0x1A, 0x002);
static const t_key KEY_84_LN = KEY_(0x1A, 0x004);
static const t_key KEY_84_LOG = KEY_(0x1A, 0x008);
static const t_key KEY_84_SQU = KEY_(0x1A, 0x010);
static const t_key KEY_84_INV = KEY_(0x1A, 0x020);
static const t_key KEY_84_MATH = KEY_(0x1A, 0x040);
static const t_key KEY_84_ALPHA = KEY_(0x1A, 0x080);
static const t_key KEY_84_GRAPH = KEY_(0x1C, 0x001);
static const t_key KEY_84_TRACE = KEY_(0x1C, 0x002);
static const t_key KEY_84_ZOOM = KEY_(0x1C, 0x004);
static const t_key KEY_84_WIND = KEY_(0x1C, 0x008);
static const t_key KEY_84_YEQU = KEY_(0x1C, 0x010);
static const t_key KEY_84_2ND = KEY_(0x1C, 0x020);
static const t_key KEY_84_MODE = KEY_(0x1C, 0x040);
static const t_key KEY_84_DEL = KEY_(0x1C, 0x080);

#endif // !KEYS_H