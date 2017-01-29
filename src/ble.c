#include "ble.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <stdio.h>
#include <sys/uio.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>


void hack_ble_read_primaries(int fd, uint16_t start, uint16_t end)
{
	uint8_t buf[7];

	buf[0] = 0x10;
	h_to_le16(buf + 1, start);
	h_to_le16(buf + 3, end);
	h_to_le16(buf + 5, 0x2800);

	write(fd, buf, sizeof(buf));
}

void hack_decode_primaries(uint8_t *buf, size_t buflen)
{
	uint8_t entry_len = buf[1];
	size_t len, i;
	uint16_t start_handle, end_handle;

	if (entry_len < 5) {
		printf("Holee scheite bad respoanza\n");
		return;
	}

	if (buflen < 2) {
		printf("El packeto seems fucketo\n");
		return;
	}

	buf += 2;
	len = buflen - 2;

	while (len >= entry_len) {
		start_handle = le16_to_h(buf + 0);
		end_handle = le16_to_h(buf + 2);
		printf("%04x -> %04x: ", start_handle, end_handle);

		for (i = 4; i < entry_len; i++)
			printf(" %02x", buf[i]);

		printf("\n");

		len -= entry_len;
		buf += entry_len;
	}
}

void ble_char_write_req(int fd, uint16_t handle, void *data, size_t len)
{
	size_t plen = len + 3;
	uint8_t hdr[3];

	struct iovec iov[2] = { {
		.iov_base = hdr,
		.iov_len = sizeof(hdr),
	}, {
		.iov_base = data,
		.iov_len = len,
	} };

	hdr[0] = BLE_ATT_WRITE_REQ;
	h_to_le16(hdr + 1, handle);

	writev(fd, iov, 2);
}

int ble_connect(uint8_t mac_addr[6])
{
// 	struct bt_security bt_sec;
	struct sockaddr_l2 sl2;
	int i, s, ret;

	s = socket(AF_BLUETOOTH, SOCK_SEQPACKET, 0);
	if (s < 0) {
		perror("Could not connect to socket\n");
		return s;
	}

// 	bt_sec.level = BT_SECURITY_LOW;
// 	bt_sec.key_size = 0;
//
// 	if (setsockopt(s, SOL_BLUETOOTH, BT_SECURITY, &bt_sec,sizeof(bt_sec)) != 0) {
// 		perror("set bt security level");
// 	}


	memset(&sl2, 0, sizeof(sl2));
	sl2.l2_family = AF_BLUETOOTH;
	sl2.l2_cid = 0x0004;
	sl2.l2_bdaddr_type = BDADDR_LE_PUBLIC;

	ret = bind(s, (void *)&sl2, sizeof(sl2));
	if (ret < 0) {
		perror("bind");
	}

	memcpy(&sl2.l2_bdaddr, mac_addr, 6);

	ret = connect(s, (void *)&sl2, sizeof(sl2));
	if (ret < 0) {
		perror("connect");
	}

	return s;
}
