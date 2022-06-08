#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
using namespace std;

#pragma comment (lib, "Ws2_32.lib") 
#define DEFAULT_BUFLEN 4096 
#define SERVER_IP "172.20.10.2"
#define DEFAULT_PORT "8888"
 
HANDLE hwnd = GetStdHandle(STD_OUTPUT_HANDLE);

char x[3];
int choiseColor()
{
    int y = 0;
    if (x[2] == '0') y = 0; 
    else if (x[2] == '1') y = 1;
    else if (x[2] == '2') y = 2;
    else if (x[2] == '3') y = 3;
    else if (x[2] == '4') y = 4;
    else if (x[2] == '5') y = 5;
    else if (x[2] == '6') y = 6;
    else if (x[2] == '7') y = 7;
    else if (x[2] == '8') y = 8;
    else if (x[2] == '9') y = 9;
    
    return y;
}

SOCKET client_socket;

DWORD WINAPI Sender(void* param)
{ 
    string nick;
    string vstup;
    cout << "Nick?" << endl;
    cin >> nick;
    nick += " ";

    int color;
    while (true)
    {
        cout << "Color 0 - 9 ?" << endl;
        cin >> color;
        if (color >= 0 && color <= 9) break;
    } 

    vstup += nick + "add to chat"; 
    send(client_socket, vstup.c_str(), vstup.size(), 0);
     

    while (true)    
    {
        string query;
        getline(cin, query);
        send(client_socket, nick.c_str(), nick.size(), 0);
        query += "[";
        query += to_string(color);
        query += "]";

        send(client_socket, query.c_str(), query.size(), 0);
    }
}

DWORD WINAPI Receiver(void* param)
{
    while (true) 
    {
        char response[DEFAULT_BUFLEN];
        int result = recv(client_socket, response, DEFAULT_BUFLEN, 0);
        response[result] = '\0';  
       
        int i1 = 0;
        for (size_t i = 0; i < 3; i++) x[i] = response[result - i]; 
        i1 = choiseColor();

        SetConsoleTextAttribute(hwnd, i1); 

        response[result - 3] = '\0';
        cout << response << "\n"; 
    }
}

BOOL ExitHandler(DWORD whatHappening)
{
    switch (whatHappening)
    {
    case CTRL_C_EVENT: // closing console by ctrl + c
    case CTRL_BREAK_EVENT: // ctrl + break
    case CTRL_CLOSE_EVENT: // closing the console window by X button
    case CTRL_LOGOFF_EVENT: // user logs off
    case CTRL_SHUTDOWN_EVENT: // system is shutting down
        cout << "Shutting down...\n";
        Sleep(1000);
        send(client_socket, "off", 3, 0);
        return(TRUE);
        break;
    default:
        return FALSE;
    }
}

int main()
{ 
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ExitHandler, true); 
    system("title Client");
     
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) 
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
     
    addrinfo* result = nullptr;
    iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) 
    {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 2;
    }

    addrinfo* ptr = nullptr; 
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) 
    { 
        client_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (client_socket == INVALID_SOCKET)
        {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 3;
        }
         
        iResult = connect(client_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) 
        {
            closesocket(client_socket);
            client_socket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (client_socket == INVALID_SOCKET) 
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 5;
    }

    CreateThread(0, 0, Sender, 0, 0, 0);
    CreateThread(0, 0, Receiver, 0, 0, 0);

    Sleep(INFINITE);
} 