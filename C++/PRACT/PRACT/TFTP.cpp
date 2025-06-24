#include "TFTP.h"
#include <stdexcept>
#include <algorithm>
#include <cctype> 

// Создание запроса на чтение
RRQ_WRQ create_RRQ(const std::string& name, Mode mode) {
    RRQ_WRQ packet;
    packet.opcode = RRQ;
    packet.filename = name;
    packet.mode = mode;
    return packet;
}

// Создание запроса на запись
RRQ_WRQ create_WRQ(const std::string& name, Mode mode) {
    RRQ_WRQ packet;
    packet.opcode = WRQ;
    packet.filename = name;
    packet.mode = mode;
    return packet;
}

// Создание пакета данных
Data create_DATA(unsigned short block, const std::vector<unsigned char>& data) {
    Data packet;
    packet.block_number = block;
    packet.data = data;
    return packet;
}

// Создание пакета подтверждения
ACKPacket create_ACKPacket(unsigned short block) {
    ACKPacket packet;
    packet.block_number = block;
    return packet;
}

// Создание пакета ошибки
ERRORPacket create_ERRORPacket(ErrorCode code, const std::string& message) {
    ERRORPacket packet;
    packet.error_code = static_cast<unsigned short>(code);
    packet.error_message = message;
    return packet;
}

// Сериализация RRQ/WRQ
std::vector<unsigned char> serialize(const RRQ_WRQ& packet) {
    std::vector<unsigned char> result;

    // Опкод (2 байта)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(packet.opcode));

    // Имя файла
    for (char c : packet.filename) {
        result.push_back(static_cast<unsigned char>(c));
    }
    result.push_back(0); // Конец строки

    // Режим передачи
    std::string mode_str = (packet.mode == octet) ? "octet" : "netascii";
    for (char c : mode_str) {
        result.push_back(static_cast<unsigned char>(c));
    }
    result.push_back(0); // Конец строки

    return result;
}

// Сериализация DATA
std::vector<unsigned char> serialize(const Data& packet) {
    std::vector<unsigned char> result;

    // Опкод (2 байта)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(DATA));

    // Номер блока (2 байта)
    result.push_back(static_cast<unsigned char>(packet.block_number >> 8));
    result.push_back(static_cast<unsigned char>(packet.block_number & 0xFF));

    // Данные
    result.insert(result.end(), packet.data.begin(), packet.data.end());

    return result;
}

// Сериализация ACKPacket
std::vector<unsigned char> serialize(const ACKPacket& packet) {
    std::vector<unsigned char> result;

    // Опкод (2 байта)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(ACK));

    // Номер блока (2 байта)
    result.push_back(static_cast<unsigned char>(packet.block_number >> 8));
    result.push_back(static_cast<unsigned char>(packet.block_number & 0xFF));

    return result;
}

// Сериализация ERRORPacket
std::vector<unsigned char> serialize(const ERRORPacket& packet) {
    std::vector<unsigned char> result;

    // Опкод (2 байта)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(ERROR));

    // Код ошибки (2 байта)
    result.push_back(static_cast<unsigned char>(packet.error_code >> 8));
    result.push_back(static_cast<unsigned char>(packet.error_code & 0xFF));

    // Сообщение об ошибке
    for (char c : packet.error_message) {
        result.push_back(static_cast<unsigned char>(c));
    }
    result.push_back(0); // Конец строки

    return result;
}

//// Десериализация RRQ/WRQ пакета
RRQ_WRQ deserialize_RRQ_WRQ(const std::vector<unsigned char>& data) {
    if (data.size() < 6) { // Минимальный размер: 2(opcode) + 1(имя) + 1(0) + 1(режим) + 1(0)
        throw std::runtime_error("Ошибка RRQ/WRQ packet: слишком короткий");
    }

    // Проверка opcode
    if (data[0] != 0 || (data[1] != RRQ && data[1] != WRQ)) {
        throw std::runtime_error("Неверный opcode для RRQ/WRQ");
    }

    RRQ_WRQ packet;
    packet.opcode = static_cast<Opcode>(data[1]);

    // Извлечение имени файла
    size_t pos = 2;
    while (pos < data.size() && data[pos] != 0) {
        packet.filename += static_cast<char>(data[pos++]);
    }

    if (pos >= data.size() || data[pos] != 0) {
        throw std::runtime_error("Имя файла некорректно завершено");
    }
    pos++; // Пропуск нулевого байта

    // Извлечение режима передачи
    std::string mode_str;
    while (pos < data.size() && data[pos] != 0) {
        mode_str += static_cast<char>(data[pos++]);
    }

    if (pos >= data.size() || data[pos] != 0) {
        throw std::runtime_error("Режим передачи некорректно завершен");
    }

    // Преобразование строки режима в enum
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (mode_str == "netascii") {
        packet.mode = netascii;
    }
    else if (mode_str == "octet") {
        packet.mode = octet;
    }
    else {
        throw std::runtime_error("Неизвестный режим передачи: " + mode_str);
    }

    return packet;
}

//// Десериализация DATA пакета
Data deserialize_DATA(const std::vector<unsigned char>& data) {
    if (data.size() < 4) {
        throw std::runtime_error("Неверный DATA пакет: слишком короткий");
    }

    // Проверка opcode
    if (data[0] != 0 || data[1] != DATA) {
        throw std::runtime_error("Неверный опкод для DATA");
    }

    Data packet;
    // Сборка номера блока из двух байт (big-endian)
    packet.block_number = (static_cast<unsigned short>(data[2]) << 8) | data[3];

    // Копирование данных
    if (data.size() > 4) {
        packet.data.assign(data.begin() + 4, data.end());
    }

    return packet;
}

//// Десериализация ACK пакета
ACKPacket deserialize_ACK(const std::vector<unsigned char>& data) {
    if (data.size() != 4) {
        throw std::runtime_error("Неверный ACK пакет: должен быть 4 байта");
    }

    // Проверка opcode
    if (data[0] != 0 || data[1] != ACK) {
        throw std::runtime_error("Неверный опкод для ACK");
    }

    ACKPacket packet;
    // Сборка номера блока из двух байт (big-endian)
    packet.block_number = (static_cast<unsigned short>(data[2]) << 8) | data[3];

    return packet;
}

//// Десериализация ERROR пакета
ERRORPacket deserialize_ERROR(const std::vector<unsigned char>& data) {
    if (data.size() < 5) { // Минимальный размер: 2(opcode) + 2(error) + 1(msg) + 1(0)
        throw std::runtime_error("Неверный ERROR пакет: слишком короткий");
    }

    // Проверка opcode
    if (data[0] != 0 || data[1] != ERROR) {
        throw std::runtime_error("Неверный опкод для ERROR");
    }

    ERRORPacket packet;
    // Сборка кода ошибки из двух байт (big-endian)
    packet.error_code = (static_cast<unsigned short>(data[2]) << 8) | data[3];

    // Извлечение сообщения об ошибке
    size_t pos = 4;
    while (pos < data.size() && data[pos] != 0) {
        packet.error_message += static_cast<char>(data[pos++]);
    }

    if (pos >= data.size() || data[pos] != 0) {
        throw std::runtime_error("Сообщение об ошибке некорректно завершено");
    }

    return packet;
}

// Функция определения типа TFTP пакета
Opcode determine_tftp_type(const std::vector<unsigned char>& data) {
    if (data.size() < 2) return ERROR;
    return static_cast<Opcode>(data[1]);
}