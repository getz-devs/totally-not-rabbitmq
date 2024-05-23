//
// Created by Serge on 16.03.2024.
//

#ifndef RABBIT_SESSION_H
#define RABBIT_SESSION_H

#include <iostream>
#include <map>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "protocol/STIP.h"
#include "protocol/STIPVersion.h"
#include <vector>
//using namespace std;
using boost::asio::ip::udp;

namespace STIP {

    class Session {
    public:
        /// \brief Обработка входящего пакета
        ///
        /// Помечает, что пакет с данными получен
        ///
        /// \param packet пакет
        virtual void processIncomingPacket(STIP_PACKET packet) = 0;

        /// \brief Полуение идентификатора сессии
        ///
        /// \return возвращает идентификатор сессии
        uint32_t getId() const;

    protected:
        uint32_t id = 0;
        void *data = nullptr;
    };



// ------------------------------------------------ PingSession.h ------------------------------------------------

    class PingSession : public Session {
    public:
        PingSession() {
            std::cout << "PingSession created" << std::endl;
        }

        /// \brief Конструктор
        ///
        /// Создание сессии пинга
        ///
        /// \param id - идентификатор сессии
        explicit PingSession(uint32_t id);

        /// \brief Обработка входящего пакета ?
        ///
        /// Процесс обработки входящего пакетов
        ///
        /// \param packet - пакет
        void processIncomingPacket(STIP_PACKET packet) override;

        /// \brief Ответ сервера на пинг
        ///
        /// Устанавливает команду, идентификатор сессии, версию протокола и размер пакета
        ///
        /// \param socket - указатель на сокет
        /// \param endpoint - указатель на конечную точку
        /// \param sessionId - идентификатор сессии
        static void serverAnswer(udp::socket &socket, udp::endpoint &endpoint, uint32_t sessionId);

        /// \brief Деструктор
        ///
        ~PingSession();

        /// \brief Ожидание ответа
        ///
        /// Ожидание ответа от сервера
        ///
        /// \return возвращает ответ от сервера
        uint32_t waitAnswer();

    private:
        std::mutex mtx;
        std::condition_variable cv;
        bool isAnswered = false;
        uint32_t answer = 0;
    };


// ------------------------------------------------ SendMessageSession.h ------------------------------------------------
    // create enum for SendMessageStatuses;
    enum SendMessageStatuses : int {
//        CANCELED = 404,

        INIT = 0,
        INIT_REQUEST_SENT = 1,
        INIT_RESPONSE_SUCCESS = 2,
        INIT_RESPONSE_FAILURE = 3,

        DATA_REQEUST_SENT = 4,
        DATA_RESPONSE_RESEND = 5,
        DATA_RESPONSE_SUCCESS = 6,
        DATA_RESPONSE_FAILURE = 7,
    };


    class SendMessageSession : public Session {
    public:
        /// \brief Конструктор
        ///
        /// Создание сессии отправки сообщения
        ///
        /// \param id - идентификатор сессии
        /// \param data - данные
        /// \param size - размер данных
        /// \param socket - указатель на сокет
        /// \param endpoint - указатель на конечную точку
        explicit SendMessageSession(uint32_t id, void *data, uint32_t size, udp::socket *socket,
                                    udp::endpoint &endpoint) {
            status = SendMessageStatuses::INIT; // -1
            this->id = id;
            this->data = data;
            this->size = size;

            this->socket = socket;
            this->endpoint = endpoint;

            packet_counts = size / MAX_STIP_DATA_SIZE + 1;
        }

        /// \brief Обработка входящего пакета ?
        ///
        /// Процесс обработки входящего пакетов
        ///
        /// \param packet - пакет
        void processIncomingPacket(STIP_PACKET packet) override;

//    void sendAnswer(udp::socket &socket, udp::endpoint &endpoint, void *data, size_t size);
        /// \brief Инициализация отправки
        ///
        /// Инициализация отправки сообщения
        /// Установка команды, идентификатора сессии, идентификатора пакета и размера пакета.
        /// Копирование данных в пакет, отправка пакета, ожидание ответа
        ///
        /// \return возвращает true
        bool initSend();

        /// \brief Инициализация отправки с таймаутом
        ///
        /// \param timeout_result - сообщает программе, что произошел таймаут
        /// \param timeout - время таймаута
        /// \return возвращает true, если сообщение отправлено. Или false, если сервер отменил прием
        bool initSendWrappedTimout(bool &timeout_result, int timeout, int retry_count);

        /// \brief Отправка данных
        ///
        /// Отправка данных по частям
        ///
        /// \return возвращает true
        bool sendData();

        /// \brief Ожидание подтверждения с таймаутом. Или же произошла просьба перепосылки
        ///
        /// \param timeout_result
        /// \param timeout
        /// \param retry_count
        /// \return успех подтверждения
        bool waitApprovalWrappedTimout(bool &timeout_result, int timeout, int retry_count);

        /// \brief Ожидание подтверждения ?
        ///
        /// Ожидание подтверждения
        ///
        /// \return возвращает статус 5
        SendMessageStatuses waitApproval();

        void cancel();

    private:
        std::mutex mtx;
        std::condition_variable cv;
        bool _cancaled = false;

        void *data = nullptr;
        size_t size = 0;
        size_t packet_counts = 0;
        SendMessageStatuses status = SendMessageStatuses::INIT;

        udp::socket *socket = nullptr;
        udp::endpoint endpoint;

        /// \brief Отправка части данных
        ///
        /// Установка команды, идентификатора сессии и идентификатора части пакета.
        /// Копирование части данных в пакет, отправка пакета
        ///
        /// \param packet_id - идентификатор пакета
        void sendPart(uint32_t packet_id);

        void doResend(STIP_PACKET &packet);

        /// \brief Спрашиваем у сервера получил ли тот все пакеты
        void askAllReceived();
    };

// ------------------------------------------------ ReceiveMessageSession.h ------------------------------------------------

    class ReceiveMessageSession : public Session {
    public:
        explicit ReceiveMessageSession(uint32_t id, size_t size, size_t packet_counts, udp::socket *socket,
                                       udp::endpoint &endpoint) {
            status = -1;
            this->id = id;
            this->size = size;
            this->packet_counts = packet_counts;
            receivedParts.resize(packet_counts, false);
            data = malloc(size);

            this->socket = socket;
            this->endpoint = endpoint;
        }

        /// \brief Обработка входящего пакета ?
        ///
        /// Проверка команды пакета
        ///
        /// \param packet - пакет
        void processIncomingPacket(STIP_PACKET packet) override;

//    void sendAnswer(udp::socket &socket, udp::endpoint &endpoint, void *data, size_t size);

        void waitAprroval();

        /// \brief Получение статуса
        ///
        /// \return статус
        int getStatus() const;

        /// \brief Получение данных в виде строки
        ///
        /// \return строка данных
        std::string getDataAsString();
        bool dispatched = false;
    private:
        std::mutex mtx;
        std::condition_variable cv;
        bool isAnswered = false;
        void *answer = nullptr;
        size_t answerSize = 0;
        int status = -1;

        void *data = nullptr;
        size_t size = 0;
        size_t packet_counts = 0;

        std::vector<bool> receivedParts;

        udp::socket *socket = nullptr;
        udp::endpoint endpoint;

        uint32_t countUnreceivedParts() const;
    };



// ------------------------------------------------ SessionManager.h ------------------------------------------------

    class SessionManager {
    public:
        /// \brief Добавление сессии
        ///
        /// Добавление сессии в список сессий.
        /// Проверка наличия уже сессии с таким идентификатором
        ///
        /// \param session
        void addSession(Session *session);

        /// \brief Удаление сессии
        ///
        /// Удаление сессии из списка сессий
        ///
        /// \param session - сессия
        void deleteSession(Session *session);

        /// \brief Получение сессии
        ///
        /// Получение сессии по идентификатору.
        /// Проверка наличия сессии с таким идентификатором
        ///
        /// \param id - идентификатор сессии
        /// \return возвращает сессию
        Session *getSession(uint32_t id);
      
        void deleteSessionById(uint32_t id);

        /// \brief Генерация идентификатора сессии
        ///
        /// Генерация идентификатора сессии.
        /// Генерация случайного числа в диапазоне от 0 до 1000000.
        /// Проверка наличия сессии с таким идентификатором
        ///
        /// \return сгенерированный идентификатор сессии
        uint32_t generateSessionId();

    private:
        std::map<uint32_t, Session *> sessions;
        std::mutex mtx;
    };

}

#endif //RABBIT_SESSION_H
