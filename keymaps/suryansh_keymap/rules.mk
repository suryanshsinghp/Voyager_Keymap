# Set any rules.mk overrides for your specific keymap here.
# See rules at https://docs.qmk.fm/#/config_options?id=the-rulesmk-file
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
AUTO_SHIFT_ENABLE = yes
ORYX_ENABLE = yes
RGB_MATRIX_CUSTOM_KB = yes
TAP_DANCE_ENABLE = yes
SPACE_CADET_ENABLE = no
CAPS_WORD_ENABLE = yes
COMBO_ENABLE = yes
LEADER_ENABLE = yes
OS_DETECTION_ENABLE = yes
SWAP_HANDS_ENABLE=yes
SRC += features/layer_lock.c

