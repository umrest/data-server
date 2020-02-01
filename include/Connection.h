
#pragma once

#include <set>

#include "ConnectionHandler.h"

#include <comm/BitArray8.h>

#include <mutex>

#include <chrono>

class Network{
    private:
       
       boost::asio::deadline_timer timer;
       std::chrono::time_point<std::chrono::high_resolution_clock> sent_connection_status;
    public:
    Network(boost::asio::io_service& io_service) : timer(io_service, boost::posix_time::seconds(1)){
        start_timer();
  
    }

    void start_timer(){
        timer.expires_from_now(boost::posix_time::seconds(1));
        timer.async_wait(boost::bind(&Network::timer_tick, this, _1));
    }

    void timer_tick(const boost::system::error_code &err){
        send_connection_status();

        start_timer();
    }

    void add(){

    }

    void send_to_dashboard(unsigned char* data, int size){
        connect_lock.lock();
        for(auto &d : dashboard){
            if(d.get() != nullptr){
                d.get()->write(data, size);
            }
        }
        connect_lock.unlock();
            
    }

    void send_to_hero(unsigned char* data, int size){
        connect_lock.lock();
if(tcpserial.get() != nullptr){
                 tcpserial.get()->write(data, size);
             }
             connect_lock.unlock();
    }

     void send_to_vision(unsigned char* data, int size){
        connect_lock.lock();
        if(vision.get() != nullptr){
            vision.get()->write(data, size);
        }
        connect_lock.unlock();
    }

    void send_to_datasaver(unsigned char* data, int size){
        connect_lock.lock();
        if(datasaver.get() != nullptr){
            datasaver.get()->write(data, size);
        }
        connect_lock.unlock();
    }

    void send_connection_status(){

        BitArray8 status;
        auto now = std::chrono::high_resolution_clock::now();
        // if we have recieved a message in the last 5 seconds, the hero is "connected"
        bool hero_connected = std::chrono::duration_cast<std::chrono::seconds>(now - tcpserial_message_recieved).count() < 1;
        status.SetBit(0, hero_connected);
        status.SetBit(1, (bool)vision);
        status.SetBit(2, dashboard.size() > 0);
        status.SetBit(3, (bool)realsense);
        status.SetBit(4, (bool)tcpserial);

        unsigned char buf[131];
        buf[0] = 44;
        buf[1] = 254;
        buf[2] = 153;
        buf[3] = (int)CommunicationDefinitions::TYPE::DATAAGGREGATOR_STATE;
        buf[4] = status.aByte;
        

//        send_to_hero(buf, 131);
        send_to_dashboard(buf, 131);
    }

    std::set<ConnectionHandler::ptr> dashboard;
    ConnectionHandler::ptr tcpserial;
    ConnectionHandler::ptr vision;
    ConnectionHandler::ptr realsense;
    ConnectionHandler::ptr datasaver;
    
    // Last Message Recieved
    std::chrono::time_point<std::chrono::high_resolution_clock> dashboard_message_recieved;
    std::chrono::time_point<std::chrono::high_resolution_clock> tcpserial_message_recieved;
    std::chrono::time_point<std::chrono::high_resolution_clock> vision_message_recieved;
    std::chrono::time_point<std::chrono::high_resolution_clock> realsense_message_recieved;
    std::chrono::time_point<std::chrono::high_resolution_clock> dataserver_message_recieved;

    // prevent sending on a disconnected or connecting connection (which causes segfault)
    std::mutex connect_lock;

};

class Connection : public ConnectionHandler{
    public:
    Network& network;
    CommunicationDefinitions::IDENTIFIER identifier;
    void on_close(){
        network.connect_lock.lock();
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
            network.tcpserial.reset();
        }
        if(identifier == CommunicationDefinitions::IDENTIFIER::DATASAVER){
            
            network.datasaver.reset();
        }

        network.connect_lock.unlock();

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
            network.tcpserial = this->shared_from_this();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::DATASAVER){
            network.datasaver = this->shared_from_this();
        }

        network.send_connection_status();
    }

    void update_recieved_timestamp(){
        if(identifier == CommunicationDefinitions::IDENTIFIER::DASHBOARD){
            network.dashboard_message_recieved = std::chrono::high_resolution_clock::now();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::VISION){
            network.vision_message_recieved = std::chrono::high_resolution_clock::now();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::TCPSERIAL){
            network.tcpserial_message_recieved = std::chrono::high_resolution_clock::now();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::DATASAVER){
            network.dataserver_message_recieved = std::chrono::high_resolution_clock::now();
        }
    }



    

    void on_recv(CommunicationDefinitions::TYPE type){
        update_recieved_timestamp();

        

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
             
             network.send_to_dashboard(data, size);
         }

         else if (type == CommunicationDefinitions::TYPE::SENSOR_STATE){
             network.send_to_dashboard(data, size);
             network.send_to_datasaver(data, size);
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


