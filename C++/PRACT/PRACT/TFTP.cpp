#include "TFTP.h"
#include <stdexcept>
#include <algorithm>
#include <cctype> 

// �������� ������� �� ������
RRQ_WRQ create_RRQ(const std::string& name, Mode mode) {
    RRQ_WRQ packet;
    packet.opcode = RRQ;
    packet.filename = name;
    packet.mode = mode;
    return packet;
}

// �������� ������� �� ������
RRQ_WRQ create_WRQ(const std::string& name, Mode mode) {
    RRQ_WRQ packet;
    packet.opcode = WRQ;
    packet.filename = name;
    packet.mode = mode;
    return packet;
}

// �������� ������ ������
Data create_DATA(unsigned short block, const std::vector<unsigned char>& data) {
    Data packet;
    packet.block_number = block;
    packet.data = data;
    return packet;
}

// �������� ������ �������������
ACKPacket create_ACKPacket(unsigned short block) {
    ACKPacket packet;
    packet.block_number = block;
    return packet;
}

// �������� ������ ������
ERRORPacket create_ERRORPacket(ErrorCode code, const std::string& message) {
    ERRORPacket packet;
    packet.error_code = static_cast<unsigned short>(code);
    packet.error_message = message;
    return packet;
}

// ������������ RRQ/WRQ
std::vector<unsigned char> serialize(const RRQ_WRQ& packet) {
    std::vector<unsigned char> result;

    // ����� (2 �����)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(packet.opcode));

    // ��� �����
    for (char c : packet.filename) {
        result.push_back(static_cast<unsigned char>(c));
    }
    result.push_back(0); // ����� ������

    // ����� ��������
    std::string mode_str = (packet.mode == octet) ? "octet" : "netascii";
    for (char c : mode_str) {
        result.push_back(static_cast<unsigned char>(c));
    }
    result.push_back(0); // ����� ������

    return result;
}

// ������������ DATA
std::vector<unsigned char> serialize(const Data& packet) {
    std::vector<unsigned char> result;

    // ����� (2 �����)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(DATA));

    // ����� ����� (2 �����)
    result.push_back(static_cast<unsigned char>(packet.block_number >> 8));
    result.push_back(static_cast<unsigned char>(packet.block_number & 0xFF));

    // ������
    result.insert(result.end(), packet.data.begin(), packet.data.end());

    return result;
}

// ������������ ACKPacket
std::vector<unsigned char> serialize(const ACKPacket& packet) {
    std::vector<unsigned char> result;

    // ����� (2 �����)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(ACK));

    // ����� ����� (2 �����)
    result.push_back(static_cast<unsigned char>(packet.block_number >> 8));
    result.push_back(static_cast<unsigned char>(packet.block_number & 0xFF));

    return result;
}

// ������������ ERRORPacket
std::vector<unsigned char> serialize(const ERRORPacket& packet) {
    std::vector<unsigned char> result;

    // ����� (2 �����)
    result.push_back(0);
    result.push_back(static_cast<unsigned char>(ERROR));

    // ��� ������ (2 �����)
    result.push_back(static_cast<unsigned char>(packet.error_code >> 8));
    result.push_back(static_cast<unsigned char>(packet.error_code & 0xFF));

    // ��������� �� ������
    for (char c : packet.error_message) {
        result.push_back(static_cast<unsigned char>(c));
    }
    result.push_back(0); // ����� ������

    return result;
}

//// �������������� RRQ/WRQ ������
RRQ_WRQ deserialize_RRQ_WRQ(const std::vector<unsigned char>& data) {
    if (data.size() < 6) { // ����������� ������: 2(opcode) + 1(���) + 1(0) + 1(�����) + 1(0)
        throw std::runtime_error("������ RRQ/WRQ packet: ������� ��������");
    }

    // �������� opcode
    if (data[0] != 0 || (data[1] != RRQ && data[1] != WRQ)) {
        throw std::runtime_error("�������� opcode ��� RRQ/WRQ");
    }

    RRQ_WRQ packet;
    packet.opcode = static_cast<Opcode>(data[1]);

    // ���������� ����� �����
    size_t pos = 2;
    while (pos < data.size() && data[pos] != 0) {
        packet.filename += static_cast<char>(data[pos++]);
    }

    if (pos >= data.size() || data[pos] != 0) {
        throw std::runtime_error("��� ����� ����������� ���������");
    }
    pos++; // ������� �������� �����

    // ���������� ������ ��������
    std::string mode_str;
    while (pos < data.size() && data[pos] != 0) {
        mode_str += static_cast<char>(data[pos++]);
    }

    if (pos >= data.size() || data[pos] != 0) {
        throw std::runtime_error("����� �������� ����������� ��������");
    }

    // �������������� ������ ������ � enum
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(),
        [](unsigned char c) { return std::tolower(c); });

    if (mode_str == "netascii") {
        packet.mode = netascii;
    }
    else if (mode_str == "octet") {
        packet.mode = octet;
    }
    else {
        throw std::runtime_error("����������� ����� ��������: " + mode_str);
    }

    return packet;
}

//// �������������� DATA ������
Data deserialize_DATA(const std::vector<unsigned char>& data) {
    if (data.size() < 4) {
        throw std::runtime_error("�������� DATA �����: ������� ��������");
    }

    // �������� opcode
    if (data[0] != 0 || data[1] != DATA) {
        throw std::runtime_error("�������� ����� ��� DATA");
    }

    Data packet;
    // ������ ������ ����� �� ���� ���� (big-endian)
    packet.block_number = (static_cast<unsigned short>(data[2]) << 8) | data[3];

    // ����������� ������
    if (data.size() > 4) {
        packet.data.assign(data.begin() + 4, data.end());
    }

    return packet;
}

//// �������������� ACK ������
ACKPacket deserialize_ACK(const std::vector<unsigned char>& data) {
    if (data.size() != 4) {
        throw std::runtime_error("�������� ACK �����: ������ ���� 4 �����");
    }

    // �������� opcode
    if (data[0] != 0 || data[1] != ACK) {
        throw std::runtime_error("�������� ����� ��� ACK");
    }

    ACKPacket packet;
    // ������ ������ ����� �� ���� ���� (big-endian)
    packet.block_number = (static_cast<unsigned short>(data[2]) << 8) | data[3];

    return packet;
}

//// �������������� ERROR ������
ERRORPacket deserialize_ERROR(const std::vector<unsigned char>& data) {
    if (data.size() < 5) { // ����������� ������: 2(opcode) + 2(error) + 1(msg) + 1(0)
        throw std::runtime_error("�������� ERROR �����: ������� ��������");
    }

    // �������� opcode
    if (data[0] != 0 || data[1] != ERROR) {
        throw std::runtime_error("�������� ����� ��� ERROR");
    }

    ERRORPacket packet;
    // ������ ���� ������ �� ���� ���� (big-endian)
    packet.error_code = (static_cast<unsigned short>(data[2]) << 8) | data[3];

    // ���������� ��������� �� ������
    size_t pos = 4;
    while (pos < data.size() && data[pos] != 0) {
        packet.error_message += static_cast<char>(data[pos++]);
    }

    if (pos >= data.size() || data[pos] != 0) {
        throw std::runtime_error("��������� �� ������ ����������� ���������");
    }

    return packet;
}

// ������� ����������� ���� TFTP ������
Opcode determine_tftp_type(const std::vector<unsigned char>& data) {
    if (data.size() < 2) return ERROR;
    return static_cast<Opcode>(data[1]);
}