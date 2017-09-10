#ifndef MPCI4G_COM_H_
#define MPCI4G_COM_H_

#define	PARITY_NONE	0
#define	PARITY_EVEN	1
#define	PARITY_ODD	2

static int		clocal;
static int		hardware;
static int		software;
static int		passflow;
static int		parity = PARITY_NONE;
static int		databits = 8;
static int		twostopb;
static struct termios	savetio_local;
static struct termios	savetio_remote;
static int		rfd ;
static unsigned int	baud = 115200;

/*****************************************************************************/
/*
 *	Baud rate table for baud rate conversions.
 */
typedef struct baudmap {
	unsigned int	baud;
	unsigned int	flag;
} baudmap_t;

struct baudmap	baudtable[] = {
	{ 0, B0 },
	{ 50, B50 },
	{ 75, B75 },
	{ 110, B110 },
	{ 134, B134 },
	{ 150, B150 },
	{ 200, B200 },
	{ 300, B300 },
	{ 600, B600 },
	{ 1200, B1200 },
	{ 1800, B1800 },
	{ 2400, B2400 },
	{ 4800, B4800 },
	{ 9600, B9600 },
	{ 19200, B19200 },
	{ 38400, B38400 },
	{ 57600, B57600 },
	{ 115200, B115200 },
	{ 230400, B230400 },
	{ 460800, B460800 }
};

#define	NRBAUDS		(sizeof(baudtable) / sizeof(struct baudmap))

/*****************************************************************************/

/*
 *	Verify that the supplied baud rate is valid.
 */

int baud2flag(unsigned int speed)
{
	int	i;

	for (i = 0; (i < NRBAUDS); i++) {
		if (speed == baudtable[i].baud)
			return(baudtable[i].flag);
	}
	return(-1);
}

/*****************************************************************************/

void restorelocaltermios(void)
{
	if (tcsetattr(1, TCSAFLUSH, &savetio_local) < 0) {
		fprintf(stderr, "ERROR: local tcsetattr(TCSAFLUSH) failed, "
			"errno=%d\n", errno);
	}
}

/*****************************************************************************/

void savelocaltermios(void)
{
	if (tcgetattr(1, &savetio_local) < 0) {
		fprintf(stderr, "ERROR: local tcgetattr() failed, errno=%d\n",
			errno);
		exit(0);
	}
}

/*****************************************************************************/

void restoreremotetermios(void)
{
	/*
	 *	This can fail if remote hung up, don't check return status.
	 */
	tcsetattr(rfd, TCSAFLUSH, &savetio_remote);
}

/*****************************************************************************/

void saveremotetermios(void)
{
	if (tcgetattr(rfd, &savetio_remote) < 0) {
		fprintf(stderr, "ERROR: remote tcgetattr() failed, errno=%d\n",
			errno);
		exit(0);
	}
}

/*****************************************************************************/

/*
 *	Set local port to raw mode, no input mappings.
 */

int setlocaltermios()
{
	struct termios	tio;

	if (tcgetattr(1, &tio) < 0) {
		fprintf(stderr, "ERROR: local tcgetattr() failed, errno=%d\n",
			errno);
		exit(1);
	}

	if (passflow)
		tio.c_iflag &= ~(ICRNL|IXON);
	else
		tio.c_iflag &= ~ICRNL;
	tio.c_lflag = 0;
	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr(1, TCSAFLUSH, &tio) < 0) {
		fprintf(stderr, "ERROR: local tcsetattr(TCSAFLUSH) failed, "
			"errno=%d\n", errno);
		exit(1);
	}
	return(0);
}

/*****************************************************************************/

/*
 *	Set up remote (connect) port termio settings according to
 *	user specification.
 */

int setremotetermios()
{
	struct termios	tio;

	memset(&tio, 0, sizeof(tio));
	tio.c_cflag = CREAD | HUPCL | baud2flag(baud);

	if (clocal)
		tio.c_cflag |= CLOCAL;

	switch (parity) {
	case PARITY_ODD:	tio.c_cflag |= PARENB | PARODD; break;
	case PARITY_EVEN:	tio.c_cflag |= PARENB; break;
	default:		break;
	}

	switch (databits) {
	case 5:		tio.c_cflag |= CS5; break;
	case 6:		tio.c_cflag |= CS6; break;
	case 7:		tio.c_cflag |= CS7; break;
	default:	tio.c_cflag |= CS8; break;
	}
	
	if (twostopb)
		tio.c_cflag |= CSTOPB;

	if (software)
		tio.c_iflag |= IXON | IXOFF;
	if (hardware)
		tio.c_cflag |= CRTSCTS;

	tio.c_cc[VMIN] = 1;
	tio.c_cc[VTIME] = 0;

	if (tcsetattr(rfd, TCSAFLUSH, &tio) < 0) {
		fprintf(stderr, "ERROR: remote tcsetattr(TCSAFLUSH) failed, "
			"errno=%d\n", errno);
		exit(1);
	}
	return(0);
}

/*****************************************************************************/

#endif /*ifndef MPCI4G_H_*/
 

