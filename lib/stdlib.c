#include <stdlib.h>
#include <ctype.h>
#include<ctype.h>
#include <string.h>
#include<sys_req.h>
#include<mpx/device.h>

int atoi(const char *s)
{
	int res = 0;
	char sign = ' ';

	while (isspace(*s))
	{
		s++;
	}

	if (*s == '-' || *s == '+')
	{
		sign = *s;
		s++;
	}

	while ('0' <= *s && *s <= '9')
	{
		res = res * 10 + (*s - '0');
		s++;
	}

	if (sign == '-')
	{
		res = res * -1;
	}

	return res;
}

int atox(const char *s)
{
	int num = 0;

	while (*s != '\0')
	{
		num = num * 16;

		if (*s >= '0' && *s <= '9')
		{
			num = num + (*s - 48);
		}
		else if (*s >= 'a' && *s <= 'f')
		{
			num = num + (*s - 87);
		}
		if (*s >= 'A' && *s <= 'F')
		{
			num = num + (*s - 55);
		}

		s++;
	}

	return num;
}

char *numtoa(uint32_t i, char *dest, int base)
{
	char defaul[50] = {'\0'};

	int index = 0;

	while (i > 0)
	{
		int temp = (i % base);
		if (temp < 10)
		{
			defaul[index] = temp + '0';
		}
		else
		{
			defaul[index] = temp - 10 + 'a';
		}
		i = i / base;
		index++;
	}

	for (uint32_t j = 0; j < strlen(defaul); j++)
	{
		dest[strlen(defaul) - j - 1] = defaul[j];
	}

	dest[strlen(defaul)] = '\0';

	return dest;
}

int printf(char *format, ...)
{
	char buf[50] = {'\0'};
	int bufIndex = 0;

	va_list args;
	va_start(args, format);

	while (*format != '\0')
	{

		if (*format == '%')
		{
			format++;

			switch (*format)
			{
			case 'd':
			{
				int temp = va_arg(args, int);
				char num[50] = {'\0'};

				numtoa(temp, num, 10);

				sys_req(WRITE, COM1, buf, strlen(buf));
				sys_req(WRITE, COM1, num, strlen(num));

				memset(buf, '\0', 50);
				bufIndex = 0;

				break;
			}
			case 'c':
			{
				char temp = va_arg(args, int);

				buf[bufIndex++] = temp;
				break;
			}
			case 's':
			{
				char *temp = va_arg(args, char *);

				sys_req(WRITE, COM1, buf, strlen(buf));
				sys_req(WRITE, COM1, temp, strlen(temp));

				memset(buf, '\0', 50);
				bufIndex = 0;
				break;
			}
			case 'x':
			{
				uint32_t temp = va_arg(args, uint32_t);
				char num[50] = {'\0'};

				numtoa(temp, num, 16);

				sys_req(WRITE, COM1, buf, strlen(buf));
				sys_req(WRITE, COM1, num, strlen(num));

				memset(buf, '\0', 50);
				bufIndex = 0;

				break;
			}
			default:
			{
				return 1;
			}
			}
		}
		else
		{
			buf[bufIndex++] = *format;
		}

		if (bufIndex >= 50)
		{
			sys_req(WRITE, COM1, buf, strlen(buf));

			memset(buf, '\0', 50);
			bufIndex = 0;
		}

		format++;
	}
	return 0;
}

void puts(char* str){
	printf("%s\n", str);
}