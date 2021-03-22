#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <iomanip>

using namespace std; 

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

// int get_angles(uint8_t * buffer){
//   //Total bytes: 59, 
//   //5+6 : ROLL_IMU_ANGLE (16 bits)
//   //7+8 : ROLL_RC_ANGLE (16 bits)
//   //9+10+11+12 : ROLL_STATOR_REL_ANGLE (32 bits) camera actual euler angle
//   //
//   //23+24 : PITCH_IMU_ANGLE (16 bits)
//   //25+26 : PITCH_RC_TARGET_ANGLE (16 bits)
//   //27+28+29+30 : PITCH_STATOR_REL_ANGLE (32 bits)
//   //
//   //41+42 : YAW_IMU_ANGLE (16 bits)
//   //43+44 : YAW_RC_TARGET_ANGLE (16 bits)
//   //45+46+47+48 : YAW_STATOR_REL_ANGLE (32 bits)
  
//   cout << *(buffer + 5);
//   return 0;
// }

// int get_16bit_angle(){


// }

// int get_32bit_angle(){


// }

// int check_sum(){

// }

int main()
{
  const char * device = "\\\\.\\COM6";
  
  uint32_t baud_rate = 115200;
  // std::cout << "abc ";
  HANDLE port = open_serial_port(device, baud_rate);
  cout << "The port code: " << port << endl;
  if (port == INVALID_HANDLE_VALUE) { 
    cout << "invalid handle returned"; 
    return 1; }

  uint8_t smsg[] = {0x3e, 0x3d, 0x00, 0x3d, 0x00}; //Return IMU angles
  //uint8_t smsg[] = {0x3e, 0x45, 0x01, 0x46, 0x12, 0x12}; //return to head
  //uint8_t smsg[] = {0xff, 0x01, 0x0f, 0x10, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x16}; //turn to 40 degree pitch
  
  write_port(port, smsg, sizeof(smsg));
  cout << "Number of bytes sent: " << sizeof(smsg) << endl;
  cout << "Bytes written successfully! " << endl ;
  
  
  uint8_t rmsg[59];
  //vector<uint8_t> rmsg;
  //vector<uint8_t> * p = &rmsg;
  int num = read_port(port, rmsg, sizeof(rmsg));
  cout << num << endl;
  // for(int i=0; i<num; i++){
  //     std::cout << (int)rmsg[i];
  //     //std::cout << std::hex << rmsg[i] << std::dec;
  // }

  //get_angles(rmsg);

  CloseHandle(port);
  return 0;
}
