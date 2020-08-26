#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <iostream>

#include <comm/CommunicationDefinitions.h>


using boost::asio::ip::tcp;
using comm::CommunicationDefinitions;

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

  virtual void on_recv(comm::CommunicationDefinitions::TYPE type) = 0;
  virtual void on_close() = 0;

//socket creation

  tcp::socket& socket()
  {
    return sock;
  }

  // RESTART read process
  void start_read(){
    key_pos = 0;
    start_read_key();
  }
  //read key values
    void start_read_key(){
        // read the header
        sock.async_read_some(
            boost::asio::buffer(data + key_pos, 1),
            boost::bind(&ConnectionHandler::handle_read_key,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

  // read header values
    void start_read_header(){
      sock.async_read_some(
            boost::asio::buffer(data + 3, 1),
            boost::bind(&ConnectionHandler::handle_read_header,
                        shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }
  void start()
  {
    boost::asio::socket_base::send_buffer_size option(128);
      sock.set_option(option);
    start_read();
  }

  int key_pos = 0;
  
  void handle_read_key(const boost::system::error_code& err, size_t bytes_transferred){
    if (!err) {
      //std::cout << key_pos << " " << (int)data[key_pos] << std::endl;
      if(comm::CommunicationDefinitions::key[key_pos] == data[key_pos]){
        key_pos++;
      }
      else{
        std::cout << "Invalid Key..." << (int)data[0] << " " << (int)data[1] << " " << (int)data[2] << std::endl;
        start_read();
        return;
      }
      if(key_pos >= 3){
        //std::cout << "Valid Key..." << std::endl;
        start_read_header();
      }
      else{
        start_read_key();
      }
    }

    else {
         std::cerr << "error: " << err.message() << std::endl;
         sock.close();
         on_close();
    }

  }

  void handle_read_header(const boost::system::error_code& err, size_t bytes_transferred)
  {
    if (!err) {
      //std::cout << "Handle Read Header" << (int)data[0] << std::endl;
         comm::CommunicationDefinitions::TYPE type = (comm::CommunicationDefinitions::TYPE)data[3];
         auto c = comm::CommunicationDefinitions();
         if(c.PACKET_SIZES.find(type) == c.PACKET_SIZES.end()){
           std::cout << "Invalid type" << (int)data[0] << std::endl;
           start_read();
           return;
         }

         int size = c.PACKET_SIZES.at(type);

         sock.async_read_some(
            boost::asio::buffer(data + 4, size),
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
         comm::CommunicationDefinitions::TYPE type = (comm::CommunicationDefinitions::TYPE)data[3];
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
    //std::cout << "Sending Type: " << (int)data[3] << std::endl;
    boost::system::error_code err;

      boost::asio::const_buffer d = boost::asio::buffer(data, size);

    sock.async_write_some( d, boost::bind(&ConnectionHandler::handle_write,
                        shared_from_this() ,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred) );
  }
};