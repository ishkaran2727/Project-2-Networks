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
