#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <iostream>
 
HANDLE open_serial_port(const char * device, uint32_t baud_rate)
{
  HANDLE port = CreateFileA(device, GENERIC_READ | GENERIC_WRITE, 0, NULL,
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (port == INVALID_HANDLE_VALUE)
  {
    //print_error(device);
    return INVALID_HANDLE_VALUE;
  }
 
  // Flush away any bytes previously read or written.
  BOOL success = FlushFileBuffers(port);
  if (!success)
  {
    //print_error("Failed to flush serial port");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  // Configure read and write operations to time out after 100 ms.
  COMMTIMEOUTS timeouts = { 0 };
  timeouts.ReadIntervalTimeout = 0;
  timeouts.ReadTotalTimeoutConstant = 100;
  timeouts.ReadTotalTimeoutMultiplier = 0;
  timeouts.WriteTotalTimeoutConstant = 100;
  timeouts.WriteTotalTimeoutMultiplier = 0;
 
  success = SetCommTimeouts(port, &timeouts);
  if (!success)
  {
    //print_error("Failed to set serial timeouts");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  DCB state;
  state.DCBlength = sizeof(DCB);
  success = GetCommState(port, &state);
  if (!success)
  {
    //print_error("Failed to get serial settings");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  state.BaudRate = baud_rate;
 
  success = SetCommState(port, &state);
  if (!success)
  {
    //print_error("Failed to set serial settings");
    CloseHandle(port);
    return INVALID_HANDLE_VALUE;
  }
 
  return port;
}

int write_port(HANDLE port, uint8_t * buffer, size_t size)
{
  DWORD written;
  BOOL success = WriteFile(port, buffer, size, &written, NULL);
  if (!success)
  {
    //print_error("Failed to write to port");
    return -1;
  }
  if (written != size)
  {
    //print_error("Failed to write all bytes to port");
    return -1;
  }
  return 0;
}

SSIZE_T read_port(HANDLE port, uint8_t * buffer, size_t size)
{
  DWORD received;
  BOOL success = ReadFile(port, buffer, size, &received, NULL);
  if (!success)
  {
    //print_error("Failed to read from port");
    return -1;
  }
  return received;
}

int main()
{
  // Choose the serial port name.  If the Jrk is connected directly via USB,
  // you can run "jrk2cmd --cmd-port" to get the right name to use here.
  // COM ports higher than COM9 need the \\.\ prefix, which is written as
  // "\\\\.\\" in C because we need to escape the backslashes.
  const char * device = "COM6";
 
  // Choose the baud rate (bits per second).  This does not matter if you are
  // connecting to the Jrk over USB.  If you are connecting via the TX and RX
  // lines, this should match the baud rate in the Jrk's serial settings.
  uint32_t baud_rate = 115200;
 
  HANDLE port = open_serial_port(device, baud_rate);
  if (port == INVALID_HANDLE_VALUE) { return 1; }

  uint8_t smsg[] = {0x3e, 0x45, 0x01, 0x46, 0x12, 0x12};
  write_port(port, smsg, sizeof(smsg));

  uint8_t rmsg[20];
  int num = read_port(port, rmsg, sizeof(rmsg));

  for(int i=0; i<num; i++){
      std::cout << rmsg[i];
  }
  CloseHandle(port);
  return 0;
}