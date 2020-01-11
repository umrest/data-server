#pragma once

#include <set>

#include "ConnectionHandler.h"

#include <comm/BitArray8.h>

class Network{
    public:
    Network(){
        
  
    }

    void add(){

    }

    void send_to_dashboard(unsigned char* data, int size){
        for(auto &d : dashboard){
            if(d.get() != nullptr){
                d.get()->write(data, size);
            }
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

    void send_to_datasaver(unsigned char* data, int size){
        if(datasaver.get() != nullptr){
            datasaver.get()->write(data, size);
        }
    }

    void send_connection_status(){

        BitArray8 status;
        status.SetBit(0, (bool)hero);
        status.SetBit(1, (bool)vision);
        status.SetBit(2, dashboard.size() > 0);
        status.SetBit(3, (bool)realsense);

        unsigned char buf[131];
        buf[0] = 44;
        buf[1] = 254;
        buf[2] = 153;
        buf[3] = (int)CommunicationDefinitions::TYPE::DATAAGGREGATOR_STATE;
        buf[4] = status.aByte;
        

        send_to_hero(buf, 131);
        send_to_dashboard(buf, 131);
    }

    std::set<ConnectionHandler::ptr> dashboard;
    ConnectionHandler::ptr hero;
    ConnectionHandler::ptr vision;
    ConnectionHandler::ptr realsense;
    ConnectionHandler::ptr datasaver;

};

class Connection : public ConnectionHandler{
    public:
    Network& network;
    CommunicationDefinitions::IDENTIFIER identifier;
    void on_close(){
        if(identifier == CommunicationDefinitions::IDENTIFIER::DASHBOARD){
            auto it = network.dashboard.find(this->shared_from_this());
            if(it != network.dashboard.end()){
                network.dashboard.erase(it);
            }
           
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::VISION){
            network.vision.reset();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::TCPSERIAL){
            network.hero.reset();
        }
        if(identifier == CommunicationDefinitions::IDENTIFIER::DATASAVER){
            
            network.datasaver.reset();
        }

        network.send_connection_status();

        key_pos = 0;
    }

    void on_identifier(){
        if(identifier == CommunicationDefinitions::IDENTIFIER::DASHBOARD){
            network.dashboard.insert(this->shared_from_this());
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::VISION){
            network.vision = this->shared_from_this();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::TCPSERIAL){
            network.hero = this->shared_from_this();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::DATASAVER){
            network.datasaver = this->shared_from_this();
        }

        network.send_connection_status();
    }

    void on_recv(CommunicationDefinitions::TYPE type){
        //std::cout << "Recieved Type: " <<(int) type << std::endl;
        int size = CommunicationDefinitions::PACKET_SIZES.at(type) + 4;
        

        // Identifier
        if(type == CommunicationDefinitions::TYPE::IDENTIFIER){
            CommunicationDefinitions::IDENTIFIER id = (CommunicationDefinitions::IDENTIFIER)data[4];

            std::cout << "Recieved Identifier: " << (int)id << std::endl;

            identifier = id;
            on_identifier();

             
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
         else if (type == CommunicationDefinitions::TYPE::DATAAGGREGATOR_STATE || type == CommunicationDefinitions::TYPE::VISION_IMAGE) {
            network.send_to_dashboard(data, size);
         }

         else if (type == CommunicationDefinitions::TYPE::ROBOT_STATE){
             network.send_to_datasaver(data, size);
             network.send_to_dashboard(data, size);
         }

         else if (type == CommunicationDefinitions::TYPE::VISION_COMMAND || type == CommunicationDefinitions::TYPE::VISION_PROPERTIES){
             network.send_to_vision(data, size);
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


