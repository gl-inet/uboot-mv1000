#define GPIO_VPN_LED 11
#define GPIO_WIFI_LED 12
#define GPIO_POWER_LED 13

#define GPIO_RESET_BTN 14

#define GL_RESET_BUTTON_IS_PRESS        0

#define LED_ON 0
#define LED_OFF 1

int power_led_on(void);

int mv_gpio_request(void);
int mv_gpio_free(void);
void led_toggle(unsigned int gpio);
void check_button_is_press(void);

#define WEBFAILSAFE_PROGRESS_START		    0
#define WEBFAILSAFE_PROGRESS_TIMEOUT		1
#define WEBFAILSAFE_PROGRESS_UPLOAD_READY	2
#define WEBFAILSAFE_PROGRESS_UPGRADE_READY	3
#define WEBFAILSAFE_PROGRESS_UPGRADE_FAILED	4

#define WEBFAILSAFE_UPGRADE_TYPE_FIRMWARE	0
#define WEBFAILSAFE_UPGRADE_TYPE_UBOOT		1
#define WEBFAILSAFE_UPGRADE_TYPE_ART		2

#define CONFIG_LOADADDR 0x5000000
#define WEBFAILSAFE_UPLOAD_RAM_ADDRESS                  0x5000000

#define WEBFAILSAFE_UPLOAD_UBOOT_ADDRESS                0
#define WEBFAILSAFE_UPLOAD_FW_ADDRESS                   0x100000
#define WEBFAILSAFE_UPLOAD_ART_ADDRESS                  0xf8000

#define CONFIG_UBOOT_START                0
#define CONFIG_FW_START                   0x100000
#define CONFIG_ART_START                  0xf8000

#define WEBFAILSAFE_UPLOAD_UBOOT_SIZE_IN_BYTES          (960*1024)
#define WEBFAILSAFE_UPLOAD_ART_SIZE_IN_BYTES            (32*1024)

#define FW_TYPE_NOR 0
#define FW_TYPE_EMMC 1

int check_test(void);
int check_config(void);
int auto_update_by_tftp(void);
int check_fw_type(void *address);
