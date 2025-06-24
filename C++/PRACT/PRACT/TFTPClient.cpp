#include "TFTPClient.h"
#include <iostream>

using namespace std;

// �������� ��� SpaceWire-��������������
void send_spacewire(const vector<unsigned char>& data) {
    // �������� ���������� ����� �����
    cout << "���������� " << data.size() << " ���� ����� SpaceWire" << endl;
}

vector<unsigned char> receive_spacewire() {
    // �������� ���������� ����� �����
    cout << "����� ������ ����� SpaceWire..." << endl;
    return {};
}

bool send_file(const string& filename,
    const vector<unsigned char>& file_data,
    Mode mode)
{
    try {
        // 1. �������� WRQ �������
        cout << "�������� WRQ ������� ��� �����: " << filename << endl;
        auto wrq = create_WRQ(filename, mode);
        auto serialized_wrq = serialize(wrq);
        auto stup_wrq = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_wrq);
        send_spacewire(serialize(stup_wrq));

        // 2. ��������� ACK0
        auto recv_data = receive_spacewire();
        if (recv_data.empty()) {
            throw runtime_error("������� ��� �������� ACK0");
        }

        auto stup_ack0 = deserialize_STUP(recv_data);
        auto ack0 = deserialize_ACK(stup_ack0.data);

        if (ack0.block_number != 0) {
            throw runtime_error("������� ������������ ����� ����� � ACK0");
        }

        // 3. �������� ������ �������
        size_t offset = 0;
        unsigned short block_number = 1;

        while (offset < file_data.size()) {
            // ������������ ����� ������
            size_t chunk_size = min(TFTP_BLOCK_SIZE, file_data.size() - offset);
            vector<unsigned char> block_data(file_data.begin() + offset,
                file_data.begin() + offset + chunk_size);
            offset += chunk_size;

            // �������� DATA ������
            cout << "�������� DATA ����� #" << block_number
                << " (" << chunk_size << " ����)" << endl;

            auto data_packet = create_DATA(block_number, block_data);
            auto serialized_data = serialize(data_packet);
            auto stup_data = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_data);
            send_spacewire(serialize(stup_data));

            // �������� ACK
            recv_data = receive_spacewire();
            if (recv_data.empty()) {
                throw runtime_error("������� ��� �������� ACK");
            }

            auto stup_ack = deserialize_STUP(recv_data);
            auto ack = deserialize_ACK(stup_ack.data);

            if (ack.block_number != block_number) {
                throw runtime_error("������������ ����� ����� � ACK: �������� " +
                    to_string(block_number) + ", ������� " +
                    to_string(ack.block_number));
            }

            block_number++;
        }

        // 4. �������� ������� ����� ��� ����������
        if (file_data.size() % TFTP_BLOCK_SIZE == 0) {
            cout << "�������� ���������� ������� �����" << endl;
            auto final_packet = create_DATA(block_number, {});
            auto serialized_final = serialize(final_packet);
            auto stup_final = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_final);
            send_spacewire(serialize(stup_final));

            // �������� ���������� ACK
            recv_data = receive_spacewire();
            if (recv_data.empty()) {
                throw runtime_error("������� ��� �������� ���������� ACK");
            }

            auto stup_ack = deserialize_STUP(recv_data);
            auto ack = deserialize_ACK(stup_ack.data);

            if (ack.block_number != block_number) {
                throw runtime_error("������������ ����� ����� � ��������� ACK");
            }
        }

        cout << "���� ������� ���������!" << endl;
        return true;
    }
    catch (const exception& e) {
        cerr << "������ �������� �����: " << e.what() << endl;
        return false;
    }
}

vector<unsigned char> receive_file(const string& filename,
    Mode mode)
{
    vector<unsigned char> file_data;

    try {
        // 1. �������� RRQ �������
        cout << "�������� RRQ ������� ��� �����: " << filename << endl;
        auto rrq = create_RRQ(filename, mode);
        auto serialized_rrq = serialize(rrq);
        auto stup_rrq = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_rrq);
        send_spacewire(serialize(stup_rrq));

        // 2. ����� ������ �������
        unsigned short expected_block = 1;
        bool last_block = false;

        while (!last_block) {
            // ��������� DATA ������
            auto recv_data = receive_spacewire();
            if (recv_data.empty()) {
                throw runtime_error("������� ��� �������� DATA");
            }

            auto stup_packet = deserialize_STUP(recv_data);
            auto opcode = determine_tftp_type(stup_packet.data);

            if (opcode == ERROR) {
                auto error_packet = deserialize_ERROR(stup_packet.data);
                throw runtime_error("������ �� ����������: " + error_packet.error_message);
            }

            if (opcode != DATA) {
                throw runtime_error("�������� DATA �����, �������: " + to_string(opcode));
            }

            auto data_packet = deserialize_DATA(stup_packet.data);

            // �������� ������ �����
            if (data_packet.block_number != expected_block) {
                throw runtime_error("������������ ����� �����: �������� " +
                    to_string(expected_block) + ", ������� " +
                    to_string(data_packet.block_number));
            }

            // ���������� ������
            file_data.insert(file_data.end(),
                data_packet.data.begin(),
                data_packet.data.end());

            // �������� �� ��������� ����
            last_block = (data_packet.data.size() < TFTP_BLOCK_SIZE);

            // �������� ACK
            auto ack = create_ACKPacket(expected_block);
            auto serialized_ack = serialize(ack);
            auto stup_ack = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_ack);
            send_spacewire(serialize(stup_ack));

            expected_block++;
        }

        cout << "���� ������� �������! ������: "
            << file_data.size() << " ����" << endl;
    }
    catch (const exception& e) {
        cerr << "������ ��������� �����: " << e.what() << endl;
        file_data.clear();
    }

    return file_data;
}