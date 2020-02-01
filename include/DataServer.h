#pragma once

#include "ConnectionHandler.h"
#include "Connection.h"

class DataServer
{
private:
   tcp::acceptor acceptor_;


   void start_accept();
public:
//constructor for accepting connection from client
  DataServer(boost::asio::io_service& io_service);
  void handle_accept(ConnectionHandler::ptr connection, const boost::system::error_code& err);

  Network network;
};