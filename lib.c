#include "defines.h"
#include "serial.h"
#include "lib.h"

int putc(char c)
{
#ifdef UNITTEST
	return 0;
#else
	if (c == '\n') {
		serial_send_byte(SERIAL_DEFAULT_DEVICE, '\r');
	}
	return serial_send_byte(SERIAL_DEFAULT_DEVICE, c);
#endif
}

int puts(const char *str)
{
	while (*str) {
		putc(*(str++));
	}
	return 0;
}

unsigned char getc(void)
{
#ifdef UNITTEST
	unsigned char c = 0;
#else
	unsigned char c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
#endif
	c = (c == '\r') ? '\n' : c;
	putc(c); /* echo */
	return c;
}

char *gets(char *buf, int size)
{
	int i;
	if (size <= 0) return buf;
	for (i = 0; i < size - 1; i++) {
		buf[i] = getc();
		if (buf[i] == '\n') {
			buf[i] = '\0';
			return buf;
		}
	}
	buf[size - 1] = '\0';
	return buf;
}

int putxval(unsigned long value, int column)
{
	char buf[9];
	char *p;

	p = buf + sizeof buf - 1;
	*(p--) = '\0';
	if (!value && !column) {
		column++;
	}

	while (value || column) {
		*(p--) = "0123456789abcdef"[value & 0xf];
		value >>= 4;
		if (column) column--;
	}
	puts("0x");
	puts(p + 1);
	return 0;
}

int putdval(unsigned int value, int column)
{
	char buf[9];
	char *p;

	p = buf + sizeof buf - 1;
	*(p--) = '\0';
	if (!value && !column) {
		column++;
	}

	while (value || column) {
		*(p--) = "0123456789"[value % 10];
		value /= 10;
		if (column) column--;
	}
	puts(p + 1);
	return 0;
}



void *memset(void *b, int c, size_t len)
{
	char *p;
	for (p = b; len > 0; len--) {
		*(p++) = c;
	}
	return b;
}

void *memcpy(void *dst, const void *src, size_t len)
{
	char *d = dst;
	const char *s = src;
	for (; len > 0; len--) {
		*(d++) = *(s++);
	}
	return dst;
}

void *memmove(void *dst, const void *src, size_t len)
{
	char *d = dst;
	const char *s = src;
	if (src < dst) {
		for (; len > 0; len--) {
			d[len - 1] = s[len - 1];
		}
	} else if (src > dst) {
		for (; len > 0; len--) {
			*(d++) = *(s++);
		}
	}
	return dst;
}

int memcmp(const void *b1, const void *b2, size_t len)
{
	const char *p1 = b1, *p2 = b2;
	for (; len > 0; len--) {
		if (*p1 != *p2) {
			return (*p1 > *p2) ? 1 : -1;
		}
		p1++;
		p2++;
	}
	return 0;
}

size_t strlen(const char *s)
{
	size_t len;
	for (len = 0; *s; s++, len++) ;
	return len;
}

char *strcpy(char *dst, const char *src)
{
	char *d = dst;
	for (;; dst++, src++) {
		*dst = *src;
		if (!*src) break;
	}
	return d;
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 || *s2) {
		if (*s1 != *s2) {
			return (*s1 > *s2) ? 1 : -1;
		}
		s1++;
		s2++;
	}
	return 0;
}

int strncmp(const char *s1, const char *s2, size_t len)
{
	while ((*s1 || *s2) && (len > 0)) {
		if (*s1 != *s2) {
			return (*s1 > *s2) ? 1 : -1;
		}
		s1++;
		s2++;
		len--;
	}
	return 0;
}

char *strpbrk(const char *s, const char *stopset)
{
	for (; *s; s++) {
		for (; *stopset; stopset++) {
			if (*s == *stopset) {
				return (char *) s;
			}
		}
	}
	return 0;
}

static int ascii2hex(int c)
{
	if ('0' <= c && c <= '9') {
		return c - '0';
	}
	if ('a' <= c && c <= 'f') {
		return c - 'a' + 0xA;
	}
	if ('A' <= c && c <= 'F') {
		return c - 'A' + 0xA;
	}
	return 0;
}

/* baseは0,10,16のみ対応
 * baseが0の場合の8進数未対応
 * オーバーフロー未対応 */
long strtol(const char *s, char **endptr, int base)
{
	long ret = 0;
	int state = 0;
	int signed_state = 0;
	const char *p = s;

	if (base == 0) {
		ret = strtol(s, endptr, 10);
		if (ret == 0) {
			return strtol(s, endptr, 16);
		}
		return ret;
	} else if (base == 10) {
		for (; *p; p++) {
			if (state == 0 && signed_state == 0 && isspace(*p)) {
				continue;
			}
			if (isdigit(*p)) {
				state = 1;
				ret = (ret * 10) + ascii2hex(*p);
			} else if (state == 0 && signed_state == 0 && *p == '+') {
				signed_state = 2;
			} else if (state == 0 && signed_state == 0 && *p == '-') {
				signed_state = 1;
			} else {
				break;
			}
		}
	} else if (base == 16) {
		for (; *p; p++) {
			if (state == 0 && signed_state == 0 && isspace(*p)) {
				continue;
			}
			if (isxdigit(*p)) {
				if (state == 0 && *p == '0') {
					state = 1;
				} else {
					state = 3;
					ret = (ret << 4) + ascii2hex(*p);
				}
			} else if (state == 0 && signed_state == 0 && *p == '+') {
				signed_state = 2;
			} else if (state == 0 && signed_state == 0 && *p == '-') {
				signed_state = 1;
			} else if (state == 1 && (*p == 'x' || *p == 'X')) {
				state = 2;
			} else {
				break;
			}
		}
	}
	if (endptr) {
		if (state == 2 || (signed_state && state == 0)) {
			p = s;
		}
		*endptr = (char *) p;
	}
	return (signed_state == 1) ? -ret : ret;
}

unsigned long strtoul(const char *s, char **endptr, int base)
{
	return (unsigned long) strtol(s, endptr, base);
}

int atoi(const char *s)
{
	return (int) strtol(s, 0, 10);
}

long atol(const char *s)
{
	return strtol(s, 0, 10);
}

int isdigit(int c)
{
	if ('0' <= c && c <= '9') {
		return 1;
	}
	return 0;
}

int isxdigit(int c)
{
	if ('0' <= c && c <= '9') {
		return 1;
	}
	if ('a' <= c && c <= 'f') {
		return 1;
	}
	if ('A' <= c && c <= 'F') {
		return 1;
	}
	return 0;
}

int isspace(int c)
{
	if (c == ' ' || c == '\t' || c == '\f' || c == '\r' || c == '\n' || c == '\v') {
		return 1;
	}
	return 0;
}

int isprint(int c)
{
	if (0x20 <= c && c <= 0x7E) {
		return 1;
	}
	return 0;
}

int printf(const char *format, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4)
{
	/* NOTE: attention to buffer overflow */
	static char buf[256];
	const char *p;
	int ret;
	ret = sprintf(buf, format, arg1, arg2, arg3, arg4);

	for (p = buf; *p; p++) {
		putc(*p);
	}
	return ret;
}

static size_t set_ascii(char *ascii, const char *tmp, size_t size, size_t width, int left_flag, int zero_flag)
{
	size_t i;
	if (width > 1) {
		if (left_flag) {
			for (i = 0; i < size; i++) {
				ascii[i] = tmp[size - i - 1];
			}
			if (width > size) {
				for (i = 0; i < width - size; i++) {
					ascii[size + i] = ' ';
				}
				size += width - size;
			}
		} else {
			if (width > size) {
				for (i = 0; i < width - size; i++) {
					ascii[i] = zero_flag ? '0' : ' ';
				}
				for (i = 0; i < size; i++) {
					ascii[width - size + i] = tmp[size - i - 1];
				}
				size += width - size;
			} else {
				for (i = 0; i < size; i++) {
					ascii[i] = tmp[size - i - 1];
				}
			}
		}
	} else {
		for (i = 0; i < size; i++) {
			ascii[i] = tmp[size - i - 1];
		}
	}
	return size;
}

static size_t dec2ascii(char *ascii, unsigned int dec, size_t width, int left_flag, int zero_flag, int signed_flag)
{
	size_t i;
	char tmp[16];
	const char *num_str = "0123456789";
	int signed_dec = (int) dec;
	if (dec == 0) {
		tmp[0] = '0';
		i = 1;
	} else if (signed_flag && signed_dec < 0) {
		signed_dec = -signed_dec;
		for (i = 0; signed_dec > 0 && i < 15; i++) {
			tmp[i] = num_str[signed_dec % 10];
			signed_dec /= 10;
		}
		tmp[i++] = '-';
	} else {
		for (i = 0; dec > 0 && i < 16; i++) {
			tmp[i] = num_str[dec % 10];
			dec /= 10;
		}
	}
	return set_ascii(ascii, tmp, i, width, left_flag, zero_flag);
}

static size_t hex2ascii(char *ascii, unsigned long hex, size_t width, int left_flag, int zero_flag, int case_char)
{
	size_t i;
	char tmp[16];
	const char *num_str = (case_char == 'X') ? "0123456789ABCDEF" : "0123456789abcdef";
	if (hex == 0) {
		tmp[0] = '0';
		i = 1;
	} else {
		for (i = 0; hex > 0 && i < 16; i++) {
			tmp[i] = num_str[hex & 0xf];
			hex >>= 4;
		}
	}
	return set_ascii(ascii, tmp, i, width, left_flag, zero_flag);
}

int sprintf(char *buf, const char *format, unsigned long arg1, unsigned long arg2, unsigned long arg3, unsigned long arg4)
{
	size_t i;
	const char *p = format;
	unsigned long arg_list[4];
	size_t arg_idx = 0;
	const char *tmp_str;
	unsigned long tmp_val;
	int left_flag;
	int zero_flag;
	size_t width;
	size_t inc;

	arg_list[0] = arg1;
	arg_list[1] = arg2;
	arg_list[2] = arg3;
	arg_list[3] = arg4;

	i = 0;
	while (*p != '\0') {
		if (*p != '%') {
			buf[i++] = *(p++);
			continue;
		}
		p++;
		if (*p == '%') {
			buf[i++] = *(p++);
			continue;
		}
		if (*p == '-') {
			left_flag = 1;
			p++;
		} else {
			left_flag = 0;
		}
		if (*p == '0') {
			zero_flag = 1;
			p++;
		} else {
			zero_flag = 0;
		}
		if ('1' <= *p && *p <= '9') {
			width = *p - '0';
			p++;
			if ('0' <= *p && *p <= '9') {
				width = 10 * width + (*p - '0');
				p++;
			}
		} else {
			width = 0;
		}
		switch (*p) {
		case 'c':
			buf[i++] = (char) arg_list[arg_idx++];
			p++;
			break;
		case 'd':
		case 'i':
		case 'u':
			tmp_val = (unsigned long) arg_list[arg_idx++];
			inc = dec2ascii(&buf[i], (unsigned int) tmp_val, width, left_flag, zero_flag, (*p == 'u') ? 0 : 1);
			i += inc;
			p++;
			break;
		case 'p':
			buf[i++] = '0';
			buf[i++] = 'x';
			zero_flag = 1;
			width = 8;
		case 'x':
		case 'X':
			tmp_val = (unsigned long) arg_list[arg_idx++];
			inc = hex2ascii(&buf[i], tmp_val, width, left_flag, zero_flag, *p);
			i += inc;
			p++;
			break;
		case 's':
			tmp_str = (const char *) arg_list[arg_idx++];
			while (*tmp_str != '\0') {
				buf[i++] = *(tmp_str++);
			}
			p++;
			break;
		default:
			p++;
			break;
		}
	}
	buf[i] = '\0';
	return (int) i;
}

void hexdump(const void *buf, size_t size)
{
	size_t i;
	size_t j;
	unsigned char tmp[16];
	const unsigned char *p;
	p = (const unsigned char *) buf;
	for (i = 0; i < size; i++) {
		if (i%16 == 0) {
			PRINTF1("%p: ", &p[i]);
		}
		PRINTF1("%02x ", p[i]);
		tmp[i%16] = p[i];
		if (i == size -1) {
			for (j = i+1; j < i + (16 - i%16); j++) {
				tmp[j%16] = ' ';
				PRINTF0("   ");
			}
			i = j - 1;
		}
		if (i%16 == 15) {
			PRINTF0(" ");
			for (j = 0; j < 16; j++) {
				if (isprint(tmp[j])) {
					PRINTF1("%c", tmp[j]);
				} else {
					PRINTF0(".");
				}
			}
			PRINTF0("\n");
		}
	}
}

