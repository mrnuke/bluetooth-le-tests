/*
 * Libble main test suite
 */

#include "ble.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#define ARRAY_SIZE(x)		(sizeof(x) / sizeof(x[0]))

#define DIGIT_OFF	0x0f
static unsigned int clean_digit(uint8_t digit)
{
	if (digit == DIGIT_OFF)
		return 0;

	if (digit > 9) {
		printf("Bad digit %x\n", digit);
		return 0;
	}

	return digit;
}

static float decode_decimals(uint8_t digits[4])
{
	float reading;

	reading = clean_digit(digits[0]) * 1;
	reading += clean_digit(digits[1]) * 10;
	reading += clean_digit(digits[2]) * 100;
	reading += clean_digit(digits[3]) * 1000;

	return reading;
}

static const struct {
	uint8_t mask;
	const char *name;
} mode_flags[] = {
	{ (1 << 7), " AC"},
	{ (1 << 6), " DC"},
	{ (1 << 4), " OKAY"},
	{ (1 << 3), " MIN"},
	{ (1 << 2), " MAX"},
};

static const struct {
	uint8_t unit;
	const char *name;
} units[] = {
	{ 0x01, "V" },
	{ 0x02, "A" },
	{ 0x03, "Ohm" },
	{ 0x04, "Hz" },
	{ 0x05, "uF" },
	{ 0x06, "Beep" },
	{ 0x07, "Diode" },
	{ 0x0a, "%" },
};

static uint8_t sprintf_mode_flags(char *str, uint8_t flags)
{
	size_t i;

	/* Assume we are idiots and cannot pass a pre-terminated string. */
	str[0] = '\0';
	for (i = 0; i < ARRAY_SIZE(mode_flags); i++) {
		if (!(flags & mode_flags[i].mask))
			continue;

		strcat(str, mode_flags[i].name);
		flags &= ~mode_flags[i].mask;
	}

	return flags;
}

static const char * unit_to_name(uint8_t unit)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(units); i++) {
		if (unit == units[i].unit)
			return units[i].name;
	}

	printf("No unit found for %02x\n", unit);
	return "";
}

void process_dmm_packet(uint8_t *data)
{
	float reading;
	uint8_t num_decimals, mode_flags, unit;
	char mode_str[128];
	const char *unit_str;
	const uint8_t overrange_reading[] = {0x0b, 0x0a, 0x00, 0x0b};

	num_decimals = data[9];
	unit = data[10];
	mode_flags = data[13];

	if (!memcmp(data + 5, overrange_reading, sizeof(overrange_reading))) {
		reading = INFINITY;
	} else {
		reading = decode_decimals(data + 5);
		reading /= powf(10, num_decimals);
	}

	mode_flags = sprintf_mode_flags(mode_str, mode_flags);
	unit_str = unit_to_name(unit);

	printf("%f %s %s\n", reading, unit_str, mode_str);

	if (mode_flags)
		printf("Some mode flags left over %02x\n", mode_flags);
}

#include <assert.h>
#define MIN(a, b)		((a) < (b) ? (a) : (b))
#define PACKET_LEN		15

void process(void *data, ssize_t len)
{
	static uint8_t pbuf[200];
	static size_t i, pbuf_len = 0;
	const uint8_t packet_hdr[] = {0xd5, 0xf0, 0x00, 0x0a};
	size_t num_free = sizeof(pbuf) - pbuf_len;

	assert(len <= PACKET_LEN);
	assert(num_free >= len);

	memcpy(pbuf + pbuf_len, data, len);
	pbuf_len += len;

	if (pbuf_len >= PACKET_LEN && !memcmp(pbuf, packet_hdr, 4)) {
// 		printf("Goats one\n");
		process_dmm_packet(pbuf);
		pbuf[0] = 0xde;
	}




	/* Cycle back excess bytes */
	for (i = 0; i < pbuf_len; i++) {
		if (pbuf[i] == packet_hdr[0])
			break;
	}
	pbuf_len -= MIN(i, pbuf_len);
	memmove(pbuf, pbuf + i, pbuf_len);
// 	printf("left with %zu baitz %zu\n", pbuf_len, i);
}

void process_ble_packet(void *packet, ssize_t len)
{
	uint8_t *pack = packet;
	uint16_t handle;


	switch(pack[0]) {
	case BLE_ATT_ERROR_RESP:
		printf("Error response\n");
		break;
	case BLE_ATT_READ_BY_GROUP_RESP:
		/* Read by group response */
		hack_decode_primaries(packet, len);
		break;
	case BLE_ATT_WRITE_RESP:
		/* Write response */
		printf("Write response\n");
		break;
	case BLE_ATT_HANDLE_NOTIFICATION:
		/* Notification */
		if (len < 3) {
			printf("Some tiny shit bro\n");
			break;
		}

		handle = le16_to_h(pack + 1);

		if (handle != 0x14) {
			printf("HACK: Notification on unknown handle %02x\n", handle);
		}

		packet = (void *)((uintptr_t) packet + 3);
		len -= 3;

		process(packet, len);
		break;
	default:
		printf("Some bad shit bro %02x\n", pack[0]);
	}


}

int main(int argc, char **argv)
{
	int fd;
	ssize_t len;


	uint8_t buf[1024];

	/* 80:30:DC:11:F0:10 */
	uint8_t mac[6] = {0x80, 0x30, 0xDC, 0x11, 0xF0, 0x10};
	uint8_t macr[6] = {0x10, 0xf0, 0x11, 0xdc, 0x30, 0x80};

	uint8_t com1[] = {0x01, 0x00};

	printf("Scheisestream\n");
	fd = ble_connect(macr);
	//fcntl(fd, F_SETFL, O_RDWR|O_NONBLOCK);
	ble_char_write_req(fd, 0x15, com1, sizeof(com1));
	hack_ble_read_primaries(fd, 1, 0xffff);

	//sleep(2);

	do {
		len = read(fd, buf, sizeof(buf));
		if (len > 0)
			process_ble_packet(buf, len);

	} while (len > 0);

	return 0;
}
