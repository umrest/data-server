#include "DataServer.h"



void DataServer::start_accept()
   {
    // socket
     ConnectionHandler::ptr connection = Connection::create(acceptor_.get_io_service(), network);

    // asynchronous accept operation and wait for a new connection.
     acceptor_.async_accept(connection->socket(),
        boost::bind(&DataServer::handle_accept, this, connection,
        boost::asio::placeholders::error));
  }

DataServer::DataServer(boost::asio::io_service& io_service) : acceptor_(io_service, tcp::endpoint(tcp::v4(), 8091)), network(io_service)
{
    start_accept();
}

  void DataServer:: handle_accept(ConnectionHandler::ptr connection, const boost::system::error_code& err)
  {
    if (!err) {
      connection->start();
      std::cout << "Got connection from " << connection->socket().remote_endpoint().address().to_string() << std::endl;
    }
    start_accept();
  }