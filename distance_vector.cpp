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


Distance_vector::Distance_vector(const char *filename, const char *self)
{
	fstream topology(filename);

	string line; // current line of file
	string field; // current token (to be put into entry's field)
	char my_name = self[0]; // name of self
	self1 = index_of(self[0]);

	// initialize entries1
	for (int dest = 0; dest < no_of_routers; dest++)
	{
		entries1[dest].set_next_name('0');
		entries1[dest].set_next_port(-1);
		entries1[dest].set_cost1(-1);
		entries1[dest].set_valid();
	}

	while (getline(topology, line)) // parse file line by line
	{
		stringstream linestream(line);
		dist_vec_records entry;

		entry.set_valid();

		// source router
		getline(linestream, field, ',');
		char name = field[0];

		// destination router
		getline(linestream, field, ',');
		int dest = index_of(field[0]);
		node n;
		n.name = field[0];
		entry.set_next_name(field[0]);

		// destination port number
		getline(linestream, field, ',');
		int port = atoi(field.c_str());
		entry.set_next_port(port);
		n.port_no = port;

		memset((char *)&n.addr, 0, sizeof(n.addr));
		n.addr.sin_family = AF_INET;
		n.addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		n.addr.sin_port = htons(port);

		// link cost
		getline(linestream, field, ',');
		entry.set_cost1(atoi(field.c_str()));

		if (my_name == 'H')
		{
			int i;
			for (i = 0; i < padosi.size(); i++)
			{
				if (padosi[i].name == n.name)
					break;
			}
			if (i == padosi.size())
				padosi.push_back(n);
		}
		else if (name == my_name)
		{
			initial_timer(n);
			padosi.push_back(n); // store neighbor
			entries1[dest] = entry;
		}

		port_nos1[n.name] = n.port_no;
	}

	// special port number for sending data packet
	port_nos1['H'] = 11111;

	memcpy((void*)entries1_backup, (void*)entries1, sizeof(entries1));

	if (name_of(self1) != 'H')
		print(entries1, name_of(self1), "Initial routing table", true);
}
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

// update this router's distance vector based on received incoming_table from source
// return false if this router's distance vector was not changed
void Distance_vector::bellman_ford(const void *incoming_table_buffer, char source)
{
	dist_vec_records original_entries[no_of_routers];
	memcpy((void*)original_entries, (void*)entries1, sizeof(entries1));

	bool new_distance_vector = false;

	int intermediate = index_of(source);
	if (entries1_backup[intermediate].invalid())
	{
		entries1_backup[intermediate].set_valid();
		entries1[intermediate].set_valid();

		new_distance_vector = true;
	}

	// load advertised distance vector
	dist_vec_records incoming_table[no_of_routers];
	memcpy((void*)incoming_table, incoming_table_buffer, sizeof(incoming_table));
 
	// recalculate self's distance vector
	for (int dest = 0; dest < no_of_routers; dest++)
	{
		if (dest == self1)
			continue;
		bool new_entry = false;
		entries1[dest].set_cost1(min(entries1[dest].cost1(), entries1[intermediate].cost1(), incoming_table[dest].cost1(), entries1[dest].nextname(), source, new_entry));
		if (new_entry)
		{
			new_distance_vector = true;
			entries1[dest].set_next_port(port_no_of(source));
			entries1[dest].set_next_name(source);
		}
	}
	entries1[intermediate].set_cost1(incoming_table[self1].cost1());

	if (new_distance_vector)
	{
		print(original_entries, name_of(self1), "Change detected!\nRouting table before change", true);
		print(incoming_table, source, "Distance_vector that caused the change", false);
		print(entries1, name_of(self1), "Routing table after change", false);
	}
}

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

//-----------------
// HELPER FUNCTIONS
//-----------------

// return minimum cost and set updated flag
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

