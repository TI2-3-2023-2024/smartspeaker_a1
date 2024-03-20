# idf_component_register(SRCS "main.c"
# "menu.c" "menu.h"
# "i2c_display.c" "i2c_display.h"
# "lib/buttonreader.c" "lib/buttonreader.h"
# "internet_radio.c" "internet_radio.h"
#                     INCLUDE_DIRS ".")

set(COMPONENT_SRCS "main.c"
"menu.c" "menu.h"
"i2c_display.c" "i2c_display.h"
"lib/buttonreader.c" "lib/buttonreader.h"
"lib/internet_radio.c" "lib/internet_radio.h"
"lib/ntp.c" "lib/ntp.h" "lib/wifi_setup.c" "lib/wifi_setup.h"
"lib/sd_card_player.c" "lib/sd_card_player.h"
"goertzel_filter.c" "goertzel_filter.h"
"lib/goertzel.c" "lib/goertzel.h")
set(COMPONENT_ADD_INCLUDEDIRS .)

register_component()
