#include <iostream>
#include <vector>
#include "TFTPClient.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");

    // ������ �������� �����
    cout << "\n===== ���� �������� ����� =====" << endl;
    vector<unsigned char> file_to_send = { 'H','e','l','l','o',' ','T','F','T','P','!' };
    send_file("test_send.txt", file_to_send, Mode::octet);

    // ������ ��������� �����
    cout << "\n===== ���� ��������� ����� =====" << endl;
    auto received_file = receive_file("test_receive.txt", Mode::octet);

    cout << "\n���������� ������ (" << received_file.size() << " ����):" << endl;
    for (auto byte : received_file) {
        cout << static_cast<char>(byte);
    }
    cout << endl;

    return 0;
}