#include "STUP.h"
#include <stdexcept>

// Создание STUP-пакета
STUPPacket create_packet(
    const std::vector<unsigned char>& dpa,
    unsigned char dla,
    unsigned char pid,
    unsigned char sla,
    const std::vector<unsigned char>& data) {

    STUPPacket packet;
    packet.dpa = dpa;
    packet.dla = dla;
    packet.pid = pid;
    packet.sla = sla;
    packet.data = data;
    return packet;
}

// Сериализация STUP-пакета
std::vector<unsigned char> serialize(const STUPPacket& packet) {
    std::vector<unsigned char> result;

    // DPA (если есть)
    result.insert(result.end(), packet.dpa.begin(), packet.dpa.end());

    // Основные поля
    result.push_back(packet.dla);
    result.push_back(packet.pid);
    result.push_back(packet.sla);

    // Данные
    result.insert(result.end(), packet.data.begin(), packet.data.end());

    return result;
}

//// Десериализация STUP-пакета
STUPPacket deserialize_STUP(const std::vector<unsigned char>& data) {
    if (data.size() < 3) {
        throw std::runtime_error("Неверный STUP-пакет: слишком короткий");
    }

    STUPPacket packet;
    size_t pos = 0;


    packet.dpa = {};////

    packet.dla = data[pos++];
    packet.pid = data[pos++];
    packet.sla = data[pos++];

    // Оставшиеся данные
    if (pos < data.size()) {
        packet.data.assign(data.begin() + pos, data.end());
    }

    return packet;
}