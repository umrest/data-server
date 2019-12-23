#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

#include <comm/CommunicationDefinitions.h>


using namespace boost::asio;
using ip::tcp;

using namespace comm;

class ConnectionHandler : public boost::enable_shared_from_this<ConnectionHandler>
{
protected:
  tcp::socket sock;
  enum { max_length = 65536 };
  unsigned char data[max_length];

public:

ConnectionHandler(boost::asio::io_service& io_service): sock(io_service){

}

typedef boost::shared_ptr<ConnectionHandler> ptr;

  virtual void on_recv(CommunicationDefinitions::TYPE type) = 0;
  virtual void on_close() = 0;

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
      //std::cout << "Handle Read Header" << (int)data[0] << std::endl;
         CommunicationDefinitions::TYPE type = (CommunicationDefinitions::TYPE)data[0];
         auto c = CommunicationDefinitions();
         if(c.PACKET_SIZES.find(type) == c.PACKET_SIZES.end()){
           //std::cout << "Invalid type" << (int)data[0] << std::endl;
           start_read();
           return;
         }

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
         on_close();
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
         on_close();
    }
  }

  void handle_write(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
      //std::cout << "Send to client"<< std::endl;
    } else {
       std::cerr << "error: " << err.message() << std::endl;
       sock.close();
       on_close();
    }
  }

  void write(unsigned char* data, int size){
    std::cout << "Sending Type: " << (int)data[0] << std::endl;
    boost::system::error_code err;
    int transferred = sock.write_some(boost::asio::buffer(data, size), err);
    handle_write(err, transferred);
  }
};