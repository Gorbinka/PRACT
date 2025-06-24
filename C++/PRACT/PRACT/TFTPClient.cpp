#include "TFTPClient.h"
#include <iostream>

using namespace std;

// Заглушки для SpaceWire-взаимодействия
void send_spacewire(const vector<unsigned char>& data) {
    // Реальная реализация будет здесь
    cout << "Отправлено " << data.size() << " байт через SpaceWire" << endl;
}

vector<unsigned char> receive_spacewire() {
    // Реальная реализация будет здесь
    cout << "Прием данных через SpaceWire..." << endl;
    return {};
}

bool send_file(const string& filename,
    const vector<unsigned char>& file_data,
    Mode mode)
{
    try {
        // 1. Отправка WRQ запроса
        cout << "Отправка WRQ запроса для файла: " << filename << endl;
        auto wrq = create_WRQ(filename, mode);
        auto serialized_wrq = serialize(wrq);
        auto stup_wrq = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_wrq);
        send_spacewire(serialize(stup_wrq));

        // 2. Получение ACK0
        auto recv_data = receive_spacewire();
        if (recv_data.empty()) {
            throw runtime_error("Таймаут при ожидании ACK0");
        }

        auto stup_ack0 = deserialize_STUP(recv_data);
        auto ack0 = deserialize_ACK(stup_ack0.data);

        if (ack0.block_number != 0) {
            throw runtime_error("Получен некорректный номер блока в ACK0");
        }

        // 3. Отправка данных блоками
        size_t offset = 0;
        unsigned short block_number = 1;

        while (offset < file_data.size()) {
            // Формирование блока данных
            size_t chunk_size = min(TFTP_BLOCK_SIZE, file_data.size() - offset);
            vector<unsigned char> block_data(file_data.begin() + offset,
                file_data.begin() + offset + chunk_size);
            offset += chunk_size;

            // Отправка DATA пакета
            cout << "Отправка DATA блока #" << block_number
                << " (" << chunk_size << " байт)" << endl;

            auto data_packet = create_DATA(block_number, block_data);
            auto serialized_data = serialize(data_packet);
            auto stup_data = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_data);
            send_spacewire(serialize(stup_data));

            // Ожидание ACK
            recv_data = receive_spacewire();
            if (recv_data.empty()) {
                throw runtime_error("Таймаут при ожидании ACK");
            }

            auto stup_ack = deserialize_STUP(recv_data);
            auto ack = deserialize_ACK(stup_ack.data);

            if (ack.block_number != block_number) {
                throw runtime_error("Некорректный номер блока в ACK: ожидался " +
                    to_string(block_number) + ", получен " +
                    to_string(ack.block_number));
            }

            block_number++;
        }

        // 4. Отправка пустого блока для завершения
        if (file_data.size() % TFTP_BLOCK_SIZE == 0) {
            cout << "Отправка финального пустого блока" << endl;
            auto final_packet = create_DATA(block_number, {});
            auto serialized_final = serialize(final_packet);
            auto stup_final = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_final);
            send_spacewire(serialize(stup_final));

            // Ожидание последнего ACK
            recv_data = receive_spacewire();
            if (recv_data.empty()) {
                throw runtime_error("Таймаут при ожидании финального ACK");
            }

            auto stup_ack = deserialize_STUP(recv_data);
            auto ack = deserialize_ACK(stup_ack.data);

            if (ack.block_number != block_number) {
                throw runtime_error("Некорректный номер блока в финальном ACK");
            }
        }

        cout << "Файл успешно отправлен!" << endl;
        return true;
    }
    catch (const exception& e) {
        cerr << "Ошибка отправки файла: " << e.what() << endl;
        return false;
    }
}

vector<unsigned char> receive_file(const string& filename,
    Mode mode)
{
    vector<unsigned char> file_data;

    try {
        // 1. Отправка RRQ запроса
        cout << "Отправка RRQ запроса для файла: " << filename << endl;
        auto rrq = create_RRQ(filename, mode);
        auto serialized_rrq = serialize(rrq);
        auto stup_rrq = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_rrq);
        send_spacewire(serialize(stup_rrq));

        // 2. Прием данных блоками
        unsigned short expected_block = 1;
        bool last_block = false;

        while (!last_block) {
            // Получение DATA пакета
            auto recv_data = receive_spacewire();
            if (recv_data.empty()) {
                throw runtime_error("Таймаут при ожидании DATA");
            }

            auto stup_packet = deserialize_STUP(recv_data);
            auto opcode = determine_tftp_type(stup_packet.data);

            if (opcode == ERROR) {
                auto error_packet = deserialize_ERROR(stup_packet.data);
                throw runtime_error("Ошибка от устройства: " + error_packet.error_message);
            }

            if (opcode != DATA) {
                throw runtime_error("Ожидался DATA пакет, получен: " + to_string(opcode));
            }

            auto data_packet = deserialize_DATA(stup_packet.data);

            // Проверка номера блока
            if (data_packet.block_number != expected_block) {
                throw runtime_error("Некорректный номер блока: ожидался " +
                    to_string(expected_block) + ", получен " +
                    to_string(data_packet.block_number));
            }

            // Сохранение данных
            file_data.insert(file_data.end(),
                data_packet.data.begin(),
                data_packet.data.end());

            // Проверка на последний блок
            last_block = (data_packet.data.size() < TFTP_BLOCK_SIZE);

            // Отправка ACK
            auto ack = create_ACKPacket(expected_block);
            auto serialized_ack = serialize(ack);
            auto stup_ack = create_packet(EMPTY_DPA, DEVICE_ADDR, PROTOCOL_ID, HOST_ADDR, serialized_ack);
            send_spacewire(serialize(stup_ack));

            expected_block++;
        }

        cout << "Файл успешно получен! Размер: "
            << file_data.size() << " байт" << endl;
    }
    catch (const exception& e) {
        cerr << "Ошибка получения файла: " << e.what() << endl;
        file_data.clear();
    }

    return file_data;
}