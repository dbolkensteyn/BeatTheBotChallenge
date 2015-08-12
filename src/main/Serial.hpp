#ifndef __BTB_SERIAL_H__
#define __BTB_SERIAL_H__

#include <string>
#include <termios.h>

namespace BTB
{
  class Serial
  {
  public:
    // Mac users: Use "/dev/cu.xxx", not "/dev/tty.xxx"!
    Serial(const std::string &port);
    std::string readLine();
    void writeLine(std::string line);
    ~Serial();
  private:
    int fd;
    struct termios oldtio;
  };
}

#endif
