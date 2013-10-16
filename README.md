UDP-performance-evaluation-app
==============================
Introduction

This UDP program aims to test UDP communication performance including round trip delay and packet lose ration. The program has both server and client part and it can produce different volume of traffic (by changing the sending rate and packet size), symmetrical or unsymmetrical (symmetrical: the server replies whatever it received from the client, unsymmetrical: the server simply replies an ACK for each received packet).

In detail, the program consist of client and server part, implemented on Ubuntu GCC. ./server command can start the server part. To start the client, the command should follow the following format:

./client <host IP address> <sending rate (bytes/s)> <packet size (1-500 bytes)> <symmetrical or unsymmetrical mode (s/u)>
e.g. ./client 192.168.1.103 1000 500 u

After entering the right format command for client, it will start to keeping sending UDP packet to the server. In unsymmetrical mode, the ACK replied by server is a char variable 'u'. Ctrl +C command can stop the client and then the client will print a UDP communication performance report, including the average round trip delay and packet lose ratio.

What's more
1. It can check whether the execution command parameter.
2. It uses two thread for send data and receive data respectively.
3. The program uses an array to store the data that has been send. If an element in the next round still has no response, the corresponding packet will be seen as timeout.  


===============================
How to Compile

client: gcc -o client client.c -lpthread
server: gcc -o server server.c 
In addition, to terminate the execution, please firstly use Ctrl+C to terminate the server and then terminate the client. If not, this probably causes the default port cannot be used.

================================
Function test


Test Environment

Server program whose ip is 195.148.168.107 runs on my laptop and Client program runs on the school desktop computer whose ip is 193.166.140.104.

Program compile

server: gcc –o server server.c

Client: gcc -o client client.c –lpthread

Program execution

The client execution command format is:

./client <host IP address> <sending rate (1-1000bytes/s)> <packet size (3-500 bytes)> <symmetrical or unsymmetrical mode (s/u)>

Client: ./client 195.148.168.107 1000 10 u

Server: ./server

The execution figures are uploaded seperately, named 'server_execution_figure' and 'client_execution_figure'.

Program result

Using Ctrl C to terminate the client, the uploaded figure named 'result_figure' illustrates the result.
