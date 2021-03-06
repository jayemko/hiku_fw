
/*
 *  Copyright (C) 2015, Marvell International Ltd.
 *  All Rights Reserved.
 */

/**
 *  @brief This is an  application for FCC Certification
 */

#include <stdlib.h>
#include "string.h"
#include "wmstdio.h"
#include "wm_os.h"
#include "mdev_sdio.h"
#include <lowlevel_drivers.h>

#include "cli.h"
#include "cli_utils.h"
#include "wlan.h"
#include "wmstdio.h"
#include "partition.h"
#include "flash.h"
#include "fcc_cmds.h"
#include <wifi.h>

#ifdef APPCONFIG_BT_SUPPORT
#include <bt.h>
#endif

#define MLAN_TYPE_CMD   1

#define GET			0
#define SET			1
#define COMPARE(x, y)		strcasecmp(x, y)

#define CHANNEL			"channel"
#define ANTENNA			"antenna"
#define TX_CW_MODE		"tx-cw-mode"
#define TX_CONT_MODE		"tx-cont-mode"
#define RF_POWER		"rf-power"
#define CHANNEL_BW		"channel-bw"
#define RF_BAND		        "rf-band"
#define RX_PKT_CNT		"rx-pkt-cnt"
#define BSS_ID_FILTER		"bss-id-filter"

#define B_MODE		0
#define G_MODE		1


extern int sd_wifi_init(enum wlan_type type,
			enum wlan_fw_storage_type st,
			flash_desc_t *fl, uint8_t *fw_ram_start_addr);

#ifdef APPCONFIG_BT_SUPPORT
#define LE_TX_PKT_CNT           "le-tx-pkt-cnt"
#define LE_TX_TEST              "le-tx-test"
#define LE_RX_TEST		"le-rx-test"
#define LE_TX_POWER		"le-tx-power"
#define LE_RX_TEST_PKT_ERROR	"le-rx-test-pkt-err"
#define LE_RESP_VALUE		10
#define LE_RESP_RESULT		9
#define LE_RESP_COR		12
#define LE_RESP_OPCODE		7
#define LE_REQ_FREQ_INDEX	7
#define LE_REQ_LEN		8
#define LE_REQ_PATTERN		9
#define LE_REQ_POWER		7
/* le tx event queue message */
typedef struct {
	int result;
	uint16_t value;
	uint16_t cor;
} le_pkt_resp_t;

static os_queue_t le_tx_event_queue;
static os_queue_pool_define(le_tx_event_queue_data,
			3 * sizeof(le_pkt_resp_t));

static void bt_pkt_recv(uint8_t pkt_type, uint8_t *data, uint32_t size)
{
	int ret;
	le_pkt_resp_t msg;
	if (data[LE_RESP_OPCODE] == 0x84 && data[LE_RESP_OPCODE + 1] == 0xfc) {
		memcpy(&msg.value, &data[LE_RESP_VALUE], 2);
		memcpy(&msg.cor, &data[LE_RESP_COR], 2);
	} else
		memcpy(&msg.value, &data[LE_RESP_VALUE], 2);
	msg.result = data[LE_RESP_RESULT];
	ret = os_queue_send(&le_tx_event_queue, &msg, OS_NO_WAIT);
	if (ret != WM_SUCCESS) {
		wmprintf("Failed to send event on queue");
	}
}

static void fcc_le_tx_test_usage()
{
	wmprintf("\n To start Tx Test:\r\n\n");
	wmprintf("le-tx-test start <FreqIndex = 0> <Len = 0> ");
	wmprintf("<Pattern = 0>\r\n");
	wmprintf("Range of FreqIndex is 0 - 39 \r\n");
	wmprintf("Range of Len is 0 - 37 \r\n");
	wmprintf("Range of pattern is 0 - 7 \r\n");
	wmprintf("\n To stop Tx Test:\r\n\n");
	wmprintf("le-tx-test stop\r\n");
}

static void fcc_le_rx_test_usage()
{
	wmprintf("\n To start Rx Test:\r\n\n");
	wmprintf("le-rx-test start <FreqIndex = 0>\r\n");
	wmprintf("Range of FreqIndex is 0 - 39 \r\n");
	wmprintf("\n To stop Rx Test:\r\n\n");
	wmprintf("le-rx-test stop\r\n");
}

static void fcc_le_tx_power_usage()
{
	wmprintf("le-tx-power <pwr = 0>\r\n");
	wmprintf("Range of power is -30 to 20 dBm \r\n");
}

static void fcc_le_rx_pkt_error_command_usage()
{
	wmprintf("\nTo get the rx packet error:\r\n\n");
	wmprintf("le-rx-test-pkt-err get\r\n");
}

static void fcc_le_tx_pkt_count_command_usage()
{
	wmprintf("\nTo get the tx packet count:\r\n\n");
	wmprintf("le-tx-pkt-cnt get\r\n");
}

#endif

static void fcc_bss_id_command_usage()
{
	wmprintf("\nTo disable bss id filter:\r\n\n");
	wmprintf("bss-id-filter 0\r\n\n");
	wmprintf("\nTo enable bss id filter:\r\n\n");
	wmprintf("Example: If bss_id is: aa:bb:cc:11:22:33\r\n\n");
	wmprintf("bss-id-filter 1 0xaa 0xbb 0xcc 0x11 0x22 0x33\r\n");

}

static void fcc_rx_pkt_command_usage()
{
	wmprintf("\nTo get the rx packet count:\r\n\n");
	wmprintf("rx-pkt-cnt get\r\n");
}


static void fcc_rf_band_command_usage()
{
	wmprintf("\nTo get/set rf band:\r\n\n");
	wmprintf("rf-band get\r\n\n");
	wmprintf("rf-band set <rf-band_value>\r\n");
	wmprintf("	where rf-band_value = 0 : 2.4GHz\r\n"
		"             rf-band_value = 1 : 5GHz\r\n\n");
}

static void fcc_channel_command_usage()
{
	wmprintf("\nTo get/set rf channel:\r\n\n");
	wmprintf("channel get\r\n\n");
	wmprintf("channel set <channel_value>\r\n");
}

static void fcc_antenna_command_usage()
{
	wmprintf("\nTo get/set tx antenna:\r\n\n");
	wmprintf("antenna get\r\n\n");
	wmprintf("antenna set <antenna_value>\r\n");
	wmprintf("	where antenna_value = 0 or 1\r\n\n");
}

static void fcc_tx_cw_command_usage()
{
	wmprintf("\nTo enable/disable tx-cw mode:\r\n\n");
	wmprintf("tx-cw-mode 1 : enable tx-cw mode\r\n\n");
	wmprintf("tx-cw-mode 0 : disable tx-cw mode\r\n\n");
}

static void fcc_tx_cont_command_usage()
{
	wmprintf("\nTo enable/disable tx-cont mode:\r\n\n");
	wmprintf("tx-cont-mode 1 <data_rate_index> : enable tx-cont mode"
		" with specified data rate\r\n");
	wmprintf("Data rate index:\r\n");
	wmprintf("   1 for 1M; 2 for 2M; 3 for 5.5M; 4 for 11M;"
		" 5 for 22M; 6 for 6M; 7 for 9M;\r\n");
	wmprintf("   8 for 12M; 9 for 18M; 10 for 24M; 11 for 36M;"
		" 12 for 48M; 13 for 54M; 14 for 72M;\r\n");
	wmprintf("   15 for MCS0; 16 for MCS1; 17 for MCS2; 18 for MCS3;"
		" 19 for MCS4; 20 for MCS5; 21 for MCS6; 22 for MCS7\r\n");
	wmprintf("   23 for MCS8; 24 for MCS9; 25 for MCS10; 26 for MCS11;"
		" 27 for MCS12; 28 for MCS13; 29 for MCS14; 30 for MCS15\r\n");
	wmprintf("Note: For 1x1 devices, data rates upto index 22 are "
		"valid. For 2x2 devices data rates upto index "
		"30 are valid");
	wmprintf("tx-cont-mode 0 : disable tx-cont mode\r\n\n");
}

static void fcc_rf_power_command_usage()
{
	wmprintf("\nTo set power at antenna using calibration data:\r\n\n");
	wmprintf("rf-power <channel_value> <power_level> <modulation = 0>\r\n");
	wmprintf("	      modulation = 0 (B mode) or 1 (G mode).");
	wmprintf(" Default value of modulation is 0.\r\n\n");
}

static void fcc_channel_bw_command_usage()
{
	wmprintf("\nTo set channel bandwidth:\r\n\n");
	wmprintf("channel-bw 0 : Set channel bandwidth to 20MHz\r\n");
	wmprintf("channel-bw 1 : Set channel bandwidth to 40MHz\r\n");
}

int wifi_raw_packet_recv(uint8_t **data, uint32_t *pkt_type);
uint8_t fcc_get_response(uint8_t *param)
{
	uint32_t pkt_type;
	uint8_t *packet;
	int rv = wifi_raw_packet_recv(&packet, &pkt_type);
	if (rv != WM_SUCCESS) {
		wmprintf("Error in getting the packet\r\n");
		return -WM_FAIL;
	}

	if (pkt_type == MLAN_TYPE_CMD) {
		if (param) {
			if (packet[12] == 0x12 && packet[13] == 0x90) {
				memcpy((struct rx_pkt *)param,
					&packet[24], 12);
			} else {
				*param = packet[DATA_INDEX];
			}
		}
		return packet[RESULT_INDEX];
	}

	wmprintf("Error in getting the command response\r\n");
	return -WM_FAIL;
}

void wifi_raw_packet_send(const uint8_t *packet, uint32_t length);
uint8_t fcc_send_cmd_get_resp(char *data, uint8_t *param)
{
	wifi_raw_packet_send((const uint8_t *)data, 256);
	return fcc_get_response(param);
}

bool fcc_get_type(char *argv)
{
	if (!COMPARE(argv, "get"))
		return GET;
	if (!COMPARE(argv, "set"))
		return SET;
	return -1;
}

uint16_t fcc_2char_to_short(uint8_t *response, uint8_t index)
{
	uint16_t val;
	val = response[index + 1] & 0xff;
	val = (val << 8) | response[index];
	return val;
}


/* Parameter specific functions */

/** Get/Set RF channel
 *
 *  This function is used to get/set RF channel
 *
 *  \param
 *	getset	0 = get
 *		1 = set
 *	channel_value channel number you want to set.
 *	*new_channel pointer to the variable in which you want to get the
 *			value of currently/newly configured channel.
 *
 *  \return result of the command from the firmware. 0 if successful,
 *		non zero otherwise.
 *
 */

static int fcc_get_set_channel(bool getset, int16_t channel_value,
				uint8_t *new_channel)
{
	CHANNEL_RAW_DATA[TYPE_INDEX] = getset;
	CHANNEL_RAW_DATA[DATA_INDEX] = channel_value;
	return fcc_send_cmd_get_resp(CHANNEL_RAW_DATA, new_channel);
}

/** Get/Set Antenna
 *
 *  This function is used to get/set antenna
 *
 *  \param
 *      getset  0 = get
 *              1 = set
 *      antenna_value antenna you want to set. (0 or 1)
 *      *new_antenna pointer to the variable in which you want to get the
 *                      value of currently/newly configured antenna.
 *
 *  \return result of the command from the firmware. 0 if successful,
 *              non zero otherwise.
 *
 */

static int fcc_get_set_antenna(bool getset, int8_t antenna_value,
				uint8_t *new_antenna)
{
	ANTENNA_RAW_DATA[TYPE_INDEX] = getset;
	ANTENNA_RAW_DATA[DATA_INDEX] = antenna_value;
	return fcc_send_cmd_get_resp(ANTENNA_RAW_DATA, new_antenna);
}

/** Enable/Disable TX CW mode
 *
 *  This function is used to enable/disable TX CW mode
 *
 *  \param
 *      value   0 = disable
 *              1 = enable
 *
 *  \return result of the command response from the firmware. 0 if successful,
 *              non zero otherwise.
 *
 */
static int fcc_enable_disable_tx_cw_mode(bool value)
{
	TX_MODE_CW_RAW_DATA[DATA_INDEX] = value;
	return fcc_send_cmd_get_resp(TX_MODE_CW_RAW_DATA, NULL);
}

/** Set the datarate
 *
 *  This function is used to set the data rate
 *
 *  \param
 *      datarate_index required index of the datarate array
 *
 *  \return result of the command response from the firmware. 0 if successful,
 *              non zero otherwise.
 *
 */
static int fcc_set_datarate(uint8_t datarate_index)
{
	TX_RATE_RAW_DATA[DATA_INDEX] = datarate_index - 1;
	return fcc_send_cmd_get_resp(TX_RATE_RAW_DATA, NULL);
}

/** Enable/Disable TX cont mode
 *
 *  This function is used to enable/disable TX cont mode
 *
 *  \param
 *      value   0 = disable
 *              1 = enable
 *
 *  \return result of the command response from the firmware. 0 if successful,
 *              non zero otherwise.
 *
 */
static int fcc_enable_disable_tx_cont_mode(bool value)
{
	TX_MODE_CONT_RAW_DATA[DATA_INDEX] = value;
	return fcc_send_cmd_get_resp(TX_MODE_CONT_RAW_DATA, NULL);
}

/** Set the rf power
 *
 *  This function is used to set the rf power
 *
 *  \param
 *      power_level power level you want to set.
 *	modulation_index 0 (b mode)
 *			 1 (g mode)
 *
 *  \return result of the command response from the firmware. 0 if successful,
 *              non zero otherwise.
 *
 */
static int fcc_set_rf_power(int8_t power_level, int8_t modulation_index)
{
	RF_POWER_RAW_DATA[DATA_INDEX] = power_level;
	RF_POWER_RAW_DATA[MODULATION_INDEX] = modulation_index;
	return fcc_send_cmd_get_resp(RF_POWER_RAW_DATA, NULL);
}

/** Set the channel bandwidth
 *
 *  This function is used to set the channel bandwidth
 *
 *  \param
 *	value 0 (20MHz)
 *	      1 (40MHz)
 *
 *  \return result of the command response from the firmware. 0 if successful,
 *              non zero otherwise.
 *
 */
static int fcc_set_channel_bw(bool value)
{
	CHANNEL_BW_RAW_DATA[DATA_INDEX] = value;
	return fcc_send_cmd_get_resp(CHANNEL_BW_RAW_DATA, NULL);
}

/** Get/Set RF band
 *
 *  This function is used to get/set RF band
 *
 *  \param
 *	getset	0 = get
 *		1 = set
 *	rfband  0 = 2.4GHz
 *	rfband  1 = 5GHz
 *	*new_rfband pointer to the variable in which you want to get the
 *			value of currently/newly configured rf band.
 *
 *  \return result of the command from the firmware. 0 if successful,
 *		non zero otherwise.
 *
 */

static int fcc_get_set_rf_band(bool getset, int8_t rfband,
				uint8_t *new_rfband)
{
	RF_BAND_RAW_DATA[TYPE_INDEX] = getset;
	RF_BAND_RAW_DATA[DATA_INDEX] = rfband;
	return fcc_send_cmd_get_resp(RF_BAND_RAW_DATA, new_rfband);
}

/** Enable/Disable BSSID Filter
 *
 *  This function is used to enable/disable BSSID Filter
 *
 *  \param
 *	bss_id A pointer to an array containing BSSID
 *
 *  \return result of the command from the firmware. 0 if successful,
 *		non zero otherwise.
 *
 */

static int fcc_disable_enable_bss_id_filter(uint8_t *bss_id)
{
	memcpy(&BSS_ID_FILTER_RAW_DATA[60], bss_id, 6);
	return fcc_send_cmd_get_resp(BSS_ID_FILTER_RAW_DATA, NULL);
}

/** Get Rx Packet Count
 *
 *  This function is used to get Rx packet count
 *
 *  \param
 *	pkt Packet of type rx_pkt in which you want the rx packet information
 *
 *  \return result of the command from the firmware. 0 if successful,
 *		non zero otherwise.
 *
 */

static int fcc_get_rx_pkt_cnt(uint8_t *pkt)
{
	return fcc_send_cmd_get_resp(GET_RX_PKT_CNT_RAW_DATA, pkt);
}

#ifdef APPCONFIG_BT_SUPPORT

void bt_unmask_sdio_interrupts();
int32_t bt_raw_pkt_send(uint8_t *data);

static int fcc_le_send_cmd_get_resp(char *data, char *param)
{
	int ret = 0;
	bt_unmask_sdio_interrupts();

	bt_raw_pkt_send((uint8_t *)data);
	ret = os_queue_recv(&le_tx_event_queue, param,
			OS_WAIT_FOREVER);
	if (ret != WM_SUCCESS) {
		wmprintf("tx_cnt: failed to get message "
				"from queue [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

/** Le Tx packet count
 *
 *  This function is used to get Tx packet count
 *
 *  \return result of the command from the firmware. 0 if successful,
 *              non zero otherwise
 */

static int fcc_le_tx_pkt_cnt()
{
	int ret = 0;
	le_pkt_resp_t msg;
	ret = fcc_le_send_cmd_get_resp(LE_TX_PKT_CNT_RAW_DATA, (char *)&msg);
	wmprintf("Result: %d\r\n", msg.result);
	wmprintf("Tx test Packet count: %d", msg.value);
	if (ret != WM_SUCCESS) {
		wmprintf("failed to get response : [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

/** Le Tx test start
 *
 *  This function starts the transmission of packets
 *
 *  \param
 *	freq_index = 0 -39
 *	len = 0 - 37
 *	pattern = 0 - 7
 *
 *  \return result of the command from the firmware. 0 if successful,
 *              non zero otherwise
 */

static int fcc_le_tx_test_start(int freq_index, int len, int pattern)
{
	int ret = 0;
	le_pkt_resp_t msg;
	if (freq_index != -1)
		LE_TX_TEST_START_RAW_DATA[LE_REQ_FREQ_INDEX] = freq_index;
	if (len != -1)
		LE_TX_TEST_START_RAW_DATA[LE_REQ_LEN] = len;
	if (pattern != -1)
		LE_TX_TEST_START_RAW_DATA[LE_REQ_PATTERN] = pattern;
	ret = fcc_le_send_cmd_get_resp(LE_TX_TEST_START_RAW_DATA, (char *)&msg);
	wmprintf("Result: %d\r\n", msg.result);
	if (ret != WM_SUCCESS) {
		wmprintf("failed to get response : [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

/** Le test stop
 *
 *  This function stops the transmission & receival of packets.
 *
 *  \return result of the command from the firmware. 0 if successful,
 *			non zero otherwise
 */

static int fcc_le_test_stop()
{
	int ret = 0;
	le_pkt_resp_t msg;
	ret = fcc_le_send_cmd_get_resp(LE_TEST_STOP_RAW_DATA, (char *)&msg);
	wmprintf("Result: %d\r\n", msg.result);
	if (ret != WM_SUCCESS) {
		wmprintf("failed to get response : [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

/** Le Rx test start
 *
 *  This function starts the receival of packets from the device
 *
 *  \param
 *	freq_index = 0 - 39
 *
 *  \return result of the command from the firmware. 0 if successful,
 *              non zero otherwise
 */

static int fcc_le_rx_test(int freq_index)
{
	int ret = 0;
	le_pkt_resp_t msg;
	if (freq_index != -1)
		LE_RX_TEST_START_RAW_DATA[LE_REQ_FREQ_INDEX] = freq_index;
	ret = fcc_le_send_cmd_get_resp(LE_RX_TEST_START_RAW_DATA, (char *)&msg);
	wmprintf("Result: %d\r\n", msg.result);
	if (ret != WM_SUCCESS) {
		wmprintf("failed to get response : [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

/** Le Tx power
 *
 *  This function writes the power to be used by the device
 *
 *  \param
 *	power = -30 to 20 dBm
 *
 *  \return result of the command from the firmware. 0 if successful,
 *              non zero otherwise
 */

static int fcc_le_tx_power(int power)
{
	int ret = 0;
	le_pkt_resp_t msg;
	if (power != -31)
		LE_TX_POWER_RAW_DATA[LE_REQ_POWER] = power;
	ret = fcc_le_send_cmd_get_resp(LE_TX_POWER_RAW_DATA, (char *)&msg);
	wmprintf("Result: %d\r\n", msg.result);
	if (ret != WM_SUCCESS) {
		wmprintf("failed to get response : [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

/** Le Rx packet error
 *
 *  This function is used to check for errors while packet is received.
 *
 *  \return result of the command from the firmware. 0 if successful,
 *              non zero otherwise
 */

static int fcc_le_rx_pkt_err()
{
	int ret = 0;
	le_pkt_resp_t msg;
	ret = fcc_le_send_cmd_get_resp(LE_RX_PKT_ERR_RAW_DATA, (char *)&msg);
	wmprintf("Result: %d\r\n", msg.result);
	wmprintf("Rx test: No-Crc-Err: %d \t", msg.value);
	wmprintf("No-correlation-Err : %d\r\n", msg.cor);
	if (ret != WM_SUCCESS) {
		wmprintf("failed to get response : [%d]", ret);
		return -WM_FAIL;
	}
	return WM_SUCCESS;
}

#endif

void fcc_process_cmd(int argc, char **argv)
{
	uint8_t retval;
	int8_t param = 0, param1 = 0, param2 = 0;
	int16_t param_c = 0;
	int getset;
#ifdef APPCONFIG_BT_SUPPORT
	int freq_index = -1, len = -1, pattern = -1;
	int8_t power = -31;
#endif
	getset = fcc_get_type(argv[1]);

	if (!COMPARE(argv[0], CHANNEL)) {
		switch (getset) {
		case GET:
			if (argc != 2) {
				wmprintf("\nUsage:\r\n");
				fcc_channel_command_usage();
				return;
			}
			break;
		case SET:
			if (argc != 3) {
				wmprintf("\nUsage:\r\n");
				fcc_channel_command_usage();
				return;
			}
			param_c = -1;
			sscanf(argv[2], "%hi", &param_c);
			break;
		default:
			wmprintf("\nUsage:\r\n");
			fcc_channel_command_usage();
			return;
		}

		if (param_c <= -1) {
			wmprintf("\nUsage:\r\n");
			fcc_channel_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
			fcc_get_set_channel(getset, param_c, &retval));
		wmprintf("Channel: %d\r\n", retval);
	}

	if (!COMPARE(argv[0], ANTENNA)) {
		switch (getset) {
		case GET:
			if (argc != 2) {
				wmprintf("\nUsage:\r\n");
				fcc_antenna_command_usage();
				return;
			}
			break;
		case SET:
			if (argc != 3) {
				wmprintf("\nUsage:\r\n");
				fcc_antenna_command_usage();
				return;
			}
			param = -1;
			sscanf(argv[2], "%c", &param);
			break;
		default:
			wmprintf("\nUsage:\r\n");
			fcc_antenna_command_usage();
			return;
		}
		if (param > 1 || param <= -1) {
			wmprintf("\nUsage:\r\n");
			fcc_antenna_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
			fcc_get_set_antenna(getset, param, &retval));
		wmprintf("Antenna: %d\r\n", retval);
	}

	if (!COMPARE(argv[0], TX_CW_MODE)) {
		if (argc != 2) {
			wmprintf("\nUsage:\r\n");
			fcc_tx_cw_command_usage();
			return;
		}
		param = -1;
		sscanf(argv[1], "%c", &param);
		if (param != GET && param != SET) {
			wmprintf("\nUsage:\r\n");
			fcc_tx_cw_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
			fcc_enable_disable_tx_cw_mode(param));
	}

	if (!COMPARE(argv[0], TX_CONT_MODE)) {
		param = -1;
		getset = SET;
		sscanf(argv[1], "%c", &param);
		if (param == 0 && argc == 2) {
			getset = GET;

		} else if (param == 1 && argc == 3) {
			param1 = -1;
			sscanf(argv[2], "%c", &param1);
			if (param1 <= 0 || param1 > 30) {
				wmprintf("\nUsage:\r\n");
				fcc_tx_cont_command_usage();
				return;
			}
			wmprintf("\r\nResult:  %d\r\n",
				fcc_set_datarate(param1));
		} else {
			wmprintf("\nUsage:\r\n");
			fcc_tx_cont_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
		fcc_enable_disable_tx_cont_mode(getset));
	}

	if (!COMPARE(argv[0], RF_POWER)) {
		param_c = -1;
		param1 = -1;
		param2 = -1;
		if (argc != 3 && argc != 4) {
			wmprintf("\nUsage:\r\n");
			fcc_rf_power_command_usage();
			return;
		}
		sscanf(argv[2], "%c", &param1);
		sscanf(argv[1], "%hi", &param_c);

		if (param_c <= -1) {
			wmprintf("\nUsage:\r\n");
			fcc_rf_power_command_usage();
			return;
		}
		/* Set the channel */
		wmprintf("\r\nResult:  %d\r\n",
			fcc_get_set_channel(1, param_c, &retval));
		wmprintf("Channel: %d\r\n", retval);

		if (argc == 4) {
			sscanf(argv[3], "%c", &param2);
			if (param2 != B_MODE && param2 != G_MODE) {
				wmprintf("\nUsage:\r\n");
				fcc_rf_power_command_usage();
				return;
			}
		}
		wmprintf("\r\nResult:  %d\r\n",
		fcc_set_rf_power(param1, param2));
	}

	if (!COMPARE(argv[0], CHANNEL_BW)) {
		if (argc != 2) {
			wmprintf("\nUsage:\r\n");
			fcc_channel_bw_command_usage();
			return;
		}
		param = -1;
		sscanf(argv[1], "%c", &param);
		if (param != 0 && param != 1) {
			wmprintf("\nUsage:\r\n");
			fcc_channel_bw_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
		fcc_set_channel_bw(param));
	}

	if (!COMPARE(argv[0], RF_BAND)) {
		switch (getset) {
		case GET:
			if (argc != 2) {
				wmprintf("\nUsage:\r\n");
				fcc_rf_band_command_usage();
				return;
			}
			break;
		case SET:
			if (argc != 3) {
				wmprintf("\nUsage:\r\n");
				fcc_rf_band_command_usage();
				return;
			}
			param = -1;
			sscanf(argv[2], "%c", &param);
			break;
		default:
			wmprintf("\nUsage:\r\n");
			fcc_rf_band_command_usage();
			return;
		}

		if (param != 0 && param != 1) {
			wmprintf("\nUsage:\r\n");
			fcc_rf_band_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
			fcc_get_set_rf_band(getset, param, &retval));
		wmprintf("RF Band: %d\r\n", retval);
	}

	if (!COMPARE(argv[0], RX_PKT_CNT)) {
		struct rx_pkt pkt;
		if (argc == 2 && !COMPARE(argv[1], "get")) {
			int ret = fcc_get_rx_pkt_cnt((uint8_t *)&pkt);
			wmprintf("Rx Packet: %u\r\n", pkt.rx_cnt);
			wmprintf("Multi Cast: %u\r\n", pkt.multicast_cnt);
			wmprintf("Err Count: %u\r\n", pkt.err_cnt);
			wmprintf("\r\nResult:  %d\r\n", ret);
		} else {
			wmprintf("\nUsage:\r\n");
			fcc_rx_pkt_command_usage();
		}
	}
#ifdef APPCONFIG_BT_SUPPORT
	if (!COMPARE(argv[0], LE_TX_PKT_CNT)) {
		if (argc == 2 && !COMPARE(argv[1], "get")) {
			fcc_le_tx_pkt_cnt();
		} else {
			wmprintf("\nUsage:\r\n");
			fcc_le_tx_pkt_count_command_usage();
		}

	}

	if (!COMPARE(argv[0], LE_TX_TEST)) {
		if (!COMPARE(argv[1], "start")) {
			if (argc >= 2 && argc <= 5) {
				if (argv[2] != NULL) {
					freq_index = a2hex_or_atoi(argv[2]);
					if (freq_index < 0 || freq_index > 39) {
						fcc_le_tx_test_usage();
						return;
					}
				}
				if (argv[3] != NULL) {
					len = a2hex_or_atoi(argv[3]);
					if (len < 0 || len > 37) {
						fcc_le_tx_test_usage();
						return;
					}
				}
				if (argv[4] != NULL) {
					pattern = a2hex_or_atoi(argv[4]);
					if (pattern < 0 || pattern > 7) {
						fcc_le_tx_test_usage();
						return;
					}
				}
				fcc_le_tx_test_start(freq_index, len, pattern);
			} else {
				wmprintf("\nUSAGE:\r\n");
				fcc_le_tx_test_usage();
			}
		} else if (!COMPARE(argv[1], "stop")) {
			if (argc == 2)
				fcc_le_test_stop();
			else {
				wmprintf("\nUSAGE:\r\n");
				fcc_le_tx_test_usage();
			}
		} else {
			wmprintf("\nUSAGE:\r\n");
			fcc_le_tx_test_usage();
		}
	}
	if (!COMPARE(argv[0], LE_RX_TEST)) {
		if (!COMPARE(argv[1], "start")) {
			if (argc >= 2 && argc < 4) {
				if (argv[2] != NULL) {
					freq_index = a2hex_or_atoi(argv[2]);
					if (freq_index < 0 || freq_index > 39) {
						fcc_le_rx_test_usage();
						return;
					}
				}
				fcc_le_rx_test(freq_index);
			} else {
				wmprintf("\nUSAGE:\r\n");
				fcc_le_rx_test_usage();
			}
		} else if (!COMPARE(argv[1], "stop")) {
			if (argc == 2)
				fcc_le_test_stop();
			else {
				wmprintf("\nUSAGE:\r\n");
				fcc_le_rx_test_usage();
			}
		} else {
			wmprintf("\nUSAGE:\r\n");
			fcc_le_rx_test_usage();
		}
	}
	if (!COMPARE(argv[0], LE_TX_POWER)) {
		if (argc >= 1 && argc < 3) {
			if (argv[1] != NULL) {
				power = atoi(argv[1]);
				if (power < -30 || power > 20)	{
					fcc_le_tx_power_usage();
					return;
				}
			}
			fcc_le_tx_power(power);
		} else {
			wmprintf("\nUSAGE:\r\n");
			fcc_le_tx_power_usage();
		}
	}
	if (!COMPARE(argv[0], LE_RX_TEST_PKT_ERROR)) {
		if (argc == 2 && !COMPARE(argv[1], "get")) {
			fcc_le_rx_pkt_err();
		} else {
			wmprintf("\nUsage:\r\n");
			fcc_le_rx_pkt_error_command_usage();
		}

	}
#endif
	if (!COMPARE(argv[0], BSS_ID_FILTER)) {
		uint8_t bss_id[6];
		sscanf(argv[1], "%c", &param);
		if (param == 0 && argc == 2) {
			memset(bss_id, 0xff, 6);
		} else if (param == 1 && argc == 8) {
			int i = 2;
			for (i = 2; i < 8; i++) {
				param = -1;
				bss_id[i - 2] = a2hex_or_atoi(argv[i]);
			}
		} else {
			wmprintf("\nUsage:\r\n");
			fcc_bss_id_command_usage();
			return;
		}
		wmprintf("\r\nResult:  %d\r\n",
			fcc_disable_enable_bss_id_filter(bss_id));
	}
}


static char registered_fcc;

static struct cli_command fcc_tests[] = {
	{CHANNEL, ": get/set the current rf channel", fcc_process_cmd},
	{CHANNEL_BW, ": set the channel bandwidth", fcc_process_cmd},
	{ANTENNA, ": get/set the Tx antenna", fcc_process_cmd},
	{TX_CW_MODE, ": on/off the tx cw mode", fcc_process_cmd},
	{TX_CONT_MODE, ": on/off the tx continuous mode with datarate",
					fcc_process_cmd},
	{RF_POWER, ": set rf power at antenna using cal data",
					fcc_process_cmd},
	{RF_BAND, ": get/set RF Band", fcc_process_cmd},
	{RX_PKT_CNT, ": clear/get received packet count", fcc_process_cmd},
	{BSS_ID_FILTER, ": on/off the bss id filter", fcc_process_cmd},

#ifdef APPCONFIG_BT_SUPPORT
	{LE_TX_PKT_CNT, ": Get Le Tx Test packet Count", fcc_process_cmd},
	{LE_TX_TEST, ": Le Tx Test", fcc_process_cmd},
	{LE_RX_TEST, ": Le Rx Test", fcc_process_cmd},
	{LE_TX_POWER, ": Le Tx Power", fcc_process_cmd},
	{LE_RX_TEST_PKT_ERROR, ": Le Rx Test packet Error", fcc_process_cmd},
#endif
};

int fcc_register_cli_commands()
{
	int i;
	if (registered_fcc == 1)
		return WLAN_ERROR_NONE;
	for (i = 0; i < sizeof(fcc_tests) / sizeof(struct cli_command); i++)
		if (cli_register_command(&fcc_tests[i]))
			return WLAN_ERROR_ACTION;
	registered_fcc = 1;
	return WLAN_ERROR_NONE;
}

void user_app_hardfault_handler(void)
{
}



/**
 * All application specific initialization is performed here
 */
int main()
{

	wmstdio_init(UART0_ID, 0);
	cli_init();
	fcc_register_cli_commands();

	part_init();
	struct partition_entry *p;
	p = part_get_layout_by_id(FC_COMP_WLAN_FW, NULL);
	if (p == NULL) {
		wmprintf("part get layout failed\r\n");
		return -WM_FAIL;
	}
	flash_desc_t fl;
	part_to_flash_desc(p, &fl);

	int ret = sd_wifi_init(WLAN_TYPE_WIFI_CALIB,
				WLAN_FW_IN_FLASH, &fl, NULL);

	/* At this point you can use the CLI for trying FCC commands.
	 * Optionally, you can call the parameter specific
	 * functions in here, to avoid the use of serial console.
	 * These functions include:
	 * fcc_get_set_channel : to get/set rf channel
	 * fcc_get_set_antenna : to get/set antenna
	 * fcc_set_datarate : to set the datarate
	 * fcc_set_rf_power : to set rf power and modulation
	 * fcc_set_channel_bw : to set channel bandwidth
	 * fcc_enable_disable_tx_cw_mode : to enable/disable tx cw mode
	 * fcc_enable_disable_tx_cont_mode : to enable/disable tx cont mode
	 * For parameter passing and return values of each,
	 * please refer these functions. (defined above)
	 */
#ifdef APPCONFIG_BT_SUPPORT
	bt_drv_init();
	ret = os_queue_create(&le_tx_event_queue, "le_tx_event_queue",
			sizeof(le_pkt_resp_t),
			&le_tx_event_queue_data);
	if (ret) {
		wmprintf("Failed to create BLE queue: %d", ret);
		return -WM_FAIL;
	}

	bt_drv_set_cb(&bt_pkt_recv);
#endif
	return ret;
}
