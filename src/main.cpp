// https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio/example/cpp11/chat/chat_server.cpp
// Do something similar to this, but with specific named clients

#include <iostream>

#include "DataServer.h"



int main(int argc, char const *argv[]){
int port = 8091;
if(argc > 1){
  port = atoi(argv[1]);
}
try
    {
    boost::asio::io_service io_service;  
    DataServer server(io_service, port);
    io_service.run();
    }
  catch(std::exception& e)
    {
    std::cerr << e.what() << std::endl;
    }
  return 0;
}