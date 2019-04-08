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


/*
void Distance_vector::reset(char bhand)
{
	for (int i = 0; i < padosi.size(); i++)
	{
		if (padosi[i].name == bhand)
		{
			if (entries1_backup[index_of(bhand)].cost1() != -1)
				entries1_backup[index_of(bhand)].set_invalid();
		}
	}
	memcpy((void*)entries1, (void*)entries1_backup, sizeof(entries1));
	print(entries1, name_of(self1), "Reset routing table", true);
}
*/
// update this router's distance vector based on received incoming_table from source
// return false if this router's distance vector was not changed


// return index of router
int Distance_vector::index_of(char router) const
{
	return router - 'A';
}

// return name of indexed router
char Distance_vector::name_of(int index) const
{
	return (char)index + 'A';
}

// return port number of router
int Distance_vector::port_no_of(char router)
{
	return port_nos1[router];
}

void Distance_vector::initial_addr(int port_no)
{
	memset((char *)&my_addr, 0, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	my_addr.sin_port = htons(port_no);
}

void Distance_vector::initial_timer(node &n)
{
	clock_gettime(CLOCK_MONOTONIC, &n.initial_time);
}

bool Distance_vector::time_khatam(node &n) const
{
	timespec tend={0,0};
	clock_gettime(CLOCK_MONOTONIC, &tend);

	if (((double)tend.tv_sec + 1.0e-9*tend.tv_nsec) - ((double)n.initial_time.tv_sec + 1.0e-9*n.initial_time.tv_nsec) > 5)
		return true;
	else
		return false;
}

/*
int Distance_vector::min(int original_cost, int self_via_cost, int via_desti_cost, char original_name, char new_name, bool &updated) const {
	int new_cost = self_via_cost + via_desti_cost;

	if (self_via_cost == -1 || via_desti_cost == -1)
	{
		return original_cost;
	}
	else if (original_cost == -1)
	{
		updated = true;
		return new_cost;
	}
	else if (new_cost < original_cost)
	{
		updated = true;
		return new_cost;
	}
	else if (original_cost == new_cost)
	{
		if (original_name <= new_name)
			updated = false;
		else
			updated = true;
		return new_cost;
	}
	else
	{
		return original_cost;
	}
}

// print a Distance_vector
// format: source, destination, port number of nexthop router, cost to destination
void Distance_vector::print(dist_vec_records dv[], char name, string msg, bool timestamp) const {
	if (timestamp)
	{
		time_t basic_time;
		time(&basic_time);
		cout << ctime(&basic_time);
	}
	cout << msg << ": " << name << endl;
	cout << "dst nexthop cost" << endl;
	for (int dest = 0; dest < no_of_routers; dest++)
	{
		cout << "  " << name_of(dest) << "   ";
		if (dv[dest].nextport() == -1)
			cout << "   ";
		cout << dv[dest].nextport() << "   ";
		if (dv[dest].cost1() != -1)
			cout << " ";
		cout << dv[dest].cost1();
		cout << endl;
	}
	cout << endl;
}
*/