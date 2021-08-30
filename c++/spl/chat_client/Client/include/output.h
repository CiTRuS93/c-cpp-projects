#ifndef output__
#define output__
#include <iostream>
#include <mutex>
#include <connectionHandler.h>
#include <thread>

class output
{
  private:
    int _id;
    ConnectionHandler &connectionHandler;

  public:
    output(int id, ConnectionHandler &con) : _id(id),  connectionHandler(con) {}
    void run();
    std::string encode(std::string str);
    void shortToBytes(short num, char* bytesArr);
    void shortToByte(short num, char* bytesArr);
};

#endif  