#include "TCPSocket.hpp"
#include <string>
#include <fstream>
#include <iostream>
#include <ctime>
#include <climits>
#include <filesystem>
#include <sstream>
#include <vector>

namespace fs = std::filesystem;
constexpr int BUF_SIZE = 2048;

std::vector<std::string> split(const std::string& s, const std::string& delimiter = " ") {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

class FTPClient {
private:
    std::string ip;
    TCPSocket* sock;
    TCPSocket* data_sock;
    std::ostream& output;
    fs::path ftp_path;
    fs::path lcd_path;

public:
    bool is_active = false;

    FTPClient(std::ostream& out = std::cout) 
        : ip("127.0.0.1"), output(out), ftp_path(fs::current_path()), lcd_path(fs::current_path()) {
        sock = new TCPSocket("127.0.0.1");
    }

    void open(const std::string& ip = "127.0.0.1", int port = 21) {
        sock = new TCPSocket(ip, port);
        output << "Connecting to the server..." << std::endl;
        sock->connect();
        output << recvMsg();
        is_active = true;
    }

    void enterPassiveMode() {
        std::string cmd_send = "PASV\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        std::string resp = recvMsg();
        output << "PASV response: " << resp;

        std::vector<int> ip_parts(6);
        std::sscanf(resp.c_str(), "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", &ip_parts[0], &ip_parts[1], &ip_parts[2], &ip_parts[3], &ip_parts[4], &ip_parts[5]);
        std::string data_ip = std::to_string(ip_parts[0]) + "." + std::to_string(ip_parts[1]) + "." + std::to_string(ip_parts[2]) + "." + std::to_string(ip_parts[3]);
        int data_port = ip_parts[4] * 256 + ip_parts[5];

        data_sock = new TCPSocket(data_ip, data_port);
        data_sock->connect();
    }

    void sendUser(const std::string& username) {
        std::string user_send = "USER " + username + "\r\n";
        sock->sendAll(user_send.c_str(), user_send.size());
        output << "USER response: " << recvMsg();
    }

    void sendPass(const std::string& password) {
        std::string pass_send = "PASS " + password + "\r\n";
        sock->sendAll(pass_send.c_str(), pass_send.size());
        output << "PASS response: " << recvMsg();
    }

    void signIn(const std::string& username, const std::string& password) {
        sendUser(username);
        sendPass(password);
    }

    void close() {
        std::string cmd_send = "QUIT\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        output << "QUIT response: " << recvMsg();
        sock->close();
        delete sock;
        delete data_sock;
        is_active = false;
    }

    std::string recvMsg() {
        char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        sock->receiveAll(buffer, sizeof(buffer));
        return std::string(buffer);
    }

    void listFiles() {
        enterPassiveMode();
        std::string cmd_send = "LIST\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        output << "LIST response: " << recvMsg();
        char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        std::stringstream file_data;
        while (data_sock->receiveAll(buffer, sizeof(buffer)) > 0) {
            file_data << buffer;
            memset(buffer, 0, sizeof(buffer));
        }
        output << file_data.str();
        data_sock->close();
    }

    void changeDirectory(const std::string& directory) {
        std::string cmd_send = "CWD " + directory + "\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        output << "CWD response: " << recvMsg();
    }

    void changeLocalDirectory(const std::string& directory) {
        lcd_path = fs::absolute(directory);
        output << "Local directory changed to: " << lcd_path << std::endl;
    }

    void retrieveFile(const std::string& filename) {
        enterPassiveMode();
        std::string cmd_send = "RETR " + filename + "\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        output << "RETR response: " << recvMsg();

        fs::path file_path = lcd_path / filename;
        std::ofstream file(file_path, std::ios::out | std::ios::binary);
        char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        while (data_sock->receiveAll(buffer, sizeof(buffer)) > 0) {
            file.write(buffer, sizeof(buffer));
            memset(buffer, 0, sizeof(buffer));
        }
        file.close();
        data_sock->close();
        output << "File retrieved and saved to: " << file_path << std::endl;
    }

    void sendFile(const std::string& filename) {
        enterPassiveMode();
        std::string cmd_send = "STOR " + filename + "\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        output << "STOR response: " << recvMsg();

        fs::path file_path = lcd_path / filename;
        std::ifstream file(file_path, std::ios::in | std::ios::binary);
        char buffer[BUF_SIZE];
        memset(buffer, 0, sizeof(buffer));
        while (file.read(buffer, sizeof(buffer))) {
            data_sock->sendAll(buffer, sizeof(buffer));
            memset(buffer, 0, sizeof(buffer));
        }
        file.close();
        data_sock->close();
        output << "File sent: " << file_path << std::endl;
    }

    void deleteFile(const std::string& filename) {
        std::string cmd_send = "DELE " + filename + "\r\n";
        sock->sendAll(cmd_send.c_str(), cmd_send.size());
        output << "DELE response: " << recvMsg();
    }
};
