#pragma once

#include <string>
#include <string_view>
#include <sys/socket.h>
#include <unistd.h>
#include <functional>
#include <logger.hpp>
#include <INIReader.h>
#include <assert.h>
#include <poll.h>


#include <array>


using namespace kiq::log;

enum
{
  READ_BUFFER_SIZE = 4096
};

static const std::string get_current_working_directory()
{
  std::string ret;
  char        buffer[PATH_MAX];
  if (getcwd(buffer, sizeof(buffer)) != nullptr)
    ret = std::string(buffer);
  return ret;
}

static const char g_null_terminator = '\0';
//------------------------------------------------------------------
size_t write_to_channel(int, uint8_t*, size_t);
size_t read_channel    (int, uint8_t*, size_t);
size_t recv_channel    (int, char*,    size_t);
//--------------------------------------------------------------------
class context
{
public:
  static context&     instance();

  bool run               ();
  int  get_channel_socket()                         const;
  void set_channel_socket(int socket_fd);
  bool init              (const std::string& token);

private:
  context()                           = default;
  context(const context& c)           = delete;
  context(context&& c)                = delete;
  context operator=(const context& c) = delete;
  context operator=(context&& c)      = delete;

  bool is_socket_readable(int timeout_ms = 30)      const;

  int  m_socket_fd;
};

extern context& ctx();

