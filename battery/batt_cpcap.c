#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/* spi/cpcap.h is missing in gingerbread bionic */
//#include <linux/spi/cpcap.h>
#include "cpcap.h"

#include "common.h"
#include "batt_cpcap.h"

static int cpcap_fd = -1;
static struct cpcap_adc_us_request req_us;

/**
 * bank   int : enum cpcap_adc_type
 *
 *        CPCAP_ADC_TYPE_BANK_0
 *        CPCAP_ADC_TYPE_BANK_1
 *        CPCAP_ADC_TYPE_BATT_PI
 *
 * format int : enum cpcap_adc_format
 *
 *        CPCAP_ADC_FORMAT_RAW
 *        CPCAP_ADC_FORMAT_PHASED
 *        CPCAP_ADC_FORMAT_CONVERTED
 */
static int read_bank(int bank, int format) {

    int ret;

    memset(&req_us, 0, sizeof(struct cpcap_adc_us_request));

    req_us.timing = CPCAP_ADC_TIMING_IMM;
    req_us.format = format;
    req_us.type   = bank;

    ret = ioctl(cpcap_fd, CPCAP_IOCTL_BATT_ATOD_SYNC, &req_us);
    if (ret != 0) {
        LOGE("ioctl failed, ret=%d errno=%d", ret, errno);
    }
    return ret;
}


/**
 * Read battery voltage, and convert to percentage
 * Its not really the exact value, but give an idea on battery level.
 *
 */
int cpcap_batt_percent(void) {

    int ret, percent, volt_batt;
    float range, mini;

    cpcap_fd = open("/dev/cpcap_batt", O_RDONLY | O_NONBLOCK);
    if (cpcap_fd <= 0) {
        return -1;
    }

    ret = read_bank(CPCAP_ADC_TYPE_BANK_0, CPCAP_ADC_FORMAT_CONVERTED);
    if (ret != 0) {
        percent = -1;
        goto exit;
    }

    volt_batt = req_us.result[CPCAP_ADC_BATTP];
/*
    int volt_vbus = req_us.result[CPCAP_ADC_VBUS];

    if (volt_vbus > 4400 && volt_vbus < 5200) {
        // usb powered
        volt_batt -= 100;
    }
*/

    // Minimal voltage value, max ~= 4100
    // under 3.5V, battery is almost empty
    mini  = 3500.0;
    range = (4050.0 - mini) / 100.0;

    percent = (volt_batt - mini) / range;
    if (percent > 100) percent = 100;
    if (percent < 0)   percent = 0;

exit:
    close(cpcap_fd);

    return percent;
}
