#ifndef TFTP_H
#define TFTP_H

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdexcept> // ��� ��������� ������

// ������������ ������� ��������
enum Mode {
    netascii,
    octet
};

// ������������ ����� �������� TFTP
enum Opcode {
    RRQ = 1,
    WRQ = 2,
    DATA = 3,
    ACK = 4,
    ERROR = 5
};

// ������������ ����� ������ TFTP
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

// ��������� ������ ������� ������/������
struct RRQ_WRQ {
    short opcode;    // 1 ��� RRQ, 2 ��� WRQ
    std::string filename;
    Mode mode;
};

// ��������� ������ ������
struct Data {
    unsigned short block_number;
    std::vector<unsigned char> data;
};

// ��������� ������ �������������
struct ACKPacket {
    unsigned short block_number;
};

// ��������� ������ ������
struct ERRORPacket {
    unsigned short error_code;
    std::string error_message;
};

// ������� ��� �������� �������
RRQ_WRQ create_RRQ(const std::string& name, Mode mode);
RRQ_WRQ create_WRQ(const std::string& name, Mode mode);
Data create_DATA(unsigned short block, const std::vector<unsigned char>& data);
ACKPacket create_ACKPacket(unsigned short block);
ERRORPacket create_ERRORPacket(ErrorCode code, const std::string& message);

// ������� ������������
std::vector<unsigned char> serialize(const RRQ_WRQ& packet);
std::vector<unsigned char> serialize(const Data& packet);
std::vector<unsigned char> serialize(const ACKPacket& packet);
std::vector<unsigned char> serialize(const ERRORPacket& packet);

//// ������� ��������������
RRQ_WRQ deserialize_RRQ_WRQ(const std::vector<unsigned char>& data);
Data deserialize_DATA(const std::vector<unsigned char>& data);
ACKPacket deserialize_ACK(const std::vector<unsigned char>& data);
ERRORPacket deserialize_ERROR(const std::vector<unsigned char>& data);

Opcode determine_tftp_type(const std::vector<unsigned char>& data);

#endif // TFTP_H