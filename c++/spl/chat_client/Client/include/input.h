#ifndef input__
#define input__
#include <iostream>
#include <mutex>
#include <connectionHandler.h>
#include <thread>

class Input
{
  private:
    int _id;
    ConnectionHandler &connectionHandler;
    bool flag=0;

  public:
    Input(int id, ConnectionHandler &con) : _id(id),  connectionHandler(con) {}
    void run();
    std::string decode(std::string str);
    short bytesToShort(char* bytesArr);
    short byteToShort(char* bytesArr);
};

#endif  
