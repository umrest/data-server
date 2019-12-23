// https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio/example/cpp11/chat/chat_server.cpp
// Do something similar to this, but with specific named clients

#include <iostream>

#include "DataServer.h"


int main(){

try
    {
    boost::asio::io_service io_service;  
    DataServer server(io_service);
    io_service.run();
    }
  catch(std::exception& e)
    {
    std::cerr << e.what() << std::endl;
    }
  return 0;
}