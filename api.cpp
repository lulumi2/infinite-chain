#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "blockchain.hpp"
#include "api.hpp"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

void APIServer::run(Blockchain &blockchain, int port)
{
    // TODO: 不能限制長度
	char input_buffer[4096] = {};
	int server_fd = 0, client_fd = 0;
	server_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("socket() error\n");
		exit(1);
	}
	struct sockaddr_in server_addr, client_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
		printf("bind error\n");
		exit(1);
	}
	if (listen(server_fd, 5) == -1) {
		printf("listen error\n");
		exit(1);
	}
	fd_set reads, copy_reads;
	FD_ZERO(&reads);
	FD_SET(server_fd, &reads);
	int fd_max = server_fd;
	while (1) {
		copy_reads = reads;
		int fd_num = select(fd_max + 1, &copy_reads, 0, 0, 0);
		if (fd_num == -1) {
			fprintf(stderr, "select error\n");
			break;
		} else if (fd_num == 0) {
			continue;
		}
		for (int i = 0; i < fd_max + 1; i++) {
			if (FD_ISSET(i, &copy_reads)) {
				if (i == server_fd) { // connection
					socklen_t addr_size = sizeof(client_addr);
					client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_size);
					if (client_fd == -1) {
						fprintf(stderr, "accept error\n");
					}
					FD_SET(client_fd, &reads);
					if (fd_max < client_fd) {
						fd_max = client_fd;
					}
					fprintf(stderr, "new connection\n");
				} else { // read message
					memset(input_buffer, 0, sizeof(input_buffer));
					int len = read(i, input_buffer, sizeof(input_buffer));
					fprintf(stderr, "input %s\n", input_buffer);
					if (len == 0) {
						FD_CLR(i, &reads);
						close(i);
						fprintf(stderr, "close connection\n");
					} else {
						string ret = getResponse(input_buffer, blockchain);
						ret += "\n";
						write(i, ret.c_str(), ret.length());
					}
				}
			}
		}
	}
}

string UserApi::getResponse(string message, Blockchain &blockchain)
{
	try {
		json j = json::parse(message);
		string method = j["method"];
		if (method == "getBlockCount") {
			json response;
			response["result"] = blockchain.getBlockCount();
			return response.dump();
		} else if (method == "getBlockHash") {
		} else if (method == "getBlockHeader") {
		} else if (method == "getbalance") {
		    json response;
		    response["error"] = "";
		    response["balance"] = blockchain.getBalance(j["data"]["address"]);
		    return response.dump();
		} else if (method == "sendtoaddress") {
		    blockchain.sendToAddress(j["data"]["address"], j["data"]["amount"]);
            json response;
            response["error"] = "";
            return response.dump();
		}
	} catch (exception &e) {
		fprintf(stderr, "exception: %s\n", e.what());
		fprintf(stderr, "getResponse error\n");
	}
	return "";
}

string P2PApi::getResponse(string message, Blockchain &blockchain)
{
	try {
		json j = json::parse(message);
		string method = j["method"];
		if (method == "sendBlock") {
			blockchain.addBlock(Block(message));
		} else if (method == "getBlocks") {
		} else if (method == "sendTransaction") {
		    string tx_str = j["data"].dump();
		    blockchain.addRemoteTransaction(tx_str);
		}
	} catch (exception &e) {
        fprintf(stderr, "exception: %s\n", e.what());
		fprintf(stderr, "getResponse error\n");
	}
	return "";
}
