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
"lib/connection_animation.c" "lib/connection_animation.h"
"lib/lcd_setup.c "lib/lcd_setup.h)
set(COMPONENT_ADD_INCLUDEDIRS .)

register_component()
