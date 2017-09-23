
#define PORT 0x3f8   /* COM1 */
#define PORT_IO 0
#define PORT_INTR 1
#define PORT_FIFO_CONTROL 2
#define PORT_LINE_CONTROL 3
#define PORT_LINE_STATUS 5

void init_serial() {
	// disable all interrupts
	outb(PORT + PORT_INTR, 0x00);

	// no parity, 8 data bits, one stop bit, dlab active
	outb(PORT + PORT_LINE_CONTROL, 0x3);

	// divisor 0x0003 -> 38400 bps
	outb(PORT + 0, 0x03);
	outb(PORT + 1, 0x00);

	// disable fifo
	outb(PORT + PORT_FIFO_CONTROL, 0);
}

int is_transmit_empty() {
	return (inb(PORT + PORT_LINE_STATUS) & 0x20) != 0;
}

void write_serial(char a) {
	while (!is_transmit_empty());

	outb(PORT + PORT_IO, a);
}
