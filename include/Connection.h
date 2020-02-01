#pragma once

#include <set>

#include "ConnectionHandler.h"

#include <comm/BitArray8.h>
#include <comm/DataServer.h>

class Network
{
public:
    Network()
    {
    }

    void add()
    {
    }

    void send_to_dashboard(unsigned char *data, int size)
    {
        for (auto &d : dashboard)
        {
            if (d.get() != nullptr)
            {
                d.get()->write(data, size);
            }
        }
    }

    void send_to_hero(unsigned char *data, int size)
    {
        if (hero.get() != nullptr)
        {
            hero.get()->write(data, size);
        }
    }

    void send_to_vision(unsigned char *data, int size)
    {
        if (vision.get() != nullptr)
        {
            vision.get()->write(data, size);
        }
    }

    void send_to_datasaver(unsigned char *data, int size)
    {
        if (datasaver.get() != nullptr)
        {
            datasaver.get()->write(data, size);
        }
    }

    void send_connection_status()
    {
        comm::DataServer data_server;

        data_server.connected_status.SetBit(0, (bool)hero);
        data_server.connected_status.SetBit(1, (bool)vision);
        data_server.connected_status.SetBit(2, dashboard.size() > 0);
        data_server.connected_status.SetBit(3, (bool)realsense);

        auto data = data_server.Serialize();

        send_to_hero(comm::CommunicationDefinitions::key, 3);
        send_to_dashboard(comm::CommunicationDefinitions::key, 3);

        send_to_hero(&data[0], data.size());
        send_to_dashboard(&data[0], data.size());
    }

    std::set<ConnectionHandler::ptr> dashboard;
    ConnectionHandler::ptr hero;
    ConnectionHandler::ptr vision;
    ConnectionHandler::ptr realsense;
    ConnectionHandler::ptr datasaver;
};

class Connection : public ConnectionHandler
{
public:
    Network &network;
    comm::CommunicationDefinitions::IDENTIFIER identifier;
    void on_close()
    {
        if (identifier == comm::CommunicationDefinitions::IDENTIFIER::DASHBOARD)
        {
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
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::TCPSERIAL)
        {
            network.hero.reset();
        }
        if (identifier == comm::CommunicationDefinitions::IDENTIFIER::DATASAVER)
        {

            network.datasaver.reset();
        }

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
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::TCPSERIAL)
        {
            network.hero = this->shared_from_this();
        }
        else if (identifier == comm::CommunicationDefinitions::IDENTIFIER::DATASAVER)
        {
            network.datasaver = this->shared_from_this();
        }

        network.send_connection_status();
    }

    void on_recv(comm::CommunicationDefinitions::TYPE type)
    {
        std::cout << "Recieved Type: " <<(int) type << std::endl;
        int size = comm::CommunicationDefinitions::PACKET_SIZES.at(type) + 4;

        // Identifier
        if (type == comm::CommunicationDefinitions::TYPE::IDENTIFIER)
        {
            comm::CommunicationDefinitions::IDENTIFIER id = (comm::CommunicationDefinitions::IDENTIFIER)data[4];

            std::cout << "Recieved Identifier: " << (int)id << std::endl;

            identifier = id;
            on_identifier();
        }

        // Data sent to hero
        else if (type == comm::CommunicationDefinitions::TYPE::JOYSTICK || type == comm::CommunicationDefinitions::TYPE::DASHBOARD_DATA)
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
        else if (type == comm::CommunicationDefinitions::TYPE::DATAAGGREGATOR_STATE || type == comm::CommunicationDefinitions::TYPE::VISION_IMAGE)
        {
            network.send_to_dashboard(data, size);
        }

        else if (type == comm::CommunicationDefinitions::TYPE::ROBOT_STATE)
        {
            network.send_to_datasaver(data, size);
            network.send_to_dashboard(data, size);
        }

        else if (type == comm::CommunicationDefinitions::TYPE::VISION_COMMAND || type == comm::CommunicationDefinitions::TYPE::VISION_PROPERTIES)
        {
            network.send_to_vision(data, size);
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
