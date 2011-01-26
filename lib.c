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

int printf(const char *format, ...)
{
	/* NOTE: attention to buffer overflow */
	static char buf[256];
	const char *p;
	int ret;
	va_list ap;

	va_start(ap, format);
	ret = vsprintf(buf, format, ap);
	va_end(ap);

	for (p = buf; *p; p++) {
		putc(*p);
	}
	return ret;
}

/* 
 * flags:
 *
 * bit 0: left_flag
 * bit 1: zero_flag
 * bit 2: plus_flag
 * bit 3: space_flag
 * bit 4: signed_flag('u':0, 'd'or'i':1)
 * bit 5: X_flag('x':0, 'X':1)
 * bit 6: sharp_flag
 *
 * bit 8-15: width
 */
#define SET_LEFT_FLAG(flags)	((flags) |= 0x0001)
#define SET_ZERO_FLAG(flags)	((flags) |= 0x0002)
#define SET_PLUS_FLAG(flags)	((flags) |= 0x0004)
#define SET_SPACE_FLAG(flags)	((flags) |= 0x0008)
#define SET_SIGNED_FLAG(flags)	((flags) |= 0x0010)
#define SET_LARGEX_FLAG(flags)	((flags) |= 0x0020)
#define SET_SHARP_FLAG(flags)	((flags) |= 0x0040)
#define SET_WIDTH(flags, width)	((flags) |= ((unsigned char)((width) & 0xff) << 8))

#define IS_SET_LEFT_FLAG(flags)		((flags) & 0x0001)
#define IS_SET_ZERO_FLAG(flags)		((flags) & 0x0002)
#define IS_SET_PLUS_FLAG(flags)		((flags) & 0x0004)
#define IS_SET_SPACE_FLAG(flags)	((flags) & 0x0008)
#define IS_SET_SIGNED_FLAG(flags)	((flags) & 0x0010)
#define IS_SET_LARGEX_FLAG(flags)	((flags) & 0x0020)
#define IS_SET_SHARP_FLAG(flags)	((flags) & 0x0040)
#define GET_WIDTH(flags)			((unsigned char)(((flags) & 0xff00) >> 8))

static int set_ascii(char *ascii, const char *tmp, int size, unsigned long flags)
{
	int i;
	int width = GET_WIDTH(flags);
	if (width > 1) {
		if (IS_SET_LEFT_FLAG(flags)) {
			for (i = 0; i < size; i++) {
				ascii[i] = tmp[size - i - 1];
			}
			if (width > size) {
				for (i = 0; i < width - size; i++) {
					ascii[size + i] = ' ';
				}
				size = width;
			}
		} else {
			if (width > size) {
				if (IS_SET_ZERO_FLAG(flags) && 
						(IS_SET_SHARP_FLAG(flags) || IS_SET_PLUS_FLAG(flags) || IS_SET_SPACE_FLAG(flags) || 
						(IS_SET_SIGNED_FLAG(flags) && tmp[size - 1] == '-'))) {
					int n = IS_SET_SHARP_FLAG(flags) ? 2 /* "0[xX]" */ : 1 /* "[ +-]" */;
					for (i = 0; i < n; i++) {
						ascii[i] = tmp[size - i - 1];
					}
					for (i = 0; i < width - size; i++) {
						ascii[i + n] = '0';
					}
					for (i = 0; i < size - n; i++) {
						ascii[width - size + i + n] = tmp[size - i - 1 - n];
					}
				} else {
					for (i = 0; i < width - size; i++) {
						ascii[i] = IS_SET_ZERO_FLAG(flags) ? '0' : ' ';
					}
					for (i = 0; i < size; i++) {
						ascii[width - size + i] = tmp[size - i - 1];
					}
				}
				size = width;
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

#define TMP_SIZE	32
static int dec2ascii(char *ascii, unsigned int dec, unsigned long flags)
{
	int i;
	char tmp[TMP_SIZE];
	const char *num_str = "0123456789";
	int signed_dec = (int) dec;
	if (dec == 0) {
		tmp[0] = '0';
		i = 1;
		if (IS_SET_PLUS_FLAG(flags) || IS_SET_SPACE_FLAG(flags)) {
			tmp[i++] = IS_SET_PLUS_FLAG(flags) ? '+' : ' ';
		}
	} else if (IS_SET_SIGNED_FLAG(flags) && signed_dec < 0) {
		signed_dec = -signed_dec;
		for (i = 0; signed_dec > 0 && i < TMP_SIZE - 1; i++) {
			tmp[i] = num_str[signed_dec % 10];
			signed_dec /= 10;
		}
		tmp[i++] = '-';
	} else {
		for (i = 0; dec > 0 && i < TMP_SIZE - 1; i++) {
			tmp[i] = num_str[dec % 10];
			dec /= 10;
		}
		if (IS_SET_PLUS_FLAG(flags) || IS_SET_SPACE_FLAG(flags)) {
			tmp[i++] = IS_SET_PLUS_FLAG(flags) ? '+' : ' ';
		}
	}
	return set_ascii(ascii, tmp, i, flags);
}

static int hex2ascii(char *ascii, size_t hex, unsigned long flags)
{
	int i;
	char tmp[TMP_SIZE];
	const char *num_str = IS_SET_LARGEX_FLAG(flags) ? "0123456789ABCDEF" : "0123456789abcdef";
	if (hex == 0) {
		tmp[0] = '0';
		i = 1;
	} else {
		for (i = 0; hex > 0 && i < TMP_SIZE; i++) {
			tmp[i] = num_str[hex & 0xf];
			hex >>= 4;
		}
		if (IS_SET_SHARP_FLAG(flags)) {
			tmp[i++] = IS_SET_LARGEX_FLAG(flags) ? 'X' : 'x';
			tmp[i++] = '0';
		}
	}
	return set_ascii(ascii, tmp, i, flags);
}

int sprintf(char *buf, const char *format, ...)
{
	int ret;
	va_list ap;
	va_start(ap, format);
	ret = vsprintf(buf, format, ap);
	va_end(ap);
	return ret;
}

int vsprintf(char *buf, const char *format, va_list ap)
{
	int i;
	const char *p = format;
	const char *tmp_str;
	size_t tmp_val;
	unsigned long flags = 0;
	int inc;

	i = 0;
	while (*p != '\0') {
		int long_flag = 0;
		if (*p != '%') {
			buf[i++] = *(p++);
			continue;
		}
		p++;
		if (*p == '%') {
			buf[i++] = *(p++);
			continue;
		}
		if (*p == ' ') {
			SET_SPACE_FLAG(flags);
			p++;
		}
		if (*p == '+') {
			SET_PLUS_FLAG(flags);
			p++;
		}
		if (*p == '#') {
			SET_SHARP_FLAG(flags);
			p++;
		}
		if (*p == '-') {
			SET_LEFT_FLAG(flags);
			p++;
		}
		if (*p == ' ') {
			SET_SPACE_FLAG(flags);
			p++;
		}
		if (*p == '+') {
			SET_PLUS_FLAG(flags);
			p++;
		}
		if (*p == '0') {
			SET_ZERO_FLAG(flags);
			p++;
		}
		if ('1' <= *p && *p <= '9') {
			int width = *p - '0';
			p++;
			if ('0' <= *p && *p <= '9') {
				width = 10 * width + (*p - '0');
				p++;
			}
			SET_WIDTH(flags, width);
		} else if (*p == '*') {
			int width = (int) va_arg(ap, int);
			if (width < 0) {
				SET_LEFT_FLAG(flags);
				width = -width;
			}
			SET_WIDTH(flags, width);
			p++;
		}
		if (*p == 'h' || *p == 'l') {
			if (*p == 'l') {
				long_flag = 1;
			}
			p++;
		}
		switch (*p) {
		case 'c':
			buf[i++] = va_arg(ap, char);
			p++;
			break;
		case 'd':
		case 'i':
		case 'u':
			tmp_val = va_arg(ap, int);
			if (*p != 'u') {
				SET_SIGNED_FLAG(flags);
			}
			inc = dec2ascii(&buf[i], (unsigned int) tmp_val, flags);
			i += inc;
			p++;
			break;
		case 'p':
			buf[i++] = '0';
			buf[i++] = 'x';
			SET_ZERO_FLAG(flags);
			SET_WIDTH(flags, 8);
			long_flag = 1;
		case 'x':
		case 'X':
			tmp_val = long_flag ? va_arg(ap, size_t) : va_arg(ap, int);
			if (*p == 'X') {
				SET_LARGEX_FLAG(flags);
			}
			inc = hex2ascii(&buf[i], tmp_val, flags);
			i += inc;
			p++;
			break;
		case 's':
			tmp_str = va_arg(ap, const char *);
			while (*tmp_str != '\0') {
				buf[i++] = *(tmp_str++);
			}
			p++;
			break;
		default:
			p++;
			break;
		}
		flags = 0;
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
			printf("%p: ", &p[i]);
		}
		printf("%02x ", p[i]);
		tmp[i%16] = p[i];
		if (i == size -1) {
			for (j = i+1; j < i + (16 - i%16); j++) {
				tmp[j%16] = ' ';
				printf("   ");
			}
			i = j - 1;
		}
		if (i%16 == 15) {
			printf(" ");
			for (j = 0; j < 16; j++) {
				if (isprint(tmp[j])) {
					printf("%c", tmp[j]);
				} else {
					printf(".");
				}
			}
			printf("\n");
		}
	}
}

