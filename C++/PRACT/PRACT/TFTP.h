#ifndef TFTP_H
#define TFTP_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept> // Для обработки ошибок

// Перечисление режимов передачи
enum Mode {
    netascii,
    octet
};

// Перечисление кодов операций TFTP
enum Opcode {
    RRQ = 1,
    WRQ = 2,
    DATA = 3,
    ACK = 4,
    ERROR = 5
};

// Перечисление кодов ошибок TFTP
enum ErrorCode {
    Undefined = 0,
    FileNotFound = 1,
    AccessViolation = 2,
    DiskFull = 3,
    IllegalOperation = 4,
    UnknownTransferID = 5,
    FileExists = 6,
    NoSuchUser = 7
};

// Структура пакета запроса чтения/записи
struct RRQ_WRQ {
    short opcode;    // 1 для RRQ, 2 для WRQ
    std::string filename;
    Mode mode;
};

// Структура пакета данных
struct Data {
    unsigned short block_number;
    std::vector<unsigned char> data;
};

// Структура пакета подтверждения
struct ACKPacket {
    unsigned short block_number;
};

// Структура пакета ошибки
struct ERRORPacket {
    unsigned short error_code;
    std::string error_message;
};

// Функции для создания пакетов
RRQ_WRQ create_RRQ(const std::string& name, Mode mode);
RRQ_WRQ create_WRQ(const std::string& name, Mode mode);
Data create_DATA(unsigned short block, const std::vector<unsigned char>& data);
ACKPacket create_ACKPacket(unsigned short block);
ERRORPacket create_ERRORPacket(ErrorCode code, const std::string& message);

// Функции сериализации
std::vector<unsigned char> serialize(const RRQ_WRQ& packet);
std::vector<unsigned char> serialize(const Data& packet);
std::vector<unsigned char> serialize(const ACKPacket& packet);
std::vector<unsigned char> serialize(const ERRORPacket& packet);

//// Функции десериализации
RRQ_WRQ deserialize_RRQ_WRQ(const std::vector<unsigned char>& data);
Data deserialize_DATA(const std::vector<unsigned char>& data);
ACKPacket deserialize_ACK(const std::vector<unsigned char>& data);
ERRORPacket deserialize_ERROR(const std::vector<unsigned char>& data);

Opcode determine_tftp_type(const std::vector<unsigned char>& data);

#endif // TFTP_H