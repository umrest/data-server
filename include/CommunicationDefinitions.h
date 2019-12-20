#pragma once

#include <map>


class CommunicationDefinitions
{
    public:
        
        enum class TYPE
        {
            JOYSTICK = 1,
            VISION = 2,
            REALSENSE = 3,
            DATAAGGREGATOR_STATE = 8,
            DASHBOARD_DATA = 9,
            
            ROBOT_STATE = 10,

            VISION_COMMAND = 12,
            VISION_IMAGE = 13,
            INDENTIFIER = 250

        };

        enum class IDENTIFIER
        {
            DASHBOARD = 1,
            VISION = 2,
            TCPSERIAL = 3,
            HERO = 4
        };

         const std::map<TYPE, int> PACKET_SIZES = {
            {TYPE::JOYSTICK,  127},
            {TYPE::VISION,   127},
            {TYPE::REALSENSE,  127},
            {TYPE::DATAAGGREGATOR_STATE,  127},
            {TYPE::DASHBOARD_DATA,  127},
            {TYPE::ROBOT_STATE,  127},

            {TYPE::VISION_COMMAND,  127},
            {TYPE::VISION_IMAGE,  65535},
            {TYPE::INDENTIFIER,  127},
        };

        CommunicationDefinitions(){
            
        }

        
    
};