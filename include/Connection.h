#pragma once

#include <set>

#include "ConnectionHandler.h"


class Network{
    public:
    Network(){
        
  
    }

    void add(){

    }

    void send_to_dashboard(unsigned char* data, int size){
            if(dashboard.get() != nullptr){
                 dashboard.get()->write(data, size);
             }
    }

    void send_to_hero(unsigned char* data, int size){
if(hero.get() != nullptr){
                 hero.get()->write(data, size);
             }
    }

     void send_to_vision(unsigned char* data, int size){
if(vision.get() != nullptr){
                 vision.get()->write(data, size);
             }
    }

    ConnectionHandler::ptr dashboard;
    ConnectionHandler::ptr hero;
    ConnectionHandler::ptr vision;
    ConnectionHandler::ptr realsense;

};

class Connection : public ConnectionHandler{
    public:
    Network& network;

    

    void on_recv(CommunicationDefinitions::TYPE type){
        std::cout << "Recieved Type: " <<(int) type << std::endl;
        auto c = CommunicationDefinitions();
        int size = c.PACKET_SIZES.at(type);

        // Identifier
        if(type == CommunicationDefinitions::TYPE::INDENTIFIER){
            CommunicationDefinitions::IDENTIFIER identifier = (CommunicationDefinitions::IDENTIFIER)data[1];

            std::cout << "Recieved Identifier: " << (int)identifier << std::endl;
            if(identifier == CommunicationDefinitions::IDENTIFIER::DASHBOARD){
                network.dashboard = this->shared_from_this();
            }
             else if(identifier == CommunicationDefinitions::IDENTIFIER::VISION){
                network.vision = this->shared_from_this();
             }
             else if(identifier == CommunicationDefinitions::IDENTIFIER::TCPSERIAL){
                 network.hero = this->shared_from_this();
             }
             
         }

         // Data sent to hero
         else if(type == CommunicationDefinitions::TYPE::JOYSTICK || type == CommunicationDefinitions::TYPE::DASHBOARD_DATA){
             network.send_to_hero(data, size);
         }

         // Data sent to hero and dashboard
         else if(type == CommunicationDefinitions::TYPE::VISION){
             network.send_to_hero(data, size);

             network.send_to_dashboard(data, size);
             
         }
         // data sent to dashboard only
         else if (type == CommunicationDefinitions::TYPE::DATAAGGREGATOR_STATE){
            network.send_to_dashboard(data, size);
         }
    }

    // creating the pointer
    static ptr create(boost::asio::io_service& io_service, Network& network)
    {
        return ptr(new Connection(io_service, network));
    }

    Connection(boost::asio::io_service& io_service, Network& network_in)
    : network(network_in), ConnectionHandler(io_service){

    }
};


