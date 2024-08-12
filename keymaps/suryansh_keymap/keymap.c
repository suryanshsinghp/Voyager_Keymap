#include QMK_KEYBOARD_H
#include "version.h"
#include "os_detection.h"
#define MOON_LED_LEVEL LED_LEVEL
#include <math.h>
#include <lib/lib8tion/lib8tion.h>
#include "features/layer_lock.h"

// change mouse max speed from keyboad in round robin fashion
extern uint8_t  mk_max_speed;
extern uint8_t  mk_wheel_max_speed;
static uint32_t Mouse_Setting_timer = 999999; // When setting of mouese key is changed, initialized so blinking does not start when Keyboad is plugged

enum custom_keycodes {
    RGB_SLD = ML_SAFE_RANGE,
    ST_MACRO_0,
    MAC_MISSION_CONTROL,
    MM_SPEED, // change max speed of mouse move
    MW_SPEED, // mouse wheel speed
    M_SHFT,   // tap for osm/toggle hold off, short hold for toggle hold on , long hold for normal hold
    M_CTRL,
    M_ALT,
    M_GUI,
    M_OS, // to toggle os system dependent leader behavior
    CLRMODS,
    SCRL_UP,
    SCRL_DN,
    SCRL_LF,
    SCRL_RT,
    LLOCK,
    MOD_TOGG
};

void MemkeywithMod(uint16_t kcode);

uint8_t saved_mods = 0;
// my macro to work with tap Dance, macro other than 1 will copy from 1
bool onMac            = false;
bool onWin            = true;
bool OSdetectedOnce   = false;
bool LeaderRepPressed = false;
bool MouseB2Hold      = false;
bool MouseB0Hold      = false;
bool Layer2Hold       = false;
bool Layer1Hold       = false;
bool ModLock          = false;

static uint32_t leader_time  = 0; // for rgb
bool            LeadExec     = false;
static uint32_t leader_timer = 0; // for repeat of leader key

bool            Mem2RepPressed = false;
static uint32_t mem2_timer     = 0; // for repeat of memory 2 key

bool LLockedOnce = false;
bool LLockInit   = false; // to make sure layer Tap key inits the process

static uint32_t shft_timer     = 0;
static bool     shft_trigger   = false;
static bool     shft_held_once = false;
static uint32_t ctrl_timer     = 0;
static bool     ctrl_trigger   = false;
static bool     ctrl_held_once = false;
static uint32_t alt_timer      = 0;
static bool     alt_trigger    = false;
static bool     alt_held_once  = false;
static uint32_t gui_timer      = 0;
static bool     gui_trigger    = false;
static bool     gui_held_once  = false;
#define ModQuickHoldTime 120
// #define ModLongHoldTime 200
//  matix of count elements with KC_TRNS
uint8_t count1 = 0;
uint8_t count2 = 0;
// uint8_t count3 = 0;
uint16_t        RecStore1[300]     = {KC_NO};
uint8_t         RecModsStore1[300] = {0}; // del after changint rec 2
static uint16_t RecStore2[300]     = {KC_NO};
static uint8_t  RecModsStore2[300] = {0};
// static uint16_t RecStore3[300] = {KC_NO};
static bool recording1 = false;

static bool held1                 = false;
static bool held2                 = false;
static bool held4                 = false;
static bool held5                 = false;
static bool held6                 = false;
static bool held3                 = false;
static bool held7                 = false; // mouse1
uint16_t    mem_keycode           = KC_NO;
uint16_t    mem_keycode_Leader[5] = {KC_NO}; // to remember leader key sequence
uint8_t     countLeader           = 0;

enum tap_dance_codes {
    DANCE_0,
    DANCE_1,
    DANCE_2,
    DANCE_3,
    DANCE_4,
    DANCE_5,
    DANCE_6,
    DANCE_7,
    DANCE_8,
    DANCE_9,
    DANCE_10,
    DANCE_11,
    DANCE_12,
    DANCE_13,
    DANCE_14,
    DANCE_15,
    DANCE_16,
    DANCE_17,
    DANCE_18,
    DANCE_19,
    DANCE_20,
    DANCE_21,
    DANCE_22,
    DANCE_23,
    DANCE_24,
    DANCE_25,
    DANCE_26,
    DANCE_27, // rep
    DANCE_28, // shift
    DANCE_29, // alt
    DANCE_30, // layer 1
    DANCE_31, // mouse 1
    DANCE_R1, // rec1  32
    DANCE_R2, // rec2  33
};

void matrix_init_user(void) {
    if (count1 == (300 - 2)) {
        count1 = 0; // avoid overflow
    }
}
// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_voyager(
    MOD_TOGG,       KC_NO,          OSM(MOD_HYP),   KC_X,          OSM(MOD_HYPR),   KC_NO,                                          KC_NO,          KC_NO,          KC_J,           KC_NO,          AS_TOGG,       CM_TOGG,     
    M_ALT,          KC_Q,           KC_G,           KC_F,           KC_P,           M_GUI,                                          TD(DANCE_R1),   KC_L,           KC_U,           KC_Y,           KC_Z,          KC_NO, 
    M_SHFT,         KC_A,           KC_R,           KC_S,           KC_T,           M_CTRL,                                         TD(DANCE_27),   KC_N,           KC_E,           KC_I,           KC_O,          KC_RSFT,  
    QK_RBT,         KC_V,           KC_W,           KC_C,           KC_D,           SH_TT,                                          SH_TT,          KC_H,           KC_M,           KC_B,           KC_K,          KC_NO, 
                                                    LT(1, KC_ESCAPE),KC_BSPC,                                       KC_ENTER,       LT(2,KC_SPACE)
  ),
  [1] = LAYOUT_voyager(
    KC_NO,          M_CTRL,         M_GUI,          M_SHFT,         M_ALT,          KC_NO,                                          KC_F4,          KC_F5,          KC_F6,          KC_F7,          KC_F8,          KC_F9,          
    KC_NO,          KC_NO,          TD(DANCE_0),    TD(DANCE_1),    TD(DANCE_2),    KC_NO,                                          KC_F3,          TD(DANCE_11),   TD(DANCE_12),   TD(DANCE_13),   KC_NO,          KC_F10,          
    SH_TT,          TD(DANCE_3),    TD(DANCE_4),    TD(DANCE_5),    TD(DANCE_6),    SH_TT,                                          KC_F2,          TD(DANCE_14),   TD(DANCE_15),   TD(DANCE_16),   TD(DANCE_17),   KC_F11,          
    QK_RBT,         TD(DANCE_7),    TD(DANCE_8),    TD(DANCE_9),    TD(DANCE_10),   KC_NO,                                          KC_F1,          TD(DANCE_18),   TD(DANCE_19),   TD(DANCE_20),   TD(DANCE_23),   KC_F12,          
                                                    TO(0),          KC_BSPC,                                        KC_BSPC,        TO(0)
  ),
  [2] = LAYOUT_voyager(
    KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,                                          KC_BRIGHTNESS_DOWN,KC_BRIGHTNESS_UP,TD(DANCE_21),    KC_AUDIO_VOL_UP, KC_NO,          KC_NO, 
    KC_NO,          MW_SPEED,       SCRL_UP,        KC_UP,          KC_MS_BTN3,     KC_SPACE,                                       KC_BSPC,           SCRL_LF,         KC_MS_UP,        SCRL_RT,         M_OS,           KC_NO,          
    CLRMODS,        MM_SPEED,       SCRL_DN,        KC_MS_BTN2,     TD(DANCE_31),   KC_ENTER,                                       KC_TAB,            KC_MS_LEFT,      KC_MS_DOWN,      KC_MS_RIGHT,     TD(DANCE_26),   MAC_MISSION_CONTROL,          
    QK_RBT,         TD(DANCE_23),   KC_LEFT,        KC_DOWN,        KC_RIGHT,       KC_ESC,                                         KC_DELETE,         M_CTRL,          M_GUI,           M_ALT,           M_SHFT,         KC_NO,
                                                    TD(DANCE_24),   TD(DANCE_25),                                   KC_MS_ACCEL1,   TO(0)
  ),
//     [3] = LAYOUT_voyager(
//     KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,                                          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          
//     KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,                                          KC_NO,          RCTL(KC_3),     RCTL(KC_4),     KC_NO,          KC_NO,          KC_NO,          
//     KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,                                          KC_NO,          RCTL(KC_1),     RCTL(KC_2),     RSFT(RGUI(KC_LEFT)), RSFT(RGUI(KC_RIGHT)), KC_NO,          
//     KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,          KC_NO,                                          KC_NO,          RGUI(KC_L),     RGUI(KC_R),     RSFT(RGUI(KC_T)),RGUI(KC_A),    KC_NO,          
//                                                     KC_NO,          KC_NO,                                          KC_NO,          KC_NO
//   ),
};
// clang-format on
const uint16_t PROGMEM combo0[]  = {KC_N, KC_E, COMBO_END};                 // 0 CTRL
const uint16_t PROGMEM combo1[]  = {KC_W, KC_C, COMBO_END};                 // 0 tab
const uint16_t PROGMEM combo2[]  = {KC_ENTER, LT(2, KC_SPACE), COMBO_END};  // 0 to 2
const uint16_t PROGMEM combo3[]  = {KC_F, KC_P, COMBO_END};                 // 0 clrmods
const uint16_t PROGMEM combo4[]  = {KC_MS_BTN3, KC_UP, COMBO_END};          // 2 clrmods
const uint16_t PROGMEM combo5[]  = {LT(1, KC_ESCAPE), KC_BSPC, COMBO_END};  // swap hand temp
const uint16_t PROGMEM combo6[]  = {KC_R, KC_S, COMBO_END};                 // 0 left
const uint16_t PROGMEM combo7[]  = {KC_T, KC_S, COMBO_END};                 // 0 right
const uint16_t PROGMEM combo8[]  = {KC_L, KC_N, COMBO_END};                 // 0 Macro R2
const uint16_t PROGMEM combo9[]  = {KC_H, KC_M, COMBO_END};                 // 0 shift
const uint16_t PROGMEM combo10[] = {KC_M, KC_B, COMBO_END};                 // 0 alt
const uint16_t PROGMEM combo11[] = {KC_L, KC_U, COMBO_END};                 // 0 Rep
const uint16_t PROGMEM combo12[] = {KC_MS_BTN3, TD(DANCE_31), COMBO_END};   // 2 double click
const uint16_t PROGMEM combo13[] = {TD(DANCE_15), TD(DANCE_16), COMBO_END}; // 1 enter
const uint16_t PROGMEM combo14[] = {TD(DANCE_14), TD(DANCE_15), COMBO_END}; // 1 space
const uint16_t PROGMEM combo15[] = {TD(DANCE_5), TD(DANCE_6), COMBO_END};   // 1 right
const uint16_t PROGMEM combo16[] = {TD(DANCE_4), TD(DANCE_5), COMBO_END};   // 1 left
const uint16_t PROGMEM combo17[] = {TD(DANCE_1), TD(DANCE_5), COMBO_END};   // 1 up
const uint16_t PROGMEM combo18[] = {TD(DANCE_9), TD(DANCE_5), COMBO_END};   // 1 down
const uint16_t PROGMEM combo19[] = {TD(DANCE_8), TD(DANCE_9), COMBO_END};   // 1 tab
const uint16_t PROGMEM combo20[] = {KC_F, KC_G, COMBO_END};                 // 0 leader
const uint16_t PROGMEM combo21[] = {KC_U, KC_Y, COMBO_END};                 // 0 R1 macro
const uint16_t PROGMEM combo22[] = {KC_Q, KC_A, COMBO_END};                 // 0 q
const uint16_t PROGMEM combo23[] = {KC_O, KC_Z, COMBO_END};                 // 0 z
const uint16_t PROGMEM combo24[] = {KC_DOWN, KC_RIGHT, COMBO_END};          // 2 shft
const uint16_t PROGMEM combo25[] = {TD(DANCE_9), TD(DANCE_10), COMBO_END};  // 1 del
const uint16_t PROGMEM combo26[] = {KC_U, KC_J, COMBO_END};                 // 0 j
const uint16_t PROGMEM combo27[] = {KC_F, KC_X, COMBO_END};                 // 0 x
const uint16_t PROGMEM combo28[] = {KC_U, KC_E, COMBO_END};                 // 0 Caps
const uint16_t PROGMEM combo29[] = {KC_F, KC_S, COMBO_END};                 // 0 up
const uint16_t PROGMEM combo30[] = {TD(DANCE_0), TD(DANCE_1), COMBO_END};   // 1 llock
const uint16_t PROGMEM combo31[] = {TD(DANCE_11), TD(DANCE_14), COMBO_END}; // 1 swap hands
const uint16_t PROGMEM combo32[] = {TD(DANCE_2), TD(DANCE_6), COMBO_END};   // 1 swap hand
const uint16_t PROGMEM combo33[] = {KC_MS_BTN2, SCRL_DN, COMBO_END};        // 2 gui
const uint16_t PROGMEM combo34[] = {KC_MS_BTN2, TD(DANCE_31), COMBO_END};   // 2 ctrl
const uint16_t PROGMEM combo35[] = {KC_S, KC_C, COMBO_END};                 // 0 down
const uint16_t PROGMEM combo36[] = {KC_E, KC_I, COMBO_END};                 // 0 gui
const uint16_t PROGMEM combo38[] = {KC_C, KC_D, COMBO_END};                 // 0 del
const uint16_t PROGMEM combo40[] = {KC_LEFT, KC_DOWN, COMBO_END};           // 2 alt
const uint16_t PROGMEM combo41[] = {KC_UP, SCRL_UP, COMBO_END};             // 2 llock
const uint16_t PROGMEM combo42[] = {TD(DANCE_12), TD(DANCE_11), COMBO_END}; // 1 rep

combo_t key_combos[COMBO_COUNT] = {
    COMBO(combo0, M_CTRL),        COMBO(combo1, KC_TAB), COMBO(combo2, TO(2)), COMBO(combo3, CLRMODS), COMBO(combo4, CLRMODS),    COMBO(combo5, TO(1)), COMBO(combo6, KC_LEFT), COMBO(combo7, KC_RIGHT), COMBO(combo8, TD(DANCE_R2)), COMBO(combo9, M_SHFT), COMBO(combo10, M_ALT), COMBO(combo11, TD(DANCE_27)), COMBO(combo12, ST_MACRO_0), COMBO(combo13, KC_ENTER), COMBO(combo14, KC_SPACE), COMBO(combo15, KC_RIGHT), COMBO(combo16, KC_LEFT),   COMBO(combo17, KC_UP), COMBO(combo18, KC_DOWN), COMBO(combo19, KC_TAB),       COMBO(combo20, TD(DANCE_29)),
    COMBO(combo21, TD(DANCE_R1)), COMBO(combo22, KC_Q),  COMBO(combo23, KC_Z), COMBO(combo24, M_SHFT), COMBO(combo25, KC_DELETE), COMBO(combo26, KC_J), COMBO(combo27, KC_X),   COMBO(combo28, CW_TOGG), COMBO(combo29, KC_UP),       COMBO(combo30, LLOCK), COMBO(combo31, SH_TT), COMBO(combo32, SH_TT),        COMBO(combo33, M_GUI),      COMBO(combo34, M_CTRL),   COMBO(combo35, KC_DOWN),  COMBO(combo36, M_GUI),    COMBO(combo38, KC_DELETE), COMBO(combo40, M_ALT), COMBO(combo41, LLOCK),   COMBO(combo42, TD(DANCE_27)),
};

uint16_t get_combo_term(uint16_t index, combo_t *combo) {
    switch (combo->keycode) {
        case KC_X:
            return 80;
        case KC_J:
            return 80;
        case KC_Z:
            return 80;
        case KC_Q:
            return 80;
        case TD(DANCE_22):
            return 80;
        case TO(2):
            return 80;
        case TO(1):
            return 80;
    }

    return COMBO_TERM;
}

uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case LT(1, KC_ESCAPE):
            return TAPPING_TERM - 30;
        case LT(1, KC_ENTER):
            return TAPPING_TERM - 30;
        case LT(2, KC_SPACE):
            return TAPPING_TERM - 20;
        case TD(DANCE_0):
            return TAPPING_TERM - 35;
        case TD(DANCE_1):
            return TAPPING_TERM - 35;
        case TD(DANCE_2):
            return TAPPING_TERM - 35;
        case TD(DANCE_3):
            return TAPPING_TERM - 35;
        case TD(DANCE_4):
            return TAPPING_TERM - 35;
        case TD(DANCE_5):
            return TAPPING_TERM - 35;
        case TD(DANCE_6):
            return TAPPING_TERM - 35;
        case TD(DANCE_7):
            return TAPPING_TERM - 35;
        case TD(DANCE_8):
            return TAPPING_TERM - 35;
        case TD(DANCE_9):
            return TAPPING_TERM - 35;
        case TD(DANCE_10):
            return TAPPING_TERM - 35;
        case TD(DANCE_11):
            return TAPPING_TERM - 35;
        case TD(DANCE_12):
            return TAPPING_TERM - 35;
        case TD(DANCE_13):
            return TAPPING_TERM - 35;
        case TD(DANCE_14):
            return TAPPING_TERM - 35;
        case TD(DANCE_15):
            return TAPPING_TERM - 35;
        case TD(DANCE_16):
            return TAPPING_TERM - 35;
        case TD(DANCE_17):
            return TAPPING_TERM - 35;
        case TD(DANCE_18):
            return TAPPING_TERM - 35;
        case TD(DANCE_19):
            return TAPPING_TERM - 35;
        case TD(DANCE_20):
            return TAPPING_TERM - 35;
        case TD(DANCE_21):
            return TAPPING_TERM - 35;
        case TD(DANCE_22):
            return TAPPING_TERM - 35; // layer 2 tap Dance
        case TD(DANCE_23):
            return TAPPING_TERM - 35;
        case TD(DANCE_24):
            return TAPPING_TERM - 45;
        case TD(DANCE_25):
            return TAPPING_TERM - 45;
        case TD(DANCE_26):
            return TAPPING_TERM - 35;
        case TD(DANCE_27):
            return TAPPING_TERM + 30;
        case TD(DANCE_28):
            return TAPPING_TERM - 10; // backspc
        case TD(DANCE_29):
            return TAPPING_TERM + 30;
        case TD(DANCE_30):
            return TAPPING_TERM - 35; // layer 1 tap dance
        case TD(DANCE_31):
            return TAPPING_TERM - 10; // for mouse button 1.
        case TD(DANCE_R1):
            return TAPPING_TERM + 30; // keep Tap term long, to hold for rec to Start/stop
        default:
            return TAPPING_TERM;
    }
}

extern rgb_config_t rgb_matrix_config;

void keyboard_post_init_user(void) {
    rgb_matrix_enable();
}

const uint8_t PROGMEM ledmap[][RGB_MATRIX_LED_COUNT][3] = {
    [0] = {{0, 247, 255}},
    [1] = {{140, 233, 230}},
    [2] = {{212, 239, 228}},
};

void set_layer_color(int layer) {
    uint8_t time    = scale16by8(g_rgb_timer, rgb_matrix_config.speed / 1.5);
    uint8_t colval  = scale8(abs8(sin8(time) - 128) * 2, ledmap[0][0][2]);
    uint8_t colval1 = scale8(abs8(sin8(time) - 128) * 2, ledmap[1][0][2]);
    uint8_t colval2 = scale8(abs8(sin8(time) - 128) * 2, ledmap[2][0][2]);
    if (layer == 0) {
        if (host_keyboard_led_state().caps_lock) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(12, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(18, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(6, f * rgb.r, f * rgb.g, f * rgb.b);
            // rgb_matrix_set_color( 0, f * rgb.r, f * rgb.g, f * rgb.b );
        }

        if (is_swap_hands_on()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(0, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(1, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(30, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(31, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (is_caps_word_on()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(29, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(30, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(31, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(28, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        // if (leader_sequence_active()) {
        if (LeadExec && (timer_elapsed32(leader_time) < 1500)) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(1, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(2, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(3, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (held1) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(20, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(19, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(21, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (held2) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(14, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(13, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(15, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (held7) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(7, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(8, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(9, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (recording1) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(37, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(43, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(49, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(48, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(50, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(51, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(24, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(25, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_SHIFT & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(4, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(5, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(26, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(27, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (get_oneshot_mods() & MOD_MASK_SHIFT) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(26, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(5, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_CTRL & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(22, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(23, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(45, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(44, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (MOD_MASK_CTRL & get_oneshot_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(23, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(44, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_ALT & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(10, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(11, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(32, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(33, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (MOD_MASK_ALT & get_oneshot_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(11, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(32, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_GUI & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(16, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(17, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(38, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(39, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (MOD_MASK_GUI & get_oneshot_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(17, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(38, f * rgb.r, f * rgb.g, f * rgb.b);
        }
    } else { // layer 1 and 2
        for (int i = 0; i < RGB_MATRIX_LED_COUNT; i++) {
            // if (i%6==0||i%6==5||i%6==1||i%6==2){
            if (((i % 6 == 0 || i % 6 == 1) && (i < 24)) || ((i > 25) && (i < 38))) {
                HSV hsv = {
                    .h = pgm_read_byte(&ledmap[layer][0][0]),
                    .s = pgm_read_byte(&ledmap[layer][0][1]),
                    .v = pgm_read_byte(&ledmap[layer][0][2]),
                };
                if (layer == 1) {
                    hsv.v = pgm_read_byte(&colval1);
                } else if (layer == 2) {
                    hsv.v = pgm_read_byte(&colval2);
                }

                if (!hsv.h && !hsv.s && !hsv.v) {
                    rgb_matrix_set_color(i, 0, 0, 0);
                } else {
                    RGB   rgb = hsv_to_rgb(hsv);
                    float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
                    rgb_matrix_set_color(i, f * rgb.r, f * rgb.g, f * rgb.b);
                }
            }
        }
        if (host_keyboard_led_state().caps_lock) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(12, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(18, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(6, f * rgb.r, f * rgb.g, f * rgb.b);
            // rgb_matrix_set_color( 0, f * rgb.r, f * rgb.g, f * rgb.b );
        }

        if (is_swap_hands_on()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(0, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(1, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(30, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(31, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (is_caps_word_on()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(29, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(30, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(31, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(28, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (LeadExec && (timer_elapsed32(leader_time) < 1500)) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(1, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(2, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(3, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (held1) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(20, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(19, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(21, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (held2) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(14, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(13, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(15, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (held7) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(7, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(8, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(9, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (recording1) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(37, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(43, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(49, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(48, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(50, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(51, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(24, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(25, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_SHIFT & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(4, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(5, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(26, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(27, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (get_oneshot_mods() & MOD_MASK_SHIFT) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(26, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(5, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_CTRL & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(22, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(23, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(45, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(44, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (MOD_MASK_CTRL & get_oneshot_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(23, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(44, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_ALT & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(10, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(11, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(32, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(33, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (MOD_MASK_ALT & get_oneshot_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(11, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(32, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (MOD_MASK_GUI & get_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(16, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(17, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(38, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(39, f * rgb.r, f * rgb.g, f * rgb.b);
        }
        if (MOD_MASK_GUI & get_oneshot_mods()) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]),
                .s = pgm_read_byte(&ledmap[0][0][1]),
                .v = pgm_read_byte(&colval),
            };

            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            rgb_matrix_set_color(17, f * rgb.r, f * rgb.g, f * rgb.b);
            rgb_matrix_set_color(38, f * rgb.r, f * rgb.g, f * rgb.b);
        }

        if (timer_elapsed32(Mouse_Setting_timer) < 3000) {
            HSV hsv = {
                .h = pgm_read_byte(&ledmap[0][0][0]), .s = pgm_read_byte(&ledmap[0][0][1]), .v = pgm_read_byte(&ledmap[0][0][2]),
                //.v = pgm_read_byte(&colval),
            };
            RGB   rgb = hsv_to_rgb(hsv);
            float f   = (float)rgb_matrix_config.hsv.v / UINT8_MAX;
            for (int i = 0; i < mk_wheel_max_speed; i++) { // mouse wheel speed
                rgb_matrix_set_color(i, f * rgb.r, f * rgb.g, f * rgb.b);
            }

            for (int i = 0; i < mk_max_speed; i++) { // mouse move speed
                rgb_matrix_set_color(26 + i, f * rgb.r, f * rgb.g, f * rgb.b);
            }
        }
    }
}

bool rgb_matrix_indicators_user(void) {
    if (keyboard_config.disable_layer_led) {
        return false;
    }
    if (ModLock) {
        STATUS_LED_2(true);
    } else {
        STATUS_LED_2(false);
    }
    if (is_swap_hands_on()) {
        STATUS_LED_4(true);
    } else {
        STATUS_LED_4(false);
    }

    switch (biton32(layer_state)) {
        case 0:
            set_layer_color(0);
            break;
        case 1:
            set_layer_color(1);
            break;
        case 2:
            set_layer_color(2);
            break;
        default:
            if (rgb_matrix_get_flags() == LED_FLAG_NONE) rgb_matrix_set_color_all(0, 0, 0);
            break;
    }
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!process_layer_lock(keycode, record, LLOCK)) {
        return false;
    }
    switch (keycode) {
        case SCRL_UP:
            if (record->event.pressed) {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
                    register_code16(KC_MS_WH_DOWN);
                } else {
                    register_code16(KC_MS_WH_UP);
                }
            } else {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
                    unregister_code16(KC_MS_WH_DOWN);
                } else {
                    unregister_code16(KC_MS_WH_UP);
                }
            }
            return false;

        case SCRL_DN:
            if (record->event.pressed) {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
                    register_code16(KC_MS_WH_UP);
                } else {
                    register_code16(KC_MS_WH_DOWN);
                }
            } else {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
                    unregister_code16(KC_MS_WH_UP);
                } else {
                    unregister_code16(KC_MS_WH_DOWN);
                }
            }
            return false;

        case SCRL_RT:
            if (record->event.pressed) {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
                    register_code16(KC_MS_WH_LEFT);
                } else {
                    register_code16(KC_MS_WH_RIGHT);
                }
            } else {
                if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
                    unregister_code16(KC_MS_WH_LEFT);
                } else {
                    unregister_code16(KC_MS_WH_RIGHT);
                }
            }
            return false;

        case SCRL_LF:
            if (record->event.pressed) {
                if (onMac) {
                    register_code16(KC_MS_WH_RIGHT);
                } else {
                    register_code16(KC_MS_WH_LEFT);
                }
            } else {
                if (onMac) {
                    unregister_code16(KC_MS_WH_RIGHT);
                } else {
                    unregister_code16(KC_MS_WH_LEFT);
                }
            }
            return false;

        case KC_MS_UP:
            if (record->event.pressed) {
                if (LLockInit) { // Only lock once-add timer as well-only layer lock if more than 2 sec
                    LLockInit   = false;
                    LLockedOnce = true;
                }
            }
            return true;

        case KC_MS_DOWN:
            if (record->event.pressed) {
                if (LLockInit) { // Only lock once-add timer as well-only layer lock if more than 2 sec
                    LLockInit   = false;
                    LLockedOnce = true;
                }
            }
            return true;

        case KC_MS_LEFT:
            if (record->event.pressed) {
                if (LLockInit) { // Only lock once-add timer as well-only layer lock if more than 2 sec
                    LLockInit   = false;
                    LLockedOnce = true;
                }
            }
            return true;

        case KC_MS_RIGHT:
            if (record->event.pressed) {
                if (LLockInit) { // Only lock once-add timer as well-only layer lock if more than 2 sec
                    LLockInit   = false;
                    LLockedOnce = true;
                }
            }
            return true;

        case M_SHFT:                             // Toggles key
            if (record->event.pressed) {         // When key is pressed.
                shft_timer     = timer_read32(); // Start timer.
                shft_trigger   = true;
                shft_held_once = false;
            } else { // When key is released.
                shft_trigger = false;
                if (timer_elapsed32(shft_timer) < ModQuickHoldTime) { // If timer is less than 200ms.
                    if (held4) {                                      // If shift is held
                        unregister_code16(KC_RSFT);
                        held4 = false;
                    } else {
                        set_oneshot_mods(MOD_BIT(KC_RSFT) | get_oneshot_mods()); // to chain mods like OSM
                    }
                } else { // If timer is more than 200ms.
                    if (ModLock) {
                        held4 = true;
                    } else {
                        unregister_code16(KC_RSFT); // unregister shift hold in matrix_scan_user.
                    }
                }
            }
            return false;

        case M_CTRL:                             // Toggles key
            if (record->event.pressed) {         // When key is pressed.
                ctrl_timer     = timer_read32(); // Start timer.
                ctrl_trigger   = true;
                ctrl_held_once = false;
            } else { // When key is released.
                ctrl_trigger = false;
                if (timer_elapsed32(ctrl_timer) < ModQuickHoldTime) { // If timer is less than 200ms.
                    if (held3) {                                      // If shift is held
                        unregister_code16(KC_RCTL);
                        held3 = false;
                    } else {
                        set_oneshot_mods(MOD_BIT(KC_RCTL) | get_oneshot_mods());
                        // mem_keycode= C(mem_keycode);
                        //   if (recording1) {
                        //      RecStore1[count1] = KC_RCTL; // recording only for tap fun
                        //      count1++;
                        //      }
                    }
                } else { // If timer is more than 200ms.
                    if (ModLock) {
                        held3 = true;
                    } else {
                        unregister_code16(KC_RCTL); // unregister shift hold in matrix_scan_user.
                    }
                }
            }
            return false;

        case M_ALT:                             // Toggles key
            if (record->event.pressed) {        // When key is pressed.
                alt_timer     = timer_read32(); // Start timer.
                alt_trigger   = true;
                alt_held_once = false;
            } else { // When key is released.
                alt_trigger = false;
                if (timer_elapsed32(alt_timer) < ModQuickHoldTime) { // If timer is less than 200ms.
                    if (held5) {                                     // If shift is held
                        unregister_code16(KC_RALT);
                        held5 = false;
                    } else {
                        set_oneshot_mods(MOD_BIT(KC_RALT) | get_oneshot_mods());
                        //  if (recording1) {
                        //     RecStore1[count1] = KC_RALT; // recording only for tap fun
                        //     count1++;
                        //     }
                    }
                } else { // If timer is more than 200ms.
                    if (ModLock) {
                        held5 = true;
                    } else {
                        unregister_code16(KC_RALT); // unregister shift hold in matrix_scan_user.
                    }
                }
            }
            return false;

        case M_GUI:                             // Toggles key
            if (record->event.pressed) {        // When key is pressed.
                gui_timer     = timer_read32(); // Start timer.
                gui_trigger   = true;
                gui_held_once = false;
            } else { // When key is released.
                gui_trigger = false;
                if (timer_elapsed32(gui_timer) < ModQuickHoldTime) { // If timer is less than 200ms.
                    if (held6) {                                     // If shift is held
                        unregister_code16(KC_RGUI);
                        held6 = false;
                    } else {
                        set_oneshot_mods(MOD_BIT(KC_RGUI) | get_oneshot_mods());
                        // mem_keycode= G(mem_keycode);
                        //   if (recording1) {
                        //      RecStore1[count1] = KC_RGUI; // recording only for tap fun
                        //      count1++;
                        //      }
                    }
                } else { // If timer is more than 200ms.
                    if (ModLock) {
                        held6 = true;
                    } else {
                        unregister_code16(KC_RGUI); // unregister shift hold in matrix_scan_user.
                    }
                }
            }

        case KC_A ... KC_Z:
            if (is_caps_word_on() || host_keyboard_led_state().caps_lock) {
                if (record->event.pressed) {
                    // mem_keycode = S(keycode);  // if any bugs , revert to this
                    MemkeywithMod(S(keycode));
                }
            } else {
                if (record->event.pressed) {
                    MemkeywithMod(keycode);
                }
            }
            return true;

        case KC_1 ... KC_0:
        case KC_TAB:
        case KC_ENTER:
        case KC_SPC:
        case KC_BSPC:
        case KC_UP:
        case KC_DOWN:
        case KC_LEFT:
        case KC_RGHT:
        case KC_DEL:
            if (record->event.pressed) {
                MemkeywithMod(keycode);
                // if (get_mods()) {
                //     saved_mods= get_mods();
                // } else if (get_oneshot_mods()) {
                //     saved_mods=get_oneshot_mods();
                // } else {
                //     saved_mods=0;
                // }
                // mem_keycode = keycode;
            }
            return true; // for further processing

        case QK_LEAD:
            if (record->event.pressed) {
                countLeader = 0; // this has to be here, in leader_start this will reset on repeat as well
                // mem_keycode = keycode;
            }
            return true;

        case CLRMODS:
            if (record->event.pressed) {
                clear_mods();
                clear_oneshot_mods();
                held3 = false;
                held4 = false;
                held5 = false;
                held6 = false;
            }
            return false;

        case M_OS:
            if (record->event.pressed) {
                onMac = !onMac;
                onWin = !onWin; // flip os flag
            }
            return false; // dont process further

        case MOD_TOGG:
            if (record->event.pressed) {
                ModLock = !ModLock;
            }
            return false; // dont process further

        case MM_SPEED:
            if (record->event.pressed) {
                mk_max_speed += 2;
                Mouse_Setting_timer = timer_read32();
                if (mk_max_speed >= 12) {
                    mk_max_speed = 1;
                }
            }
            return false; // dont process further

        case MW_SPEED:
            if (record->event.pressed) {
                mk_wheel_max_speed += 3;
                Mouse_Setting_timer = timer_read32();
                if (mk_wheel_max_speed >= 25) {
                    mk_wheel_max_speed = 1;
                }
            }
            return false; // dont process further

        case LT(2, KC_SPACE):
            // if (!record->tap.count || record->tap.interrupted){
            if (!record->tap.count) {
                if (record->event.pressed) {
                    layer_on(2);
                    LLockInit = true;
                } else {
                    if (LLockedOnce) {
                        LLockInit   = false;
                        LLockedOnce = false;
                    } else {
                        LLockInit = false; // else this will be left true of other keys are used
                        layer_off(2);
                    }
                }
            } else {
                if (record->event.pressed) { // tap or interrupt
                    register_code(KC_SPACE);
                    MemkeywithMod(KC_SPACE);
                } else {
                    unregister_code(KC_SPACE);
                    if (is_caps_word_on()) {
                        caps_word_off();
                    }
                }
            }
            return false; // not process normally

            // case LT(2, KC_ENTER):
            // if (record->tap.count && record->event.pressed) { // tap
            // MemkeywithMod(KC_ENTER);
            //     if (recording1) {
            //     RecStore1[count1] =  mem_keycode;
            //         RecModsStore1[count1] = saved_mods;
            //     count1++;
            //     }
            // }
            // return true; // process normally

        case LT(1, KC_ESCAPE):
            if (record->tap.count && record->event.pressed) { // tap
                MemkeywithMod(KC_ESCAPE);
            } else {
                if (is_swap_hands_on()) {
                    swap_hands_off();
                }
            }
            return true; // process normally

        case ST_MACRO_0:
            if (record->event.pressed) {
                // SEND_STRING(SS_LGUI(SS_LSFT(SS_TAP(X_5))));
                SEND_STRING(SS_TAP(X_BTN1) SS_DELAY(25) SS_TAP(X_BTN1));
            }
            return false;

        case MAC_MISSION_CONTROL:
            HCS(0x29F);

        case RGB_SLD:
            if (record->event.pressed) {
                rgblight_mode(1);
            }
            return false;
    }
    return true;
}

typedef struct {
    bool    is_press_action;
    uint8_t step;
} tap;

enum { SINGLE_TAP = 1, SINGLE_HOLD, DOUBLE_TAP, DOUBLE_HOLD, DOUBLE_SINGLE_TAP, MORE_TAPS };

static tap dance_state[34]; // total tap dance number

uint8_t dance_step(tap_dance_state_t *state);

uint8_t dance_step(tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->interrupted || !state->pressed)
            return SINGLE_TAP;
        else
            return SINGLE_HOLD;
    } else if (state->count == 2) {
        if (state->interrupted)
            return DOUBLE_SINGLE_TAP;
        else if (state->pressed)
            return DOUBLE_HOLD;
        else
            return DOUBLE_TAP;
    }
    return MORE_TAPS;
}

void on_dance_0(tap_dance_state_t *state, void *user_data);
void dance_0_finished(tap_dance_state_t *state, void *user_data);
void dance_0_reset(tap_dance_state_t *state, void *user_data);

void on_dance_0(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_LABK);
        tap_code16(KC_LABK);
        tap_code16(KC_LABK);
    }
    if (state->count > 3) {
        tap_code16(KC_LABK);
    }
}

void dance_0_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[0].step = dance_step(state);
    switch (dance_state[0].step) {
        case SINGLE_TAP:
            register_code16(KC_LABK);
            MemkeywithMod(KC_LABK);
            break;
        case SINGLE_HOLD:
            register_code16(KC_RABK);
            MemkeywithMod(KC_RABK);
            break;
        case DOUBLE_TAP:
            register_code16(KC_LABK);
            register_code16(KC_LABK);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_LABK);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_LABK);
            register_code16(KC_LABK);
    }
}

void dance_0_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[0].step) {
        case SINGLE_TAP:
            unregister_code16(KC_LABK);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_RABK);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_LABK);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_LABK);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_LABK);
            break;
    }
    dance_state[0].step = 0;
}
void on_dance_1(tap_dance_state_t *state, void *user_data);
void dance_1_finished(tap_dance_state_t *state, void *user_data);
void dance_1_reset(tap_dance_state_t *state, void *user_data);

void on_dance_1(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_MINUS);
        tap_code16(KC_MINUS);
        tap_code16(KC_MINUS);
    }
    if (state->count > 3) {
        tap_code16(KC_MINUS);
    }
}

void dance_1_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[1].step = dance_step(state);
    switch (dance_state[1].step) {
        case SINGLE_TAP:
            register_code16(KC_MINUS);
            MemkeywithMod(KC_MINUS);
            break;
        case SINGLE_HOLD:
            register_code16(KC_UNDS);
            MemkeywithMod(KC_UNDS);
            break;
        case DOUBLE_TAP:
            register_code16(KC_MINUS);
            register_code16(KC_MINUS);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_MINUS);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_MINUS);
            register_code16(KC_MINUS);
    }
}

void dance_1_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[1].step) {
        case SINGLE_TAP:
            unregister_code16(KC_MINUS);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_UNDS);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_MINUS);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_MINUS);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_MINUS);
            break;
    }
    dance_state[1].step = 0;
}
void on_dance_2(tap_dance_state_t *state, void *user_data);
void dance_2_finished(tap_dance_state_t *state, void *user_data);
void dance_2_reset(tap_dance_state_t *state, void *user_data);

void on_dance_2(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_EQUAL);
        tap_code16(KC_EQUAL);
        tap_code16(KC_EQUAL);
    }
    if (state->count > 3) {
        tap_code16(KC_EQUAL);
    }
}

void dance_2_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[2].step = dance_step(state);
    switch (dance_state[2].step) {
        case SINGLE_TAP:
            register_code16(KC_EQUAL);
            MemkeywithMod(KC_EQUAL);
            break;
        case SINGLE_HOLD:
            register_code16(KC_PLUS);
            MemkeywithMod(KC_PLUS);
            break;
        case DOUBLE_TAP:
            register_code16(KC_EQUAL);
            register_code16(KC_EQUAL);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_EQUAL);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_EQUAL);
            register_code16(KC_EQUAL);
    }
}

void dance_2_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[2].step) {
        case SINGLE_TAP:
            unregister_code16(KC_EQUAL);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_PLUS);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_EQUAL);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_EQUAL);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_EQUAL);
            break;
    }
    dance_state[2].step = 0;
}
void on_dance_3(tap_dance_state_t *state, void *user_data);
void dance_3_finished(tap_dance_state_t *state, void *user_data);
void dance_3_reset(tap_dance_state_t *state, void *user_data);

void on_dance_3(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_GRAVE);
        tap_code16(KC_GRAVE);
        tap_code16(KC_GRAVE);
    }
    if (state->count > 3) {
        tap_code16(KC_GRAVE);
    }
}

void dance_3_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[3].step = dance_step(state);
    switch (dance_state[3].step) {
        case SINGLE_TAP:
            register_code16(KC_GRAVE);
            MemkeywithMod(KC_GRAVE);
            break;
        case SINGLE_HOLD:
            register_code16(KC_TILD);
            MemkeywithMod(KC_TILD);
            break;
        case DOUBLE_TAP:
            register_code16(KC_GRAVE);
            register_code16(KC_GRAVE);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_GRAVE);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_GRAVE);
            register_code16(KC_GRAVE);
    }
}

void dance_3_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[3].step) {
        case SINGLE_TAP:
            unregister_code16(KC_GRAVE);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_TILD);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_GRAVE);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_GRAVE);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_GRAVE);
            break;
    }
    dance_state[3].step = 0;
}
void on_dance_4(tap_dance_state_t *state, void *user_data);
void dance_4_finished(tap_dance_state_t *state, void *user_data);
void dance_4_reset(tap_dance_state_t *state, void *user_data);

void on_dance_4(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_QUOTE);
        tap_code16(KC_QUOTE);
        tap_code16(KC_QUOTE);
    }
    if (state->count > 3) {
        tap_code16(KC_QUOTE);
    }
}

void dance_4_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[4].step = dance_step(state);
    switch (dance_state[4].step) {
        case SINGLE_TAP:
            register_code16(KC_QUOTE);
            MemkeywithMod(KC_QUOTE);
            break;
        case SINGLE_HOLD:
            register_code16(KC_DQUO);
            MemkeywithMod(KC_DQUO);
            break;
        case DOUBLE_TAP:
            register_code16(KC_QUOTE);
            register_code16(KC_QUOTE);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_QUOTE);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_QUOTE);
            register_code16(KC_QUOTE);
    }
}

void dance_4_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[4].step) {
        case SINGLE_TAP:
            unregister_code16(KC_QUOTE);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_DQUO);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_QUOTE);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_QUOTE);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_QUOTE);
            break;
    }
    dance_state[4].step = 0;
}
void on_dance_5(tap_dance_state_t *state, void *user_data);
void dance_5_finished(tap_dance_state_t *state, void *user_data);
void dance_5_reset(tap_dance_state_t *state, void *user_data);

void on_dance_5(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_COMMA);
        tap_code16(KC_COMMA);
        tap_code16(KC_COMMA);
    }
    if (state->count > 3) {
        tap_code16(KC_COMMA);
    }
}

void dance_5_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[5].step = dance_step(state);
    switch (dance_state[5].step) {
        case SINGLE_TAP:
            register_code16(KC_COMMA);
            MemkeywithMod(KC_COMMA);
            break;
        case SINGLE_HOLD:
            register_code16(KC_SCLN);
            MemkeywithMod(KC_SCLN);
            break;
        case DOUBLE_TAP:
            register_code16(KC_COMMA);
            register_code16(KC_COMMA);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_COMMA);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_COMMA);
            register_code16(KC_COMMA);
    }
}

void dance_5_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[5].step) {
        case SINGLE_TAP:
            unregister_code16(KC_COMMA);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_SCLN);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_COMMA);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_COMMA);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_COMMA);
            break;
    }
    dance_state[5].step = 0;
}
void on_dance_6(tap_dance_state_t *state, void *user_data);
void dance_6_finished(tap_dance_state_t *state, void *user_data);
void dance_6_reset(tap_dance_state_t *state, void *user_data);

void on_dance_6(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_DOT);
        tap_code16(KC_DOT);
        tap_code16(KC_DOT);
    }
    if (state->count > 3) {
        tap_code16(KC_DOT);
    }
}

void dance_6_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[6].step = dance_step(state);
    switch (dance_state[6].step) {
        case SINGLE_TAP:
            register_code16(KC_DOT);
            MemkeywithMod(KC_DOT);
            break;
        case SINGLE_HOLD:
            register_code16(KC_COLN);
            MemkeywithMod(KC_COLN);
            break;
        case DOUBLE_TAP:
            register_code16(KC_DOT);
            register_code16(KC_DOT);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_DOT);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_DOT);
            register_code16(KC_DOT);
    }
}

void dance_6_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[6].step) {
        case SINGLE_TAP:
            unregister_code16(KC_DOT);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_COLN);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_DOT);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_DOT);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_DOT);
            break;
    }
    dance_state[6].step = 0;
}
void on_dance_7(tap_dance_state_t *state, void *user_data);
void dance_7_finished(tap_dance_state_t *state, void *user_data);
void dance_7_reset(tap_dance_state_t *state, void *user_data);

void on_dance_7(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_LCBR);
        tap_code16(KC_LCBR);
        tap_code16(KC_LCBR);
    }
    if (state->count > 3) {
        tap_code16(KC_LCBR);
    }
}

void dance_7_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[7].step = dance_step(state);
    switch (dance_state[7].step) {
        case SINGLE_TAP:
            register_code16(KC_LCBR);
            MemkeywithMod(KC_LCBR);
            break;
        case SINGLE_HOLD:
            register_code16(KC_RCBR);
            MemkeywithMod(KC_RCBR);
            break;
        case DOUBLE_TAP:
            register_code16(KC_LCBR);
            register_code16(KC_LCBR);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_LCBR);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_LCBR);
            register_code16(KC_LCBR);
    }
}

void dance_7_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[7].step) {
        case SINGLE_TAP:
            unregister_code16(KC_LCBR);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_RCBR);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_LCBR);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_LCBR);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_LCBR);
            break;
    }
    dance_state[7].step = 0;
}
void on_dance_8(tap_dance_state_t *state, void *user_data);
void dance_8_finished(tap_dance_state_t *state, void *user_data);
void dance_8_reset(tap_dance_state_t *state, void *user_data);

void on_dance_8(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_LBRC);
        tap_code16(KC_LBRC);
        tap_code16(KC_LBRC);
    }
    if (state->count > 3) {
        tap_code16(KC_LBRC);
    }
}

void dance_8_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[8].step = dance_step(state);
    switch (dance_state[8].step) {
        case SINGLE_TAP:
            register_code16(KC_LBRC);
            MemkeywithMod(KC_LBRC);
            break;
        case SINGLE_HOLD:
            register_code16(KC_RBRC);
            MemkeywithMod(KC_RBRC);
            break;
        case DOUBLE_TAP:
            register_code16(KC_LBRC);
            register_code16(KC_LBRC);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_LBRC);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_LBRC);
            register_code16(KC_LBRC);
    }
}

void dance_8_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[8].step) {
        case SINGLE_TAP:
            unregister_code16(KC_LBRC);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_RBRC);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_LBRC);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_LBRC);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_LBRC);
            break;
    }
    dance_state[8].step = 0;
}
void on_dance_9(tap_dance_state_t *state, void *user_data);
void dance_9_finished(tap_dance_state_t *state, void *user_data);
void dance_9_reset(tap_dance_state_t *state, void *user_data);

void on_dance_9(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_SLASH);
        tap_code16(KC_SLASH);
        tap_code16(KC_SLASH);
    }
    if (state->count > 3) {
        tap_code16(KC_SLASH);
    }
}

void dance_9_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[9].step = dance_step(state);
    switch (dance_state[9].step) {
        case SINGLE_TAP:
            register_code16(KC_SLASH);
            MemkeywithMod(KC_SLASH);
            break;
        case SINGLE_HOLD:
            register_code16(KC_BSLS);
            MemkeywithMod(KC_BSLS);

            break;
        case DOUBLE_TAP:
            register_code16(KC_SLASH);
            register_code16(KC_SLASH);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_SLASH);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_SLASH);
            register_code16(KC_SLASH);
    }
}

void dance_9_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[9].step) {
        case SINGLE_TAP:
            unregister_code16(KC_SLASH);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_BSLS);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_SLASH);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_SLASH);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_SLASH);
            break;
    }
    dance_state[9].step = 0;
}
void on_dance_10(tap_dance_state_t *state, void *user_data);
void dance_10_finished(tap_dance_state_t *state, void *user_data);
void dance_10_reset(tap_dance_state_t *state, void *user_data);

void on_dance_10(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_ASTR);
        tap_code16(KC_ASTR);
        tap_code16(KC_ASTR);
    }
    if (state->count > 3) {
        tap_code16(KC_ASTR);
    }
}

void dance_10_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[10].step = dance_step(state);
    switch (dance_state[10].step) {
        case SINGLE_TAP:
            register_code16(KC_ASTR);
            MemkeywithMod(KC_ASTR);

            break;
        case SINGLE_HOLD:
            register_code16(KC_CIRC);
            MemkeywithMod(KC_CIRC);
            break;
        case DOUBLE_TAP:
            register_code16(KC_ASTR);
            register_code16(KC_ASTR);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_ASTR);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_ASTR);
            register_code16(KC_ASTR);
    }
}

void dance_10_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[10].step) {
        case SINGLE_TAP:
            unregister_code16(KC_ASTR);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_CIRC);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_ASTR);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_ASTR);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_ASTR);
            break;
    }
    dance_state[10].step = 0;
}
void on_dance_11(tap_dance_state_t *state, void *user_data);
void dance_11_finished(tap_dance_state_t *state, void *user_data);
void dance_11_reset(tap_dance_state_t *state, void *user_data);

void on_dance_11(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_7);
        tap_code16(KC_7);
        tap_code16(KC_7);
    }
    if (state->count > 3) {
        tap_code16(KC_7);
    }
}

void dance_11_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[11].step = dance_step(state);
    switch (dance_state[11].step) {
        case SINGLE_TAP:
            register_code16(KC_7);
            MemkeywithMod(KC_7);

            break;
        case SINGLE_HOLD:
            register_code16(KC_AMPR);
            MemkeywithMod(KC_AMPR);

            break;
        case DOUBLE_TAP:
            register_code16(KC_7);
            register_code16(KC_7);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_7);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_7);
            register_code16(KC_7);
    }
}

void dance_11_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[11].step) {
        case SINGLE_TAP:
            unregister_code16(KC_7);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_AMPR);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_7);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_7);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_7);
            break;
    }
    dance_state[11].step = 0;
}
void on_dance_12(tap_dance_state_t *state, void *user_data);
void dance_12_finished(tap_dance_state_t *state, void *user_data);
void dance_12_reset(tap_dance_state_t *state, void *user_data);

void on_dance_12(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_8);
        tap_code16(KC_8);
        tap_code16(KC_8);
    }
    if (state->count > 3) {
        tap_code16(KC_8);
    }
}

void dance_12_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[12].step = dance_step(state);
    switch (dance_state[12].step) {
        case SINGLE_TAP:
            register_code16(KC_8);
            MemkeywithMod(KC_8);
            break;
        case SINGLE_HOLD:
            register_code16(KC_LPRN);
            MemkeywithMod(KC_LPRN);

            break;
        case DOUBLE_TAP:
            register_code16(KC_8);
            register_code16(KC_8);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_8);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_8);
            register_code16(KC_8);
    }
}

void dance_12_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[12].step) {
        case SINGLE_TAP:
            unregister_code16(KC_8);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_LPRN);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_8);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_8);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_8);
            break;
    }
    dance_state[12].step = 0;
}
void on_dance_13(tap_dance_state_t *state, void *user_data);
void dance_13_finished(tap_dance_state_t *state, void *user_data);
void dance_13_reset(tap_dance_state_t *state, void *user_data);

void on_dance_13(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_9);
        tap_code16(KC_9);
        tap_code16(KC_9);
    }
    if (state->count > 3) {
        tap_code16(KC_9);
    }
}

void dance_13_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[13].step = dance_step(state);
    switch (dance_state[13].step) {
        case SINGLE_TAP:
            register_code16(KC_9);
            MemkeywithMod(KC_9);

            break;
        case SINGLE_HOLD:
            register_code16(KC_RPRN);
            MemkeywithMod(KC_RPRN);

            break;
        case DOUBLE_TAP:
            register_code16(KC_9);
            register_code16(KC_9);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_9);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_9);
            register_code16(KC_9);
    }
}

void dance_13_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[13].step) {
        case SINGLE_TAP:
            unregister_code16(KC_9);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_RPRN);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_9);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_9);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_9);
            break;
    }
    dance_state[13].step = 0;
}
void on_dance_14(tap_dance_state_t *state, void *user_data);
void dance_14_finished(tap_dance_state_t *state, void *user_data);
void dance_14_reset(tap_dance_state_t *state, void *user_data);

void on_dance_14(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_4);
        tap_code16(KC_4);
        tap_code16(KC_4);
    }
    if (state->count > 3) {
        tap_code16(KC_4);
    }
}

void dance_14_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[14].step = dance_step(state);
    switch (dance_state[14].step) {
        case SINGLE_TAP:
            register_code16(KC_4);
            MemkeywithMod(KC_4);

            break;
        case SINGLE_HOLD:
            register_code16(KC_DLR);
            MemkeywithMod(KC_DLR);

            break;
        case DOUBLE_TAP:
            register_code16(KC_4);
            register_code16(KC_4);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_4);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_4);
            register_code16(KC_4);
    }
}

void dance_14_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[14].step) {
        case SINGLE_TAP:
            unregister_code16(KC_4);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_DLR);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_4);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_4);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_4);
            break;
    }
    dance_state[14].step = 0;
}
void on_dance_15(tap_dance_state_t *state, void *user_data);
void dance_15_finished(tap_dance_state_t *state, void *user_data);
void dance_15_reset(tap_dance_state_t *state, void *user_data);

void on_dance_15(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_5);
        tap_code16(KC_5);
        tap_code16(KC_5);
    }
    if (state->count > 3) {
        tap_code16(KC_5);
    }
}

void dance_15_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[15].step = dance_step(state);
    switch (dance_state[15].step) {
        case SINGLE_TAP:
            register_code16(KC_5);
            MemkeywithMod(KC_5);

            break;
        case SINGLE_HOLD:
            register_code16(KC_PERC);
            MemkeywithMod(KC_PERC);

            break;
        case DOUBLE_TAP:
            register_code16(KC_5);
            register_code16(KC_5);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_5);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_5);
            register_code16(KC_5);
    }
}

void dance_15_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[15].step) {
        case SINGLE_TAP:
            unregister_code16(KC_5);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_PERC);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_5);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_5);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_5);
            break;
    }
    dance_state[15].step = 0;
}
void on_dance_16(tap_dance_state_t *state, void *user_data);
void dance_16_finished(tap_dance_state_t *state, void *user_data);
void dance_16_reset(tap_dance_state_t *state, void *user_data);

void on_dance_16(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_6);
        tap_code16(KC_6);
        tap_code16(KC_6);
    }
    if (state->count > 3) {
        tap_code16(KC_6);
    }
}

void dance_16_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[16].step = dance_step(state);
    switch (dance_state[16].step) {
        case SINGLE_TAP:
            register_code16(KC_6);
            MemkeywithMod(KC_6);

            break;
        case SINGLE_HOLD:
            register_code16(KC_QUES);
            MemkeywithMod(KC_QUES);

            break;
        case DOUBLE_TAP:
            register_code16(KC_6);
            register_code16(KC_6);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_6);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_6);
            register_code16(KC_6);
    }
}

void dance_16_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[16].step) {
        case SINGLE_TAP:
            unregister_code16(KC_6);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_QUES);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_6);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_6);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_6);
            break;
    }
    dance_state[16].step = 0;
}
void on_dance_17(tap_dance_state_t *state, void *user_data);
void dance_17_finished(tap_dance_state_t *state, void *user_data);
void dance_17_reset(tap_dance_state_t *state, void *user_data);

void on_dance_17(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_0);
        tap_code16(KC_0);
        tap_code16(KC_0);
    }
    if (state->count > 3) {
        tap_code16(KC_0);
    }
}

void dance_17_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[17].step = dance_step(state);
    switch (dance_state[17].step) {
        case SINGLE_TAP:
            register_code16(KC_0);
            MemkeywithMod(KC_0);

            break;
        case SINGLE_HOLD:
            register_code16(KC_PIPE);
            MemkeywithMod(KC_PIPE);

            break;
        case DOUBLE_TAP:
            register_code16(KC_0);
            register_code16(KC_0);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_0);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_0);
            register_code16(KC_0);
    }
}

void dance_17_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[17].step) {
        case SINGLE_TAP:
            unregister_code16(KC_0);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_PIPE);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_0);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_0);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_0);
            break;
    }
    dance_state[17].step = 0;
}
void on_dance_18(tap_dance_state_t *state, void *user_data);
void dance_18_finished(tap_dance_state_t *state, void *user_data);
void dance_18_reset(tap_dance_state_t *state, void *user_data);

void on_dance_18(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_1);
        tap_code16(KC_1);
        tap_code16(KC_1);
    }
    if (state->count > 3) {
        tap_code16(KC_1);
    }
}

void dance_18_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[18].step = dance_step(state);
    switch (dance_state[18].step) {
        case SINGLE_TAP:
            register_code16(KC_1);
            MemkeywithMod(KC_1);

            break;
        case SINGLE_HOLD:
            register_code16(KC_EXLM);
            MemkeywithMod(KC_EXLM);

            break;
        case DOUBLE_TAP:
            register_code16(KC_1);
            register_code16(KC_1);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_1);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_1);
            register_code16(KC_1);
    }
}

void dance_18_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[18].step) {
        case SINGLE_TAP:
            unregister_code16(KC_1);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_EXLM);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_1);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_1);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_1);
            break;
    }
    dance_state[18].step = 0;
}
void on_dance_19(tap_dance_state_t *state, void *user_data);
void dance_19_finished(tap_dance_state_t *state, void *user_data);
void dance_19_reset(tap_dance_state_t *state, void *user_data);

void on_dance_19(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_2);
        tap_code16(KC_2);
        tap_code16(KC_2);
    }
    if (state->count > 3) {
        tap_code16(KC_2);
    }
}

void dance_19_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[19].step = dance_step(state);
    switch (dance_state[19].step) {
        case SINGLE_TAP:
            register_code16(KC_2);
            MemkeywithMod(KC_2);

            break;
        case SINGLE_HOLD:
            register_code16(KC_AT);
            MemkeywithMod(KC_AT);

            break;
        case DOUBLE_TAP:
            register_code16(KC_2);
            register_code16(KC_2);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_2);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_2);
            register_code16(KC_2);
    }
}

void dance_19_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[19].step) {
        case SINGLE_TAP:
            unregister_code16(KC_2);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_AT);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_2);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_2);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_2);
            break;
    }
    dance_state[19].step = 0;
}
void on_dance_20(tap_dance_state_t *state, void *user_data);
void dance_20_finished(tap_dance_state_t *state, void *user_data);
void dance_20_reset(tap_dance_state_t *state, void *user_data);

void on_dance_20(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_3);
        tap_code16(KC_3);
        tap_code16(KC_3);
    }
    if (state->count > 3) {
        tap_code16(KC_3);
    }
}

void dance_20_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[20].step = dance_step(state);
    switch (dance_state[20].step) {
        case SINGLE_TAP:
            register_code16(KC_3);
            MemkeywithMod(KC_3);

            break;
        case SINGLE_HOLD:
            register_code16(KC_HASH);
            MemkeywithMod(KC_HASH);

            break;
        case DOUBLE_TAP:
            register_code16(KC_3);
            register_code16(KC_3);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_3);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_3);
            register_code16(KC_3);
    }
}

void dance_20_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[20].step) {
        case SINGLE_TAP:
            unregister_code16(KC_3);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_HASH);
            if (is_caps_word_on()) {
                caps_word_off();
            }
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_3);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_3);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_3);
            break;
    }
    dance_state[20].step = 0;
}
void on_dance_21(tap_dance_state_t *state, void *user_data);
void dance_21_finished(tap_dance_state_t *state, void *user_data);
void dance_21_reset(tap_dance_state_t *state, void *user_data);

void on_dance_21(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_AUDIO_VOL_DOWN);
        tap_code16(KC_AUDIO_VOL_DOWN);
        tap_code16(KC_AUDIO_VOL_DOWN);
    }
    if (state->count > 3) {
        tap_code16(KC_AUDIO_VOL_DOWN);
    }
}

void dance_21_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[21].step = dance_step(state);
    switch (dance_state[21].step) {
        case SINGLE_TAP:
            tap_code16(KC_AUDIO_MUTE);
            break;
        case SINGLE_HOLD:
            register_code16(KC_AUDIO_VOL_DOWN);
            break;
        case DOUBLE_TAP:
            tap_code16(KC_AUDIO_VOL_DOWN);
            tap_code16(KC_AUDIO_VOL_DOWN);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_AUDIO_VOL_DOWN);
            tap_code16(KC_AUDIO_VOL_DOWN);
    }
}

void dance_21_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[21].step) {
        case SINGLE_TAP:
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_AUDIO_VOL_DOWN);
            break;
        case DOUBLE_TAP:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
    dance_state[21].step = 0;
}
void dance_22_finished(tap_dance_state_t *state, void *user_data);
void dance_22_reset(tap_dance_state_t *state, void *user_data);

void dance_22_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->interrupted || state->pressed) {
            layer_on(2);
            Layer2Hold = true;
        } else {
            layer_move(2);
        }
    }
}

void dance_22_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    if (Layer2Hold) {
        layer_off(2);
        Layer2Hold = false;
    }
}
void on_dance_23(tap_dance_state_t *state, void *user_data);
void dance_23_finished(tap_dance_state_t *state, void *user_data);
void dance_23_reset(tap_dance_state_t *state, void *user_data);

void on_dance_23(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_HOME);
        tap_code16(KC_HOME);
        tap_code16(KC_HOME);
    }
    if (state->count > 3) {
        tap_code16(KC_HOME);
    }
}

void dance_23_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[23].step = dance_step(state);
    switch (dance_state[23].step) {
        case SINGLE_TAP:
            register_code16(KC_HOME);
            break;
        case SINGLE_HOLD:
            register_code16(KC_END);
            break;
        case DOUBLE_TAP:
            register_code16(KC_HOME);
            register_code16(KC_HOME);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_HOME);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_HOME);
            register_code16(KC_HOME);
    }
}

void dance_23_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[23].step) {
        case SINGLE_TAP:
            unregister_code16(KC_HOME);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_END);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_HOME);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_HOME);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_HOME);
            break;
    }
    dance_state[23].step = 0;
}
void on_dance_24(tap_dance_state_t *state, void *user_data);
void dance_24_finished(tap_dance_state_t *state, void *user_data);
void dance_24_reset(tap_dance_state_t *state, void *user_data);

void on_dance_24(tap_dance_state_t *state, void *user_data) {
    if (state->count > 1) {
    }
}

void dance_24_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->interrupted || state->pressed) { // hold
            register_code16(KC_MS_ACCEL0);
            MouseB0Hold = true;
        } else {
            held1 = !held1; // Toggle between holding and releasing.
            if (held1) {    // Press
                register_code16(KC_MS_ACCEL0);
            } else { // Release
                unregister_code16(KC_MS_ACCEL0);
            }
        }
    }
}

void dance_24_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    if (MouseB0Hold) {
        unregister_code16(KC_MS_ACCEL0);
        MouseB0Hold = false;
    }
}
void on_dance_25(tap_dance_state_t *state, void *user_data);
void dance_25_finished(tap_dance_state_t *state, void *user_data);
void dance_25_reset(tap_dance_state_t *state, void *user_data);

void on_dance_25(tap_dance_state_t *state, void *user_data) {
    if (state->count > 1) {
    }
}

void dance_25_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->interrupted || state->pressed) { // hold
            register_code16(KC_MS_ACCEL2);
            MouseB2Hold = true;
        } else {
            held2 = !held2; // Toggle between holding and releasing.
            if (held2) {    // Press
                register_code16(KC_MS_ACCEL2);
            } else { // Release
                unregister_code16(KC_MS_ACCEL2);
            }
        }
    }
}

void dance_25_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    if (MouseB2Hold) {
        unregister_code16(KC_MS_ACCEL2);
        MouseB2Hold = false;
    }
}

void on_dance_26(tap_dance_state_t *state, void *user_data);
void dance_26_finished(tap_dance_state_t *state, void *user_data);
void dance_26_reset(tap_dance_state_t *state, void *user_data);

void on_dance_26(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_PAGE_UP);
        tap_code16(KC_PAGE_UP);
        tap_code16(KC_PAGE_UP);
    }
    if (state->count > 3) {
        tap_code16(KC_PAGE_UP);
    }
}

void dance_26_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[26].step = dance_step(state);
    switch (dance_state[26].step) {
        case SINGLE_TAP:
            register_code16(KC_PAGE_UP);
            break;
        case SINGLE_HOLD:
            register_code16(KC_PGDN);
            break;
        case DOUBLE_TAP:
            register_code16(KC_PAGE_UP);
            register_code16(KC_PAGE_UP);
            break;
        case DOUBLE_HOLD:
            register_code16(KC_PAGE_UP);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_PAGE_UP);
            register_code16(KC_PAGE_UP);
    }
}

void dance_26_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[26].step) {
        case SINGLE_TAP:
            unregister_code16(KC_PAGE_UP);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_PGDN);
            break;
        case DOUBLE_TAP:
            unregister_code16(KC_PAGE_UP);
            break;
        case DOUBLE_HOLD:
            unregister_code16(KC_PAGE_UP);
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_PAGE_UP);
            break;
    }
    dance_state[26].step = 0;
}

void on_dance_27(tap_dance_state_t *state, void *user_data);
void dance_27_finished(tap_dance_state_t *state, void *user_data);
void dance_27_reset(tap_dance_state_t *state, void *user_data);

void on_dance_27(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        register_mods(saved_mods);
        tap_code16(mem_keycode);
        tap_code16(mem_keycode);
        tap_code16(mem_keycode);
        unregister_mods(saved_mods);
    }
}

void dance_27_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[27].step = dance_step(state);
    switch (dance_state[27].step) {
        case SINGLE_TAP:
            register_mods(saved_mods);
            tap_code16(mem_keycode);
            unregister_mods(saved_mods);
            break;
        case SINGLE_HOLD:
            register_mods(saved_mods);
            register_code16(mem_keycode);
            break;
        case DOUBLE_TAP:
            register_mods(saved_mods);
            tap_code16(mem_keycode);
            tap_code16(mem_keycode);
            unregister_mods(saved_mods);
            break;
        case DOUBLE_HOLD:
            register_mods(saved_mods);
            tap_code16(mem_keycode);
            tap_code16(mem_keycode);
            tap_code16(mem_keycode);
            tap_code16(mem_keycode);
            unregister_mods(saved_mods);
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
}

void dance_27_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[27].step) {
        case SINGLE_TAP:
            break;
        case SINGLE_HOLD:
            unregister_code16(mem_keycode);
            unregister_mods(saved_mods);
            break;
        case DOUBLE_TAP:
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
    dance_state[27].step = 0;
}

void on_dance_28(tap_dance_state_t *state, void *user_data);
void dance_28_finished(tap_dance_state_t *state, void *user_data);
void dance_28_reset(tap_dance_state_t *state, void *user_data);

void on_dance_28(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_BSPC);
        tap_code16(KC_BSPC);
        tap_code16(KC_BSPC);
    }
    if (state->count > 3) {
        tap_code16(KC_BSPC);
    }
}

void dance_28_finished(tap_dance_state_t *state, void *user_data) { // WRONG incorrect
    dance_state[28].step = dance_step(state);
    switch (dance_state[28].step) {
        case SINGLE_TAP:
            register_code16(KC_BSPC);
            if (recording1) {
                RecStore1[count1] = KC_BSPC; // recording only for tap fun
                count1++;
            }
            break;
        case SINGLE_HOLD:
            register_code16(KC_BSPC);
            break;
        case DOUBLE_TAP:
            tap_code16(KC_BSPC);
            break;
        case DOUBLE_HOLD:
            layer_on(2);
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_BSPC);
            break;
    }
}

void dance_28_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[28].step) {
        case SINGLE_TAP:
            unregister_code16(KC_BSPC);
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_BSPC);
            break;
        case DOUBLE_TAP:
            tap_code16(KC_BSPC);
            break;
        case DOUBLE_HOLD:
            layer_off(2);
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
    dance_state[28].step = 0;
}

void on_dance_29(tap_dance_state_t *state, void *user_data);
void dance_29_finished(tap_dance_state_t *state, void *user_data);
void dance_29_reset(tap_dance_state_t *state, void *user_data);

void on_dance_29(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        leader_start();
        for (int i = 0; i < countLeader; i++) {
            leader_sequence_add(mem_keycode_Leader[i]);
        }
        leader_end();
        wait_ms(35);
        leader_start();
        for (int i = 0; i < countLeader; i++) {
            leader_sequence_add(mem_keycode_Leader[i]);
        }
        leader_end();
        wait_ms(35);
        leader_start();
        for (int i = 0; i < countLeader; i++) {
            leader_sequence_add(mem_keycode_Leader[i]);
        }
        leader_end();
    }
    if (state->count > 3) {
    }
}

void dance_29_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[29].step = dance_step(state);
    switch (dance_state[29].step) {
        case SINGLE_TAP:
            leader_start();
            // for (int i = 0; i < countLeader; i++) {
            //     leader_sequence_add(mem_keycode_Leader[i]);
            // }
            // leader_end();
            break;
        case SINGLE_HOLD:
            LeaderRepPressed = true;
            break;
        case DOUBLE_TAP:
            leader_start();
            for (int i = 0; i < countLeader; i++) {
                leader_sequence_add(mem_keycode_Leader[i]);
            }
            leader_end();
            wait_ms(35);
            leader_start();
            for (int i = 0; i < countLeader; i++) {
                leader_sequence_add(mem_keycode_Leader[i]);
            }
            leader_end();
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
}

void dance_29_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[29].step) {
        case SINGLE_TAP:
            break;
        case SINGLE_HOLD:
            LeaderRepPressed = false;
            break;
        case DOUBLE_TAP:
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
    dance_state[29].step = 0;
}

void dance_30_finished(tap_dance_state_t *state, void *user_data);
void dance_30_reset(tap_dance_state_t *state, void *user_data);

void dance_30_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->interrupted || state->pressed) {
            layer_on(1);
            Layer1Hold = true;
        } else {
            layer_move(1);
        }
    }
}

void dance_30_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    if (Layer1Hold) {
        layer_off(1);
        Layer1Hold = false;
    }
}

void on_dance_31(tap_dance_state_t *state, void *user_data);
void dance_31_finished(tap_dance_state_t *state, void *user_data);
void dance_31_reset(tap_dance_state_t *state, void *user_data);

void on_dance_31(tap_dance_state_t *state, void *user_data) {
    if (state->count == 3) {
        tap_code16(KC_MS_BTN1);
        tap_code16(KC_MS_BTN1);
        tap_code16(KC_MS_BTN1);
    }
    if (state->count > 3) {
        tap_code16(KC_MS_BTN1);
    }
}

void dance_31_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[31].step = dance_step(state);
    switch (dance_state[31].step) {
        case SINGLE_TAP:
            if (held7 == false) { // Press
                register_code16(KC_MS_BTN1);
            } else { // Release
                unregister_code16(KC_MS_BTN1);
                held7 = false;
            }
            break;

        case SINGLE_HOLD:
            register_code16(KC_MS_BTN1);
            break;
        case DOUBLE_TAP:
            // tap_code16(KC_MS_BTN1);
            register_code16(KC_MS_BTN1);
            held7 = true;
            break;
        case DOUBLE_HOLD: // register_code16(KC_MS_BTN1);
            // held7 = true;
            break;
        case DOUBLE_SINGLE_TAP:
            tap_code16(KC_MS_BTN1);
            register_code16(KC_MS_BTN1);
            break;
    }
}

void dance_31_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[31].step) {
        case SINGLE_TAP:
            if (held7 == false) {
                unregister_code16(KC_MS_BTN1);
            }
            break;
        case SINGLE_HOLD:
            unregister_code16(KC_MS_BTN1);
            break;
        case DOUBLE_TAP: // tap_code16(KC_MS_BTN1);
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            unregister_code16(KC_MS_BTN1);
            break;
    }
    dance_state[31].step = 0;
}

void on_dance_R1(tap_dance_state_t *state, void *user_data);
void dance_R1_finished(tap_dance_state_t *state, void *user_data);
void dance_R1_reset(tap_dance_state_t *state, void *user_data);

void on_dance_R1(tap_dance_state_t *state, void *user_data) {}

void dance_R1_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[32].step = dance_step(state);
    switch (dance_state[32].step) {
        case SINGLE_TAP:
            for (int i = 0; i < count1; i++) { // impelement set_oneshot_mods(MOD_BIT(KC_RCTL)) for mods
                // if (((RecStore1[i] == KC_RSFT)||(RecStore1[i] == KC_RCTL)||(RecStore1[i] == KC_RALT)||(RecStore1[i] == KC_RGUI))&&(i!=(count1-1))) { //if mod is last key it wont unregister
                //  //set_oneshot_mods(MOD_BIT(RecStore1[i]));
                //     register_code16(RecStore1[i]);
                //  wait_ms(10);
                // } else {
                // register_code16(RecStore1[i]);
                // wait_ms(10);
                // unregister_code16(RecStore1[i]);
                // wait_ms(10);
                // if ((RecStore1[i-1] == KC_RSFT)||(RecStore1[i-1] == KC_RCTL)||(RecStore1[i-1] == KC_RALT)||(RecStore1[i-1] == KC_RGUI)) {
                //  //set_oneshot_mods(MOD_BIT(RecStore1[i]));
                //     unregister_code16(RecStore1[i-1]);   // unregister mods
                //     wait_ms(10);
                //     }
                // }
                register_mods(RecModsStore1[i]);
                tap_code16(RecStore1[i]);
                wait_ms(25);
                unregister_mods(RecModsStore1[i]);
            }
            break;
        case SINGLE_HOLD:
            if (recording1 == false) {
                recording1 = true; // rec start
                count1     = 0;
            } else {
                recording1 = false; // rec stop
            }
            break;
        case DOUBLE_TAP:
            for (int j = 0; j < 2; j++) {
                for (int i = 0; i < count1; i++) {
                    register_mods(RecModsStore1[i]);
                    tap_code16(RecStore1[i]);
                    wait_ms(25);
                    unregister_mods(RecModsStore1[i]);
                }
            }
            // for (int i = 0; i < count1; i++) {
            //         register_mods(RecModsStore1[i]);
            //         wait_ms(5);
            //         register_code16(RecStore1[i]);
            //         wait_ms(5);
            //         unregister_code16(RecStore1[i]);
            //         wait_ms(5);
            //         unregister_mods(RecModsStore1[i]);
            //         wait_ms(5);
            //     }
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
}

void dance_R1_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[32].step) {
        case SINGLE_TAP:
            break;
        case SINGLE_HOLD:
            break;
        case DOUBLE_TAP:
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
    dance_state[32].step = 0;
}

void on_dance_R2(tap_dance_state_t *state, void *user_data);
void dance_R2_finished(tap_dance_state_t *state, void *user_data);
void dance_R2_reset(tap_dance_state_t *state, void *user_data);

void on_dance_R2(tap_dance_state_t *state, void *user_data) {}

void dance_R2_finished(tap_dance_state_t *state, void *user_data) {
    dance_state[33].step = dance_step(state);
    switch (dance_state[33].step) {
        case SINGLE_TAP:
            if (Mem2RepPressed) {
                Mem2RepPressed = false;
            } else {
                for (int i = 0; i < count2; i++) {
                    register_mods(RecModsStore2[i]);
                    tap_code16(RecStore2[i]);
                    unregister_mods(RecModsStore2[i]);
                    wait_ms(10);
                }
            }
            break;
        case SINGLE_HOLD:
            Mem2RepPressed = true;
            break;
        case DOUBLE_TAP:
            count2 = count1;
            for (int i = 0; i < count2; i++) {
                RecStore2[i]     = RecStore1[i];
                RecModsStore2[i] = RecModsStore1[i];
            }
            break; // modify later to reset Rec2
        case DOUBLE_HOLD:
            Mem2RepPressed = true;
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
}

void dance_R2_reset(tap_dance_state_t *state, void *user_data) {
    wait_ms(10);
    switch (dance_state[33].step) {
        case SINGLE_TAP:
            break;
        case SINGLE_HOLD:
            Mem2RepPressed = false;
            break;
        case DOUBLE_TAP:
            break;
        case DOUBLE_HOLD:
            break;
        case DOUBLE_SINGLE_TAP:
            break;
    }
    dance_state[33].step = 0;
}

tap_dance_action_t tap_dance_actions[] = {
    [DANCE_0] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_0, dance_0_finished, dance_0_reset),     [DANCE_1] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_1, dance_1_finished, dance_1_reset),     [DANCE_2] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_2, dance_2_finished, dance_2_reset),     [DANCE_3] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_3, dance_3_finished, dance_3_reset),     [DANCE_4] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_4, dance_4_finished, dance_4_reset),     [DANCE_5] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_5, dance_5_finished, dance_5_reset),     [DANCE_6] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_6, dance_6_finished, dance_6_reset),     [DANCE_7] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_7, dance_7_finished, dance_7_reset),     [DANCE_8] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_8, dance_8_finished, dance_8_reset),
    [DANCE_9] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_9, dance_9_finished, dance_9_reset),     [DANCE_10] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_10, dance_10_finished, dance_10_reset), [DANCE_11] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_11, dance_11_finished, dance_11_reset), [DANCE_12] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_12, dance_12_finished, dance_12_reset), [DANCE_13] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_13, dance_13_finished, dance_13_reset), [DANCE_14] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_14, dance_14_finished, dance_14_reset), [DANCE_15] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_15, dance_15_finished, dance_15_reset), [DANCE_16] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_16, dance_16_finished, dance_16_reset), [DANCE_17] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_17, dance_17_finished, dance_17_reset),
    [DANCE_18] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_18, dance_18_finished, dance_18_reset), [DANCE_19] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_19, dance_19_finished, dance_19_reset), [DANCE_20] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_20, dance_20_finished, dance_20_reset), [DANCE_21] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_21, dance_21_finished, dance_21_reset), [DANCE_22] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_22_finished, dance_22_reset),        [DANCE_23] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_23, dance_23_finished, dance_23_reset), [DANCE_24] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_24, dance_24_finished, dance_24_reset), [DANCE_25] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_25, dance_25_finished, dance_25_reset), [DANCE_26] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_26, dance_26_finished, dance_26_reset),
    [DANCE_27] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_27, dance_27_finished, dance_27_reset), [DANCE_28] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_28, dance_28_finished, dance_28_reset), [DANCE_29] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_29, dance_29_finished, dance_29_reset), [DANCE_30] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, dance_30_finished, dance_30_reset),        [DANCE_31] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_31, dance_31_finished, dance_31_reset), [DANCE_R1] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_R1, dance_R1_finished, dance_R1_reset), [DANCE_R2] = ACTION_TAP_DANCE_FN_ADVANCED(on_dance_R2, dance_R2_finished, dance_R2_reset),
};

// void oneshot_locked_mods_changed_user(uint8_t mods) {
//    if (mods & MOD_MASK_SHIFT) {
//     STATUS_LED_1(true);
//    }
//     else {
//      STATUS_LED_1(false);
//     }
//   if (mods & MOD_MASK_CTRL) {
//     STATUS_LED_2(true);
//   }
//   else {
//     STATUS_LED_2(false);
//   }
//   if (mods & MOD_MASK_ALT) {
//     STATUS_LED_3(true);
//   }
//   else {
//     STATUS_LED_3(false);
//   }
//   if (mods & MOD_MASK_GUI) {
//     STATUS_LED_4(true);
//   }
//   else {
//     STATUS_LED_4(false);
//   }
// }

bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        // Keycodes that continue Caps Word, with shift applied.
        case KC_A ... KC_Z:
            add_weak_mods(MOD_BIT(KC_LSFT)); // Apply shift to next key.
            return true;

        // Keycodes that continue Caps Word, without shifting.
        case TD(DANCE_1):
        case TD(DANCE_11):
        case TD(DANCE_12):
        case TD(DANCE_13):
        case TD(DANCE_14):
        case TD(DANCE_15):
        case TD(DANCE_16):
        case TD(DANCE_17):
        case TD(DANCE_18):
        case TD(DANCE_19):
        case TD(DANCE_20):
        case TD(DANCE_30):
        case KC_BSPC:
        case KC_DEL:
        case KC_UNDS:
            return true;

        default:
            return false; // Deactivate Caps Word.
    }
}

void leader_start_user(void) {
    // Do something when the leader key is pressed
    if (OSdetectedOnce == false) { // remove this if post init user works
        if (detected_host_os() == OS_MACOS || detected_host_os() == OS_IOS) {
            onMac = true;
            onWin = false;
        } else {
            onMac = false;
            onWin = true;
        }
        OSdetectedOnce = true;
    }
    LeadExec = false;
}

void leader_end_user(void) {
    //// ONE
    if (leader_sequence_one_key(KC_S)) {
        mem_keycode_Leader[0] = KC_S;
        countLeader           = 1;
        if (onMac) {
            SEND_STRING("Suryansh :Mac\n");
            LeadExec = true;
        } else if (onWin) {
            SEND_STRING("Suryansh :Windows\n");
            LeadExec = true;
        }
    }

    else if (leader_sequence_one_key(KC_L)) { // select left word by word
        mem_keycode_Leader[0] = KC_L;
        countLeader           = 1;
        if (onMac) {
            tap_code16(RALT(RSFT(KC_LEFT)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RCTL(RSFT(KC_LEFT)));
            LeadExec = true;
        }
    }

    else if (leader_sequence_one_key(KC_R)) { // select right word by word
        mem_keycode_Leader[0] = KC_R;
        countLeader           = 1;
        if (onMac) {
            tap_code16(RALT(RSFT(KC_RIGHT)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RCTL(RSFT(KC_RIGHT)));
            LeadExec = true;
        }
    }

    //// TWO

    else if (leader_sequence_two_keys(KC_P, KC_R)) {
        mem_keycode_Leader[0] = KC_P;
        mem_keycode_Leader[1] = KC_R;
        countLeader           = 2;
        SEND_STRING("suryansh");
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_G, KC_C)) { // git commit
        mem_keycode_Leader[0] = KC_G;
        mem_keycode_Leader[1] = KC_C;
        countLeader           = 2;
        // SEND_STRING("git add .");
        // tap_code16(KC_ENTER);
        // wait_ms(100);
        SEND_STRING("git add . && git commit -m ");
        tap_code16(KC_DQUO);
        tap_code16(KC_DQUO);
        tap_code16(KC_LEFT);
        LeadExec = true;
    }
    // APPLICATIONS
    else if (leader_sequence_two_keys(KC_A, KC_T)) {
        mem_keycode_Leader[0] = KC_A;
        mem_keycode_Leader[1] = KC_T;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RALT(RCTL(RSFT(RGUI(KC_T)))));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RALT(RCTL(RSFT(KC_T))));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_A, KC_C)) { // vs  code
        mem_keycode_Leader[0] = KC_A;
        mem_keycode_Leader[1] = KC_C;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RALT(RCTL(RSFT(RGUI(KC_V)))));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RALT(RCTL(RSFT(KC_V))));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_A, KC_B)) { // web browser
        mem_keycode_Leader[0] = KC_A;
        mem_keycode_Leader[1] = KC_B;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RALT(RCTL(RSFT(RGUI(KC_S)))));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RALT(RCTL(RSFT(KC_S))));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_G, KC_S)) { // quick google search
        mem_keycode_Leader[0] = KC_G;
        mem_keycode_Leader[1] = KC_S;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RALT(RCTL(RSFT(RGUI(KC_G)))));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_C, KC_P)) { // code cmd pallete
        mem_keycode_Leader[0] = KC_C;
        mem_keycode_Leader[1] = KC_P;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RSFT(RGUI(KC_P)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RSFT(RCTL(KC_P)));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_C, KC_D)) { // code copy line down
        mem_keycode_Leader[0] = KC_C;
        mem_keycode_Leader[1] = KC_D;
        countLeader           = 2;
        tap_code16(RSFT(RALT(KC_DOWN)));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_D, KC_L)) { // code delete line
        mem_keycode_Leader[0] = KC_D;
        mem_keycode_Leader[1] = KC_L;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RSFT(RGUI(KC_K)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RSFT(RCTL(KC_K)));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_M, KC_B)) { // code match brackets
        mem_keycode_Leader[0] = KC_M;
        mem_keycode_Leader[1] = KC_B;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RSFT(RGUI(KC_BSLS)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RSFT(RCTL(KC_BSLS)));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_C, KC_B)) { // put cursor below
        mem_keycode_Leader[0] = KC_C;
        mem_keycode_Leader[1] = KC_B;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RGUI(KC_ENTER));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RCTL(KC_ENTER));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_C, KC_A)) { // put cursor above
        mem_keycode_Leader[0] = KC_C;
        mem_keycode_Leader[1] = KC_B;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RSFT(RGUI(KC_ENTER)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RSFT(RCTL(KC_ENTER)));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_R, KC_F)) { // revel file
        mem_keycode_Leader[0] = KC_R;
        mem_keycode_Leader[1] = KC_F;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RGUI(KC_K));
            tap_code16(KC_R);
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RCTL(KC_K));
            tap_code16(KC_R);
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_L, KC_U)) { // move line up
        mem_keycode_Leader[0] = KC_L;
        mem_keycode_Leader[1] = KC_U;
        countLeader           = 2;
        tap_code16(RALT(KC_UP));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_L, KC_D)) { // move line down
        mem_keycode_Leader[0] = KC_L;
        mem_keycode_Leader[1] = KC_D;
        countLeader           = 2;
        tap_code16(RALT(KC_DOWN));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_T, KC_W)) { // toggle word wrap
        mem_keycode_Leader[0] = KC_T;
        mem_keycode_Leader[1] = KC_W;
        countLeader           = 2;
        tap_code16(RALT(KC_Z));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_B, KC_C)) { // block comment
        mem_keycode_Leader[0] = KC_B;
        mem_keycode_Leader[1] = KC_C;
        countLeader           = 2;
        tap_code16(RSFT(RALT(KC_A)));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_I, KC_S)) { // insert snippet
        mem_keycode_Leader[0] = KC_I;
        mem_keycode_Leader[1] = KC_S;
        countLeader           = 2;
        tap_code16(RALT(KC_J));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_N, KC_J)) { // new jupyter notebook
        mem_keycode_Leader[0] = KC_N;
        mem_keycode_Leader[1] = KC_J;
        countLeader           = 2;
        tap_code16(RALT(RCTL(RGUI(KC_J))));
        LeadExec = true;
    }

    else if (leader_sequence_two_keys(KC_T, KC_F)) { // terminal focus
        mem_keycode_Leader[0] = KC_T;
        mem_keycode_Leader[1] = KC_F;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RGUI(KC_T));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RCTL(KC_T));
            LeadExec = true;
        }
    }

    else if (leader_sequence_two_keys(KC_E, KC_F)) { // editor focus
        mem_keycode_Leader[0] = KC_E;
        mem_keycode_Leader[1] = KC_F;
        countLeader           = 2;
        if (onMac) {
            tap_code16(RGUI(KC_E));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RCTL(KC_E));
            LeadExec = true;
        }
    }

    //// THREE

    else if (leader_sequence_three_keys(KC_N, KC_T, KC_P)) { // new terminal in path
        mem_keycode_Leader[0] = KC_N;
        mem_keycode_Leader[1] = KC_T;
        mem_keycode_Leader[2] = KC_P;
        countLeader           = 3;
        if (onMac) {
            tap_code16(RSFT(RGUI(KC_T)));
            LeadExec = true;
        } else if (onWin) {
            tap_code16(RSFT(RCTL(KC_T)));
            LeadExec = true;
        }
    }

    //// FOUR

    //// FIVE

    if (LeadExec) {
        leader_time = timer_read32();
    }
}

void matrix_scan_user(void) {
    if (shft_trigger) {
        if ((!shft_held_once) & (timer_elapsed32(shft_timer) > ModQuickHoldTime)) { // do these comp only if shft is pressed
            register_code(KC_RSFT);
            shft_held_once = true;
        }
    }

    if (ctrl_trigger) {
        if ((!ctrl_held_once) & (timer_elapsed32(ctrl_timer) > ModQuickHoldTime)) {
            register_code(KC_RCTL);
            ctrl_held_once = true;
        }
    }

    if (alt_trigger) {
        if ((!alt_held_once) & (timer_elapsed32(alt_timer) > ModQuickHoldTime)) {
            register_code(KC_RALT);
            alt_held_once = true;
        }
    }

    if (gui_trigger) {
        if ((!gui_held_once) & (timer_elapsed32(gui_timer) > ModQuickHoldTime)) {
            register_code(KC_RGUI);
            gui_held_once = true;
        }
    }

    if (LeaderRepPressed) {
        if ((timer_elapsed32(leader_timer) > 230)) {
            leader_start();
            for (int i = 0; i < countLeader; i++) {
                leader_sequence_add(mem_keycode_Leader[i]);
            }
            leader_end();
            leader_timer = timer_read32(); // first press will be quick-> from 0 or last leader repeat will >290. this will affect repeat only
        }
    }

    if (Mem2RepPressed) {
        if ((timer_elapsed32(mem2_timer) > 210)) {
            for (int i = 0; i < count2; i++) { // impelement set_oneshot_mods(MOD_BIT(KC_RCTL)) for mods
                register_mods(RecModsStore2[i]);
                register_code16(RecStore2[i]);
                wait_ms(5);
                unregister_code16(RecStore2[i]);
                unregister_mods(RecModsStore2[i]);
                wait_ms(10);
            }
            mem2_timer = timer_read32(); // first press will be quick-> from 0 or last  repeat will >210. this will affect repeat only
        }
    }
}

void MemkeywithMod(uint16_t kcode) {
    mem_keycode = kcode;
    // if (get_mods()){
    //     saved_mods = get_mods();
    // } else if (get_oneshot_mods()){
    //     saved_mods = get_oneshot_mods();
    // } else {
    //     saved_mods = 0;
    // }

    saved_mods = get_mods() | get_oneshot_mods();

    if (recording1) {
        RecModsStore1[count1] = saved_mods;
        RecStore1[count1]     = kcode;
        count1++;
    }
}