#ifndef STUP_H
#define STUP_H

#include <vector>
#include <string>

// Структура STUP-пакета
struct STUPPacket {
    std::vector<unsigned char> dpa; // Destination Path Address (опционально)
    unsigned char dla;              // Destination Logical Address
    unsigned char pid;              // Protocol Identifier
    unsigned char sla;              // Source Logical Address
    std::vector<unsigned char> data;
};

// Функция для создания STUP-пакета
STUPPacket create_packet(
    const std::vector<unsigned char>& dpa,
    unsigned char dla,
    unsigned char pid,
    unsigned char sla,
    const std::vector<unsigned char>& data);

// Функция для сериализации STUP-пакета
std::vector<unsigned char> serialize(const STUPPacket& packet);

// Функция десериализации STUP-пакета
STUPPacket deserialize_STUP(const std::vector<unsigned char>& data);

#endif // STUP_H