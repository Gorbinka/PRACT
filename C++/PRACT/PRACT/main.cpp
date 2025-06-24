#include <iostream>
#include <vector>
#include "TFTPClient.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");

    // Пример отправки файла
    cout << "\n===== Тест отправки файла =====" << endl;
    vector<unsigned char> file_to_send = { 'H','e','l','l','o',' ','T','F','T','P','!' };
    send_file("test_send.txt", file_to_send, Mode::octet);

    // Пример получения файла
    cout << "\n===== Тест получения файла =====" << endl;
    auto received_file = receive_file("test_receive.txt", Mode::octet);

    cout << "\nПолученные данные (" << received_file.size() << " байт):" << endl;
    for (auto byte : received_file) {
        cout << static_cast<char>(byte);
    }
    cout << endl;

    return 0;
}