#include "Serial.hpp"

#include <sstream>
#include <stdexcept>
#include <fcntl.h>
#include <strings.h>
#include <unistd.h>

#define BAUDRATE B9600

// Mac users: Use "/dev/cu.xxx", not "/dev/tty.xxx"!
BTB::Serial::Serial(const std::string &port)
{
  fd = open(port.c_str(), O_RDWR | O_CLOEXEC | O_NOCTTY);
  if (fd < 0)
  {
    throw std::logic_error("open");
  }

  if (tcgetattr(fd, &oldtio) != 0)
  {
    throw std::logic_error("tcgetattr");
  }

  struct termios newtio;
  bzero(&newtio, sizeof(newtio));

  if (cfsetispeed(&newtio, BAUDRATE) != 0 || cfsetospeed(&newtio, BAUDRATE) != 0)
  {
    throw std::logic_error("cfsetispeed/cfsetospeed");
  }
  newtio.c_cflag = CS8 | CREAD;
  newtio.c_iflag = 0;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;
  newtio.c_cc[VMIN] = 1;
  newtio.c_cc[VTIME] = 0;

  if (tcflush(fd, TCIFLUSH) != 0)
  {
    throw std::logic_error("tcflush");
  }

  if (tcsetattr(fd, TCSANOW, &newtio) != 0)
  {
    throw std::logic_error("tcsetattr");
  }
}

std::string BTB::Serial::readLine()
{
  std::stringstream ss;

  // TODO: Not very efficient - but does the job
  char c, r;
  while ((r = read(fd, &c, 1)) == 1 && c != '\n')
  {
    ss << c;
  }
  if (r != 1)
  {
    throw std::logic_error("read");
  }

  return ss.str();
}

void BTB::Serial::writeLine(std::string line)
{
  if (write(fd, line.c_str(), line.length()) != line.length())
  {
    throw std::logic_error("write");
  }
  const char lf = '\n';
  if (write(fd, &lf, 1) != 1)
  {
    throw std::logic_error("write");
  }
}

BTB::Serial::~Serial()
{
  tcflush(fd, TCIOFLUSH);
  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
}
