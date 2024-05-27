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
#include "protocol/SessionKiller.h"

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
        SessionKiller sessionKiller;
        bool isRunning = false;

        /// \brief Обработка пакетов
        ///
        /// Обработка пакетов из очереди
        ///
        void processThread();
      
        int countPacketWaiting = 0;
        bool cancelPacketWaitingFlag = false;

        // message
        std::queue<ReceiveMessageSession *> messageQueue;
        std::mutex messageMtx;
        std::condition_variable messageCv;

    public:
        /// \brief Connection конструктор
        ///
        /// Конструктор класса Connection
        ///
        /// \param endpoint - адрес и порт подключения
        /// \param socket
        Connection(udp::endpoint &endpoint, udp::socket *socket);

        /// \brief Отправка пинга(получение версии протокола)
        ///
        /// Создает сессию отправки и обработки пинга PingSession
        ///(добавляет объект PingSession в SessionManager).
        /// Отправляет пинг, ждет ответ, удаляет объект PingSession из SessionManager
        ///
        /// \return версия протокола
        uint32_t pingVersion();

        /// \brief Отправка сообщения
        ///
        /// Создает сессию отправки сообщения SendMessageSession
        ///(добавляет объект SendMessageSession в SessionManager).
        /// Отправляет сообщение, ждет подтверждения, удаляет объект SendMessageSession из SessionManager
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

        /// \brief Получение пакета из очереди
        ///
        /// Если очередь пуста, ждет пока не появится пакет.
        /// Возвращает первый пакет из очереди и удаляет его из очереди
        ///
        /// \return
        ReceiveMessageSession *receiveMessage();

        /// \brief Добавление пакета в очередь обработки
        ///
        /// Добавляет пакет в очередь для данного соединения.
        /// После добавления пакета в очередь, уведомляет поток обработки пакетов
        ///
        /// \param
        void addPacket(const STIP_PACKET &packet);

        /// \brief Получение пакета из очереди
        ///
        /// Если очередь пуста, ждет пока не появится пакет.
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
      
        void cancelPacketWaiting();

        /// \brief Запуск обработки пакетов
        ///
        /// Запускает поток processThread,
        /// помечает isRunning как true
        ///
        void startProcessing();

        /// \brief Остановка обработки пакетов
        ///
        /// Посылает сигнал на остановку,
        /// помечает флаг isRunning как false
        ///
        void stopProcessing();

        /// \brief Connection деструктор
        ///
        /// Останавливает поток обработки пакетов,
        /// удаляет объект SessionManager
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
        /// \brief Конструктор ConnectionManager
        ///
        /// Устанавливает ссылку на сокет
        ///
        /// \param socket - ссылка на сокет
        explicit ConnectionManager(const udp::socket &socket);

        /// \brief Обработка пакета
        ///
        /// Обработка пакетов из очереди.
        /// Устанавливает соединение для пакета
        ///
        /// \param endpoint - адрес и порт подключения
        /// \param packet - пакет
        void accept(const udp::endpoint &endpoint, const STIP_PACKET &packet);

        /// \brief Добавление соединения
        ///
        /// Добавляет соединение в список соединений
        ///
        /// \param endpoint - адрес и порт соединения
        /// \param connection - указатель на соединение
        void addConnection(const udp::endpoint &endpoint, Connection *connection);

        /// \brief Проверка наличия соединения
        ///
        /// Проверяет наличие соединения в списке соединений
        ///
        /// \param endpoint - адрес и порт соединения
        /// \return возвращает true если соединение есть в списке, иначе false
        bool check(const udp::endpoint &endpoint);

        /// \brief Получение соединения
        ///
        /// Получение соединения из списка соединений
        ///
        /// \param endpoint - адрес и порт соединения
        /// \return возвращает указатель на соединение, если соединение есть в списке, иначе nullptr
        Connection *getConnection(const udp::endpoint &endpoint);

        /// \brief Удаление соединения
        ///
        /// Удаляет соединение из списка соединений
        ///
        /// \param endpoint - адрес и порт соединения
        void remove(const udp::endpoint &endpoint);

        /// \brief Деструктор ConnectionManager
        ///
        /// Удаляет все соединения
        ///
        ~ConnectionManager();
    };

}


#endif //RABBIT_CONNECTION_H
