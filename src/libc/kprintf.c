#include <stdarg.h>
#include <system.h>

#define MAX_BUFFER		50

static void write_char(char c) {
	terminal__write(&c, 1);
}

static void print_sequence(char* s) {
	while (*s != 0) {
		write_char(*s);
		++s;
	}
}

char *convert(unsigned int num, int base, char* buffer)
{
	static const char representation[]= "0123456789ABCDEF";
	char *ptr;

	ptr = &buffer[MAX_BUFFER-1];
	*ptr = '\0';

	do
	{
		*--ptr = representation[num%base];
		num /= base;
	}while(num != 0);

	return(ptr);
}

void kprintf(const char *format, ...)
{
	char *traverse;
	int i;
	char *s;
	char buffer[MAX_BUFFER];

	//Module 1: Initializing Myprintf's arguments
	va_list arg;
	va_start(arg, format);

	for(traverse = (char*) format; *traverse != '\0'; traverse++)
	{
		while( *traverse != '%' && *traverse != 0 )
		{
			write_char(*traverse);
			traverse++;
		}

		if ( *traverse == 0 ) {
			return;
		}

		traverse++;

		//Module 2: Fetching and executing arguments
		switch(*traverse)
		{
			case 'c' : i = va_arg(arg,int);		//Fetch char argument
						write_char(i);
						break;

			case 'd' : i = va_arg(arg,int); 		//Fetch Decimal/Integer argument
						if(i<0)
						{
							i = -i;
							write_char('-');
						}
						print_sequence(convert(i, 10, buffer));
						break;

			case 'o': i = va_arg(arg,unsigned int); //Fetch Octal representation
						print_sequence(convert(i, 8, buffer));
						break;

			case 's': s = va_arg(arg,char *); 		//Fetch string
						if (s) {
							print_sequence(s);
						} else {
							print_sequence("(null)");
						}
						break;

			case 'x': i = va_arg(arg,unsigned int); //Fetch Hexadecimal representation
						print_sequence(convert(i, 16, buffer));
						break;

			case '%':
						write_char('%');
						break;
		}
	}

	//Module 3: Closing argument list to necessary clean-up
	va_end(arg);
}

/*int main()
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;

	mi = (1 << (bs-1)) + 1;
	kprintf("%s\n", ptr);
	kprintf("printf test\n");
	kprintf("%s is null pointer\n", np);
	kprintf("%d = 5\n", i);
	kprintf("%d = - max int\n", mi);
	kprintf("char %c = 'a'\n", 'a');
	kprintf("hex %x = ff\n", 0xff);
	kprintf("signed %d = hex %x\n", -3, -3, -3);
	kprintf("%d %s(s)", 0, "message");
	kprintf("\n");
	kprintf("%d %s(s) with %%\n", 0, "message");
	return 0;
}
*/
