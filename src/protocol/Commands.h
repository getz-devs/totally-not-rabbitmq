//
// Created by Serge on 12.04.2024.
//

#ifndef RABBIT_COMMANDS_H
#define RABBIT_COMMANDS_H
#include <ostream>

namespace STIP {
    enum class Command : int {
        MSG_INIT_REQUEST = 0,
        MSG_INIT_RESPONSE_SUCCESS = 1,
        MSG_INIT_RESPONSE_FAILURE = 2,

        MSG_SEND_DATA_PART = 3,

        MSG_REQUEST_ALL_RECEIVED = 4,
        MSG_RESPONSE_ALL_RECEIVED = 5,
        MSG_RESPONSE_RESEND = 6,

        MSG_KILLED = 7,


        PING_ASK = 10,
        PING_ANSWER = 11,


        CONNECTION_SYN = 100,
        CONNECTION_SYN_ACK = 101,
        CONNECTION_ACK = 102,
        CONNECTION_FIN = 103,
    };

    inline std::ostream& operator<<(std::ostream & os, const Command &command) {
        switch (command) {
            case Command::MSG_INIT_REQUEST:
                os << "MSG_INIT_REQUEST";
                break;
            case Command::MSG_INIT_RESPONSE_SUCCESS:
                os << "MSG_INIT_RESPONSE_SUCCESS";
                break;
            case Command::MSG_INIT_RESPONSE_FAILURE:
                os << "MSG_INIT_RESPONSE_FAILURE";
                break;
            case Command::MSG_SEND_DATA_PART:
                os << "MSG_SEND_DATA_PART";
                break;
            case Command::MSG_REQUEST_ALL_RECEIVED:
                os << "MSG_REQUEST_ALL_RECEIVED";
                break;
            case Command::MSG_RESPONSE_ALL_RECEIVED:
                os << "MSG_RESPONSE_ALL_RECEIVED";
                break;
            case Command::MSG_RESPONSE_RESEND:
                os << "MSG_RESPONSE_RESEND";
                break;
            case Command::MSG_KILLED:
                os << "MSG_KILLED";
                break;
            case Command::PING_ASK:
                os << "PING_ASK";
                break;
            case Command::PING_ANSWER:
                os << "PING_ANSWER";
                break;
            case Command::CONNECTION_SYN:
                os << "CONNECTION_SYN";
                break;
            case Command::CONNECTION_SYN_ACK:
                os << "CONNECTION_SYN_ACK";
                break;
            case Command::CONNECTION_ACK:
                os << "CONNECTION_ACK";
                break;
            case Command::CONNECTION_FIN:
                os << "CONNECTION_FIN";
                break;
        }
        os << " (" << static_cast<int>(command) << ")";
        return os;
    }
}

#endif //RABBIT_COMMANDS_H
