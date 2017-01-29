#include <stddef.h>
#include <stdint.h>

#define BLE_ATT_ERROR_RESP		0x01
#define BLE_ATT_EXCHANGE_MTU_REQ	0x02
#define BLE_ATT_EXCHANGE_MTU_RESP	0x03
#define BLE_ATT_FIND_INFORMATION_REQ	0x04
#define BLE_ATT_FIND_INFORMATION_RESP	0x05
#define BLE_ATT_FIND_BY_TYPE_REQ	0x06
#define BLE_ATT_FIND_BY_TYPE_RESP	0x07
#define BLE_ATT_READ_BY_TYPE_REQ	0x08
#define BLE_ATT_READ_BY_TYPE_RESP	0x09
#define BLE_ATT_READ_REQ		0x0a
#define BLE_ATT_READ_RESP		0x0b
#define BLE_ATT_READ_BLOB_REQ		0x0c
#define BLE_ATT_READ_BLOB_RESP		0x0d
#define BLE_ATT_READ_MULTIPLE_REQ	0x0e
#define BLE_ATT_READ_MULTIPLE_RESP	0x0f
#define BLE_ATT_READ_BY_GROUP_REQ	0x10
#define BLE_ATT_READ_BY_GROUP_RESP	0x11
#define BLE_ATT_WRITE_REQ		0x12
#define BLE_ATT_WRITE_RESP		0x13
#define BLE_ATT_WRITE_CMD		0x16
#define BLE_ATT_HANDLE_NOTIFICATION	0x1b
#define BLE_ATT_HANDLE_INDICATION	0x1d
#define BLE_ATT_HANDLE_CONFIRMATION	0x1e
#define BLE_ATT_SIGNED_WRITE_CMD	0x52

inline static void h_to_le16(void *dest, uint16_t val16)
{
	uint8_t *b = dest;
	b[0] = (val16 >> 0) & 0xff;
	b[1] = (val16 >> 8) & 0xff;
};

inline static uint16_t le16_to_h(void *src)
{
	uint8_t *b = src;
	return	b[0] | (b[1] << 8);
};

int ble_connect(uint8_t mac_addr[6]);
void ble_char_write_req(int fd, uint16_t handle, void *data, size_t len);


void hack_ble_read_primaries(int fd, uint16_t start, uint16_t end);
void hack_decode_primaries(uint8_t *buf, size_t buflen);
