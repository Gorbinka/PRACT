#ifndef STUP_H
#define STUP_H

#include <vector>
#include <string>

// ��������� STUP-������
struct STUPPacket {
    std::vector<unsigned char> dpa; // Destination Path Address (�����������)
    unsigned char dla;              // Destination Logical Address
    unsigned char pid;              // Protocol Identifier
    unsigned char sla;              // Source Logical Address
    std::vector<unsigned char> data;
};

// ������� ��� �������� STUP-������
STUPPacket create_packet(
    const std::vector<unsigned char>& dpa,
    unsigned char dla,
    unsigned char pid,
    unsigned char sla,
    const std::vector<unsigned char>& data);

// ������� ��� ������������ STUP-������
std::vector<unsigned char> serialize(const STUPPacket& packet);

// ������� �������������� STUP-������
STUPPacket deserialize_STUP(const std::vector<unsigned char>& data);

#endif // STUP_H