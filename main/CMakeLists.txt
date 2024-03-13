# idf_component_register(SRCS "sd_card_player.c" "main.c"
# "menu.c" "menu.h"
# "i2c_display.c" "i2c_display.h"
# "lib/buttonreader.c" "lib/buttonreader.h"
# "internet_radio.c" "internet_radio.h"
#                     INCLUDE_DIRS ".")

set(COMPONENT_SRCS "main.c"
"menu.c" "menu.h"
"i2c_display.c" "i2c_display.h"
"lib/buttonreader.c" "lib/buttonreader.h"
"internet_radio.c" "internet_radio.h")
set(COMPONENT_ADD_INCLUDEDIRS .)

register_component()
