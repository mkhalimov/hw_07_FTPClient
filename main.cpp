#include "FTPClient.hpp"

int main() {
    FTPClient client;
    client.open(); // Connect to the server
    client.signIn("username", "password"); // Sign in with username and password
    
    // Perform FTP operations
    client.listFiles();
    client.changeDirectory("directory");
    client.changeLocalDirectory("local_directory");
    client.retrieveFile("filename");
    client.sendFile("filename");
    client.deleteFile("filename");
    
    client.close(); // Close the connection
    
    return 0;
}
