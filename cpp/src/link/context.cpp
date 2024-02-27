#include "context.hpp"

bool context::init(const std::string& token)
{
  const char* msg_ptr = &token[0];
  bool res = write_to_channel(m_socket_fd, reinterpret_cast<uint8_t*>(const_cast<char*>(msg_ptr)), token.size());
  if (!res)
  {
    klog().e("Write failed with error: {}", strerror(errno));
    throw std::runtime_error("Failed to open virtual channel");
  }

  return true;
}
//------------------------------------------------
bool context::run()
{
  try
  {
    klog().d("context::run()");
    size_t      rx = 0;
    size_t      tx = 0;
    char        read_buffer[READ_BUFFER_SIZE];
    std::string message       = fmt::format("C++ Test {}", getpid());

    klog().d("Created read buffer");
    klog().d("Created message: {}", message);
    klog().d("Writing to channel");

    tx = write_to_channel(m_socket_fd, reinterpret_cast<uint8_t*>(message.data()), message.size());
    if (!tx)
    {
      klog().e("Write failed with error: {}", strerror(errno));
      return false;
    }

    if (is_socket_readable())
    rx = recv_channel(m_socket_fd, read_buffer, READ_BUFFER_SIZE);

    if (!rx)
      klog().w("Nothing to read");
    else
    {
      klog().d("Deserializing");

      read_buffer[rx] = g_null_terminator;
      std::string deserialized{read_buffer};

      klog().d("Read from channel: {}", deserialized);
    }

    return true;
  }
  catch (const std::exception& e)
  {
    klog().e("Exception caught: {}", e.what());
    return false;
  }
}
//------------------------------------------------
void context::set_channel_socket(int socket_fd)
{
  m_socket_fd = socket_fd;
}
//------------------------------------------------
int context::get_channel_socket() const
{
  return m_socket_fd;
}
//------------------------------------------------
context& context::instance()
{
  static context* context_instance;
  if (!context_instance)
    context_instance = new context;

  assert(context_instance);

  return *context_instance;
}
//------------------------------------------------
context& ctx()
{
  return context::instance();
}
//------------------------------------------------
bool context::is_socket_readable(int timeout_ms) const
{
  klog().d("Polling socket");
  struct pollfd pfd;

  pfd.fd     = m_socket_fd;
  pfd.events = POLLIN;

  int result = poll(&pfd, 1, timeout_ms);

  if (result == -1)
  {
    klog().e("Poll failed");
    return false;
  }
  else if (result == 0)
  {
    klog().t("No data to read from {}", m_socket_fd);
    return false;
  }

  klog().d("Socket ready to be read");
  return true;
}
//------------------------------------------------
//--------------HELPERS---------------------------
//------------------------------------------------
size_t write_to_channel(int socket, uint8_t* buffer, size_t size)
{
  klog().d("write_to_channel called to write {} bytes to {}", size, socket);

  size_t bytes_written = 0;
  while (bytes_written < size)
  {
    ssize_t curr_written = write(socket, buffer + bytes_written, size - bytes_written);
    if (curr_written <= 0)
    {
      klog().i("Channel write failed with {} bytes written. Error:{}",
        bytes_written, strerror(errno));
      return 0;
    }

    bytes_written += curr_written;

    klog().t("Wrote {} of {} bytes", bytes_written, size);
  }

  return bytes_written;
}
//--------------------------------------------------------------------
size_t read_channel(int socket_fd, uint8_t *buffer, size_t size)
{
  klog().d("read_channel called to read {} bytes from {}", size, socket_fd);

  size_t bytes_read = 0;
  while (bytes_read < size)
  {
    ssize_t curr_read = read(socket_fd, buffer + bytes_read, size - bytes_read);
    if (curr_read <= 0)
    {
      klog().i("Error or unable to read with {} bytes read. Last read returned {}. Last error: {}",
        bytes_read, curr_read, strerror(errno));
      return 0;
    }

    bytes_read += curr_read;

    klog().t("read {} of {} bytes", bytes_read, size);
  }

  return bytes_read;
}
//--------------------------------------------------------------------
size_t recv_channel(int socket_fd, char* buffer, size_t buf_size)
{
  klog().d("Reading from {}", socket_fd);

  size_t bytes_read = 0;

  bytes_read = recv(socket_fd, buffer + bytes_read, buf_size, MSG_DONTWAIT);

  if (bytes_read == -1)
    klog().i("Error calling recv. Last error: {}", strerror(errno));

  klog().t("Read {} bytes", bytes_read);

  return bytes_read;
}
