#ifndef TFTPCLIENT_H
#define TFTPCLIENT_H

#include "TFTP.h"
#include "STUP.h"
#include <vector>

// ��������� ����������
const unsigned char HOST_ADDR = 0x01;
const unsigned char DEVICE_ADDR = 0x02;
const unsigned char PROTOCOL_ID = 0x99;
const std::vector<unsigned char> EMPTY_DPA;

// ������� ������ ����� ������ TFTP
const size_t TFTP_BLOCK_SIZE = 512;

// ������� �������� ����� �� ����������
bool send_file(const std::string& filename,
    const std::vector<unsigned char>& file_data,
    Mode mode);

// ������� ������� ����� � ����������
std::vector<unsigned char> receive_file(const std::string& filename,
    Mode mode);

#endif // TFTPCLIENT_H