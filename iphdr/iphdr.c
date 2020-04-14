#include <stdio.h>

unsigned short checksum(unsigned short *buf, int nword)
{
	unsigned long sum;
	for (sum = 0; nword > 0; nword--)
		sum += *buf++;

	sum = (sum>>16) + (sum&0xffff);
	sum += (sum>>16);

	return ~sum;
}

int main(int argc, char *argv[])
{
	unsigned short a[20] = {0x4500,0x0054,0x8771,0x0000,0x4001,0x0000,0x0101,0x010e,0x0101,0x0d02};
	unsigned short b[20] = {0x4500,0x0054,0x8775,0x0000,0x4001,0x0000,0x0101,0x010e,0x0101,0x0d02};
	unsigned short c[20] = {0x4500,0x0054,0x8776,0x0000,0x4001,0x0000,0x0101,0x010e,0x0101,0x0d02};

	unsigned short d[2] = {0x51dc, 0x6b7c};

	printf("hello, iphdr, %x\n", checksum(a, 20));
	printf("hello, iphdr, %x\n", checksum(b, 20));
	printf("hello, iphdr, %x\n", checksum(c, 20));

	printf("hello, iphdr, %x\n", checksum(d, 2));


	return 0;
}
