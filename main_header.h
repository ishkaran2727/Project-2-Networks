#ifndef Distance_vector_H
#define Distance_vector_H

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

//#include "Distance_vector.h"

//Distance_vector.h begins----------------------------------------------------------------------------------------



#define no_of_routers 6

struct dist_vec_records
{
public:	
	int nextport() const { return (invalid() ? -1 : next_port); }
	char nextname() const { return (invalid() ? '0' : next_name); }
	int cost1() const { return (invalid() ? -1 : cost); }
	bool invalid() const { return not_valid; }

	void set_next_port(int n) { next_port = n; }
	void set_next_name(char n) { next_name = n; }
	void set_cost1(int c) { cost = c; }
	void set_valid() { not_valid = false; }
	void set_invalid() { not_valid = true; }
//private hataya
	bool not_valid;
	int next_port; // port number of next hop router
	char next_name;
	int cost; // link cost to destination
};

struct node
{
	char name;
	int port_no;
	timespec initial_time;
	sockaddr_in addr;
};

class Distance_vector
{
public:
	//Distance_vector() {}
	Distance_vector(const char *filename, const char *self);
	//~Distance_vector() {}
	
	void reset(char bhand);
	dist_vec_records *get_entries() { return entries1; }
	int get_size() const { return sizeof(entries1); }
	char get_name() const { return name_of(self1); }
	void bellman_ford(const void *incoming_table, char src);
	dist_vec_records routeTo(const char dest) const { return entries1[index_of(dest)]; };
	std::vector<node> neighbours() const { return padosi; };
	int port_no_of(char router);
	char name_of(int index) const;
	int index_of(char router) const;
	void initial_addr(int port_no);
	sockaddr_in myaddr() const { return my_addr; }
	void initial_timer(node &n);
	bool time_khatam(node &n) const;
	int port() { return port_no_of(get_name()); }

    //private hataya
	// member variables
	int self1; // index of self
	int size1;
	dist_vec_records entries1[no_of_routers]; // each router's distance vectors
	dist_vec_records entries1_backup[no_of_routers]; // initial distance vectors (for resetting)
	vector<node> padosi; // port numbers of self's neighbours
	sockaddr_in my_addr;
	map<char, int> port_nos1;

	// helper functions
	int min(int original_cost, int self_via_cost, int via_desti_cost, char original_name, char new_name, bool &updated) const;
	void print(dist_vec_records dv[], char name, std::string msg, bool timestamp) const;
};


#endif
