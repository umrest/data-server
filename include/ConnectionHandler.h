#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

#include "CommunicationDefinitions.h"


using namespace boost::asio;
using ip::tcp;

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
protected:
  tcp::socket sock;
  enum { max_length = 1024 };
  unsigned char data[max_length];

public:

ConnectionHandler(boost::asio::io_service& io_service): sock(io_service){

}

typedef boost::shared_ptr<ConnectionHandler> ptr;

  virtual void on_recv(CommunicationDefinitions::TYPE type) = 0;
//socket creation

  tcp::socket& socket()
  {
    return sock;
  }
    void start_read(){
      
        // read the header
        sock.async_read_some(
            boost::asio::buffer(data, 1),
            boost::bind(&ConnectionHandler::handle_read_header,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }
  void start()
  {
    start_read();
  }

  void handle_read_header(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
         CommunicationDefinitions::TYPE type = (CommunicationDefinitions::TYPE)data[0];
         //std::cout << "Recieved Type: " << type << std::endl;
         auto c = CommunicationDefinitions();
         int size = c.PACKET_SIZES.at(type);

         sock.async_read_some(
            boost::asio::buffer(data + 1, size),
                boost::bind(&ConnectionHandler::handle_read,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        
    } else {
         std::cerr << "error: " << err.message() << std::endl;
         sock.close();
    }
  }
  void handle_read(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
         CommunicationDefinitions::TYPE type = (CommunicationDefinitions::TYPE)data[0];
         on_recv(type);
         start_read();
    } else {
         std::cerr << "error: " << err.message() << std::endl;
         sock.close();
    }
  }

  void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
       std::cout << "Send to client"<< std::endl;
    } else {
       std::cerr << "error: " << err.message() << std::endl;
       sock.close();
    }
  }

  void write(unsigned char* data, int size){
    sock.async_write_some(boost::asio::buffer(data, size), boost::bind(&ConnectionHandler::handle_write,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
  }
};