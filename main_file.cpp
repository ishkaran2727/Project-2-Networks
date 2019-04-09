#include"main_header.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h> 
#include <string.h>	
#include <string>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <time.h>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <stdlib.h>


using namespace std;

#define buffer_length 2048

using namespace std;

struct header
{
	int type;
	char source;
	char dest;
	int length;
};

enum type
{
	datatype, incoming_tabletype, check_waketype, resettype
};

void *create_packet(int type, char source, char dest, int payload_length, void *payload);
header get_header(void *packet);
void *get_length(void *packet, int length);
void send_to_all(Distance_vector &dv, int socketfd);
void repeated_check(Distance_vector &dv, int socketfd, int type, char source = 0, char dest = 0, int payload_length = 0, void *payload = 0);

int main(int argc, char **argv)
{
	// check for errors
	if (argc < 3)
	{
		perror("Not enough arguments.\nUsage: ./my_router <initialization file> <router name>\n");
		return 0;
	}

	Distance_vector dv(argv[1], argv[2]);

	vector<node> neighbours = dv.neighbours();

	int myPort = dv.port_no_of(argv[2][0]); // my port

	dv.initial_addr(myPort);
	sockaddr_in myaddr = dv.myaddr();

	socklen_t addrlen = sizeof(sockaddr_in); // length of addresses

	// create a UDP socket
	int socketfd; // our socket
	if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("cannot create socket\n");
		return 0;
	}
	
	// bind the socket to localhost and myPort
	if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0)
	{
		perror("bind failed");
		return 0;
	}

	// send a data packet to router A
	if (dv.get_name() == 'H')
	{
		
		char data[100];
		memset(data, 0, 100);
		cin.getline(data, 100);
		for (int i = 0; i < neighbours.size(); i++)
		{
			if (neighbours[i].name == 'A')
			{
				void *dataPacket = create_packet(datatype, dv.get_name(), 'D', strlen(data), (void*)data);
				sendto(socketfd, dataPacket, sizeof(header) + dv.get_size(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));

				// print info
				header h = get_header(dataPacket);
				cout << "Sent data packet" << endl;
				cout << "Type: data" << endl;
				cout << "Source: " << h.source << endl;
				cout << "Destination: " << h.dest << endl;
				cout << "Length of packet: " << sizeof(header) + h.length << endl;
				cout << "Length of payload: " << h.length << endl;
				cout << "Payload: " << data << endl;

				free(dataPacket);
			}
		}
		exit(0);
	}

	// distance vector routing
	int pid = fork();
	if (pid < 0)
	{
		perror("fork failed");
		return 0;
	}
	else if (pid == 0) // send to each neighbor periodically
	{
		for (;;)
		{
			// periodically wake up parent process
			repeated_check(dv, socketfd, check_waketype);
			sleep(1);
		}
	}
	else // listen for incoming_tables
	{
		void *rcvbuf = malloc(buffer_length);
		sockaddr_in remaddr;
		for (;;)
		{
			memset(rcvbuf, 0, buffer_length);
			int recvlen = recvfrom(socketfd, rcvbuf, buffer_length, 0, (struct sockaddr *)&remaddr, &addrlen);
			
			header h = get_header(rcvbuf);
			void *payload = get_length(rcvbuf, h.length);
			switch(h.type)
			{
				case datatype:{
					cout << "Received data packet" << endl;
					time_t basic_time;
					time(&basic_time);
					cout << "Timestamp: " << ctime(&basic_time);
					cout << "ID of source node: " << h.source << endl;
					cout << "ID of destination node: " << h.dest << endl;
					cout << "UDP port in which the packet arrived: " << myPort << endl;
					if (h.dest != dv.get_name()) // only forward if this router is not the destination
					{
						if (dv.routeTo(h.dest).nextport() == -1)
						{
							cout << "Error: packet could not be forwarded" << endl;
						}
						else
						{
							cout << "UDP port along which the packet was forwarded: " << dv.routeTo(h.dest).nextport() << endl;
							cout << "ID of node that packet was forwarded to: " << dv.routeTo(h.dest).nextname() << endl;
							void *forwardPacket = create_packet(datatype, h.source, h.dest, h.length, (void*)payload);
							for (int i = 0; i < neighbours.size(); i++)
							{
								if (neighbours[i].name == dv.routeTo(h.dest).nextname())
									sendto(socketfd, forwardPacket, sizeof(header) + dv.get_size(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));
							}
							free(forwardPacket);
						}
						cout << endl;
					}
					else
					{
						char data[100];
						memset(data, 0, 100);
						memcpy((void*)data, payload, h.length);
						cout << "Data payload: " << data << endl << endl;
					}}
					break;
				case incoming_tabletype:{
					dist_vec_records entries[no_of_routers];
					memcpy((void*)entries, payload, h.length);
					for (int i = 0; i < neighbours.size(); i++)
					{
						if (neighbours[i].name == h.source)
							dv.initial_timer(neighbours[i]);
					}
					dv.bellman_ford(payload, h.source);}
					break;
				case check_waketype: // perform periodic tasks
					{for (int i = 0; i < neighbours.size(); i++)
					{
						node curNeighbor = neighbours[i];
						if ((dv.get_entries()[dv.index_of(curNeighbor.name)].cost1() != -1) && dv.time_khatam(neighbours[i]))
						{
							repeated_check(dv, socketfd, resettype, dv.get_name(), neighbours[i].name, dv.get_size() / sizeof(dist_vec_records) - 2);
						}
					}
					send_to_all(dv, socketfd);}
					break;
				case resettype:{
					int hopcount = (int)h.length - 1;
					dv.reset(h.dest);
					if (hopcount > 0)
					{
						void *forwardPacket = create_packet(resettype, dv.get_name(), h.dest, hopcount, (void*)0);
						for (int i = 0; i < neighbours.size(); i++)
						{
							if (neighbours[i].name != h.source)
								sendto(socketfd, forwardPacket, sizeof(header), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));
						}
					}}
					break;
			}
		}
		free(rcvbuf);
	}
}

// create a packet with header and payload
void *create_packet(int type, char source, char dest, int payload_length, void *payload)
{
	int allocatedPayloadLength = payload_length;
	if ((type != datatype) && (type != incoming_tabletype))
		allocatedPayloadLength = 0;

	// create empty packet
	void *packet = malloc(sizeof(header)+allocatedPayloadLength);

	// create header
	header h;
	h.type = type;
	h.source = source;
	h.dest = dest;
	h.length = payload_length;

	// fill in packet
	memcpy(packet, (void*)&h, sizeof(header));
	memcpy((void*)((char*)packet+sizeof(header)), payload, allocatedPayloadLength);

	return packet;
}

// extract the header from the packet
header get_header(void *packet)
{
	header h;
	memcpy((void*)&h, packet, sizeof(header));
	return h;
}

// extract the payload from the packet
void *get_length(void *packet, int length)
{
	void *payload = malloc(length);
	memcpy(payload, (void*)((char*)packet+sizeof(header)), length);
	return payload;
}

// send_to_all incoming_table to all neighbours
void send_to_all(Distance_vector &dv, int socketfd)
{
	vector<node> neighbours = dv.neighbours();
	for (int i = 0; i < neighbours.size(); i++)
	{
		void *sendPacket = create_packet(incoming_tabletype, dv.get_name(), neighbours[i].name, dv.get_size(), (void*)dv.get_entries());
		sendto(socketfd, sendPacket, sizeof(header) + dv.get_size(), 0, (struct sockaddr *)&neighbours[i].addr, sizeof(sockaddr_in));
		free(sendPacket);
	}
}

// periodically wake yourself up to send_to_all incoming_table
void repeated_check(Distance_vector &dv, int socketfd, int type, char source, char dest, int payload_length, void *payload)
{
	void *sendPacket = create_packet(type, source, dest, payload_length, payload);
	sockaddr_in destAddr = dv.myaddr();
	sendto(socketfd, sendPacket, sizeof(header), 0, (struct sockaddr *)&destAddr, sizeof(sockaddr_in));
	free(sendPacket);
}

//fuddu bana diya