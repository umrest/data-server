
#pragma once

#include <set>

#include "ConnectionHandler.h"

#include <comm/BitArray8.h>
#include <comm/Data_Server.h>

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

    void send_to_hardware(unsigned char* data, int size){
        connect_lock.lock();
        if(hardware.get() != nullptr){
            hardware.get()->write(data, size);
        }
        connect_lock.unlock();
    }

    void send_connection_status()
    {
        comm::Data_Server data_server;

        auto now = std::chrono::high_resolution_clock::now();
        // if we have recieved a message in the last 5 seconds, the hero is "connected"
        bool hero_connected = std::chrono::duration_cast<std::chrono::seconds>(now - tcpserial_message_recieved).count() < 1;
        data_server._connected_status.SetBit(0, hero_connected);
        data_server._connected_status.SetBit(1, (bool)vision);
        data_server._connected_status.SetBit(2, dashboard.size() > 0);
        data_server._connected_status.SetBit(3, (bool)realsense);
        data_server._connected_status.SetBit(4, (bool)tcpserial);
        data_server._connected_status.SetBit(5, (bool)hardware);
        

        //send_to_hero(comm::CommunicationDefinitions::key, 3);
        //        send_to_hero(buf, 131);

        send_to_dashboard(comm::CommunicationDefinitions::key, 3);
        
        uint8_t type = (uint8_t)(data_server.type());
        send_to_dashboard(&type, 1);

        auto data = data_server.Serialize();
        send_to_dashboard(&data[0], data.size());
    }

     std::set<ConnectionHandler::ptr> dashboard;
    ConnectionHandler::ptr tcpserial;
    ConnectionHandler::ptr vision;
    ConnectionHandler::ptr realsense;
    ConnectionHandler::ptr datasaver;
    ConnectionHandler::ptr hardware;
    
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
            if (it != network.dashboard.end())
            {
                network.dashboard.erase(it);
            }
        }
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::VISION)
        {
            network.vision.reset();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::TCPSERIAL){
            network.tcpserial.reset();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::REALSENSE){
            network.realsense.reset();
        }
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::DATASAVER)
        {

            network.datasaver.reset();
        }
        
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::HARDWARE)
        {

            network.hardware.reset();
        }

        network.connect_lock.unlock();

        network.send_connection_status();

        key_pos = 0;

        
    }

    void on_identifier()
    {
        if (identifier == comm::CommunicationDefinitions::IDENTIFIER::DASHBOARD)
        {
           network.dashboard.insert(this->shared_from_this());
        }
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::VISION)
        {
            network.vision = this->shared_from_this();
        }
        else if(identifier == CommunicationDefinitions::IDENTIFIER::TCPSERIAL){
            network.tcpserial = this->shared_from_this();
        }
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::DATASAVER)
        {
            network.datasaver = this->shared_from_this();
        }
         else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::REALSENSE)
        {
            network.realsense = this->shared_from_this();
        }
         else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::HARDWARE)
        {
            network.hardware = this->shared_from_this();
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
        else if(identifier == CommunicationDefinitions::IDENTIFIER::REALSENSE){
            network.realsense_message_recieved = std::chrono::high_resolution_clock::now();
        }
    }



    

    void on_recv(CommunicationDefinitions::TYPE type){
        update_recieved_timestamp();

        

        std::cout << "Recieved Type: " <<(int) type << std::endl;
        int size = CommunicationDefinitions::PACKET_SIZES.at(type) + 4;
        

        // Identifier
        if (type == comm::CommunicationDefinitions::TYPE::IDENTIFIER)
        {
            comm::CommunicationDefinitions::IDENTIFIER id = (comm::CommunicationDefinitions::IDENTIFIER)data[4];

            std::cout << "Recieved Identifier: " << (int)id << std::endl;

            identifier = id;
            on_identifier();
        }

        // Data sent to hero
        else if (type == comm::CommunicationDefinitions::TYPE::JOYSTICK || type == comm::CommunicationDefinitions::TYPE::DASHBOARD)
        {
            network.send_to_hero(data, size);
        }

        // Data sent to hero and dashboard
        else if (type == comm::CommunicationDefinitions::TYPE::VISION)
        {
            network.send_to_hero(data, size);

            network.send_to_dashboard(data, size);
        }
        // data sent to dashboard only
        else if (type == comm::CommunicationDefinitions::TYPE::DATA_SERVER || type == comm::CommunicationDefinitions::TYPE::VISION_IMAGE || type == comm::CommunicationDefinitions::TYPE::REALSENSE)
        {
            network.send_to_dashboard(data, size);
        }

    
         else if (type == CommunicationDefinitions::TYPE::SENSOR_STATE){
             network.send_to_dashboard(data, size);
             network.send_to_datasaver(data, size);
         }

         else if (type == CommunicationDefinitions::TYPE::VISION_COMMAND || type == CommunicationDefinitions::TYPE::VISION_PROPERTIES){
             network.send_to_vision(data, size);
         }
         else if (type == CommunicationDefinitions::TYPE::HARDWARE){
             network.send_to_hardware(data,size);
         }
    }

    // creating the pointer
    static ptr create(boost::asio::io_service &io_service, Network &network)
    {
        return ptr(new Connection(io_service, network));
    }

    Connection(boost::asio::io_service &io_service, Network &network_in)
        : network(network_in), ConnectionHandler(io_service)
    {
    }
};
