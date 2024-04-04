//
// Created by Serge on 08.03.2024.
//

#ifndef RABBIT_CONNECTION_H
#define RABBIT_CONNECTION_H


#include <queue>
#include <unordered_map>
#include <condition_variable>
#include <mutex>
#include "protocol/STIP.h"
#include "utils/Timer.h"
#include "protocol/Connection.h"
#include "protocol/STIP.h"
#include "protocol/Session.h"


using boost::asio::ip::udp;

// TODO: Add to connection check live connection with ping thread

namespace STIP {

    class Connection {
    private:

        std::mutex mtx;
        std::condition_variable cv;
        Timer timeoutTimer;
        std::thread mainThread;
        udp::endpoint endpoint;
        udp::socket *socket = nullptr;
        char connectionStatus = 100;
        std::queue<STIP_PACKET> packetQueue;
        SessionManager *sessionManager = nullptr;
        bool isRunning = false;

        /// \brief Обработка пакетов
        ///
        /// Обработка пакетов из очереди
        ///
        void processThread();

        // message
        std::queue<ReceiveMessageSession *> messageQueue;
        std::mutex messageMtx;
        std::condition_variable messageCv;

    public:
        /// \brief Connection конструктор
        ///
        /// Конструктор класса Connection
        /// Создает объект Connection
        ///
        /// \param endpoint - адрес и порт подключения
        /// \param socket
        Connection(udp::endpoint &endpoint, udp::socket *socket);

        /// \brief Отправка пинга(получение версии протокола)
        ///
        /// Создает сессию отправки и обработки пинга PingSession
        /// Добавляет объект PingSession в SessionManager
        /// Отправляет пинг
        /// Ждет ответ
        /// Удаляет объект PingSession из SessionManager
        ///
        /// \return версия протокола
        uint32_t pingVersion();

        /// \brief Отправка сообщения
        ///
        /// Создает сессию отправки сообщения SendMessageSession
        /// Добавляет объект SendMessageSession в SessionManager
        /// Отправляет сообщение
        /// Ждет подтверждения
        /// Удаляет объект SendMessageSession из SessionManager
        ///
        /// \param data - указатель на данные
        /// \param size - размер данных в байтах
        /// \return результат отправки
        bool sendMessage(void *data, size_t size);

        /// \brief Отправка сообщения
        ///
        /// Вызывает функцию sendMessage с параметрами message.c_str() и message.size()
        ///
        /// \param message - текстовое сообщение
        /// \return результат отправки
        bool sendMessage(const std::string &message);

        ReceiveMessageSession *receiveMessage();

        /// \brief Добавление пакета в очередь обработки
        ///
        /// Добавляет пакет в очередь для данного соединения
        /// После добавления пакета в очередь, уведомляет поток обработки пакетов
        ///
        /// \param
        void addPacket(const STIP_PACKET &packet);

        /// \brief Получение пакета из очереди
        ///
        /// Получает пакет из очереди
        /// Если очередь пуста, ждет пока не появится пакет
        /// Возвращает пакет и результат выполнения
        ///
        /// \param result
        /// \return packet
        STIP_PACKET getPacket(bool &result);

        /// \brief Установка статуса соединения
        ///
        /// Устанавливает статус соединения
        ///
        /// \param status - статус соединения
        void setConnectionStatus(char status);

        void startProcessing();

        void stopProcessing();

        /// \brief Connection деструктор
        ///
        /// Деструктор класса Connection
        /// Останавливает поток обработки пакетов
        /// Удаляет объект SessionManager
        ///
        ~Connection();
    };


// TODO: Create connection killer thread
    class ConnectionManager {
    private:
        std::unordered_map<udp::endpoint, Connection *> connections;
        std::mutex mtx;
        const udp::socket *socket;

    public:
        explicit ConnectionManager(const udp::socket &socket);


        void accept(const udp::endpoint &endpoint, const STIP_PACKET &packet);

        void addConnection(const udp::endpoint &endpoint, Connection *connection);

        bool check(const udp::endpoint &endpoint);

        Connection *getConnection(const udp::endpoint &endpoint);

        void remove(const udp::endpoint &endpoint);

        ~ConnectionManager();
    };

}


#endif //RABBIT_CONNECTION_H
