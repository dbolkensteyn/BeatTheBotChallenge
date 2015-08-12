#ifndef __BTB_SERIAL_H__
#define __BTB_SERIAL_H__

#include <string>
#include <termios.h>

namespace BTB
{
  class SerialPort
  {
  public:
    // Mac users: Use "/dev/cu.xxx", not "/dev/tty.xxx"!
    SerialPort(const std::string &port);
    ~SerialPort();
    int fd;
  };

  class Serial
  {
  public:
    // Mac users: Use "/dev/cu.xxx", not "/dev/tty.xxx"!
    Serial(const std::string &port);
    ~Serial();
    std::string readLine();
    void writeLine(std::string line);
  private:
    SerialPort serialPort;
    struct termios oldtio;
  };
}

#endif
