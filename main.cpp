//	Simple LAN Chat 
//	Matthew Morrish

// Libs
#include "main.h"

// Chat server
void server(int port, std::string source_ip)
{
	// Update user
	std::cout << "\n[===== Begin Server =====]\n";
	std::cout << "[APP] Awaiting connection... ";

	// Create socket struct & get its size (to check connection)
	SOCKADDR_IN address;
	int conn_address_size = sizeof(address);

	// Connection socket - SOCK_DGRAM = UDP
	SOCKET sock_connection = socket(AF_INET, SOCK_STREAM, 0);

	// Convert source ip from string to char array & set source_ip in address
	char ch_source_ip[source_ip.length() + 1];
	strcpy(ch_source_ip, source_ip.c_str());
	address.sin_addr.s_addr = inet_addr(ch_source_ip); // Set ip of self

	// Set other address attr's
	address.sin_family = AF_INET; // Set domain type to internet domain
	address.sin_port = htons(port); // Convert type & set port

	// Listening socket - SOCK_DGRAM = UDP
	SOCKET sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	bind(sock_listen, (SOCKADDR*)&address, sizeof(address)); // Bind socket to ip & port & its size
	listen(sock_listen, 0); // Listen for connection requests & set max backlog to 0

	// If connection found
	char message[1024]; // Buffer tail (1024 = max messge len)
	std::string converter; // Buffer head
	if (sock_connection = accept(sock_listen, (SOCKADDR*)&address, &conn_address_size))
	{
		// Store client ip & update user
		std::string dest_ip = inet_ntoa(address.sin_addr);
		std::cout << "[CONNECTED]\n\n";
		std::cout << "[===== Begin Chat =====]\n";

		// Begin message exchange
		int alive = 1;
		while(alive >= 0)
		{
			// Get message from user
			std::cout << "[Me] ";
			std::getline(std::cin, converter);

			// Convert from string to char array & send
			char to_send[converter.length() + 1];
			strcpy(to_send, converter.c_str());
			alive = send(sock_connection, to_send, sizeof(to_send), 0);

			// Receive as a character array
			alive = recv(sock_connection, message, sizeof(message), 0);
			if(alive >= 0) // Prevent previous message getting dumped to console
			{
				// Convert to string for cout (recv() doesn't null terminate)
				converter = message;
				std::cout << "[" << dest_ip << "] " << message << "\n";
			}
		}
	}
}

// local_ip member functions
namespace localIp
{
	// Finds & returns available adapter addresses
	std::vector<std::string> get()
	{
		//Locals
		std::vector<std::string> addrs;

		// Get the hostname & lookup ip
		char ac[80];
		gethostname(ac, sizeof(ac));
		struct hostent *phe = gethostbyname(ac);

		// Iterate through address list
		std::string convert;
		for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));

			// Convert from char array to string, push to vector
			convert = inet_ntoa(addr);
			addrs.push_back(convert);
		}
		
		return addrs;
	}

	// Similar to dest_ip::validate but stripped down
	int validate(std::string str, int max_size)
	{	 
		// Create a lambda to check that the string isn't empty & if any characters don't match 0-9
		auto is_num = [&] (std::string l_str) 
					  {return !l_str.empty() && (l_str.find_first_not_of("0123456789") == std::string::npos); };

		// Check that string is a number & doesn't exceed max_size
		if(is_num(str))
		{
			if(stoi(str) <= max_size && stoi(str) > 0)
			return stoi(str);
		}
		return -1;
	}
}

// Chat client
int client(int port, std::string dest_ip)
{
	// Update user
	std::cout << "\n[===== Begin Client =====]\n";
	std::cout << "[APP] Attempting to connect to [" << dest_ip << "]... ";

	// Create a socket & socket addres obj
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0); //TCP
	SOCKADDR_IN address;

	// Convert destination ip from string to char array & set dest_ip in address
	char ch_dest_ip[dest_ip.length() + 1];
	strcpy(ch_dest_ip, dest_ip.c_str());
	address.sin_addr.s_addr = inet_addr(ch_dest_ip);

	// Set other address attr's
	address.sin_family = AF_INET; // Internet domain
	address.sin_port = htons(port); // Convert type & set port 

	// Connect to server & error handle
	if(connect(sock, (SOCKADDR*)&address, sizeof(address)) != 0)
	{
		std::cout << "[FAILED]\n\n";
		return 1;
	}

	// Update user
	std::cout << "[PASSED]\n";
	std::cout << "\n[===== Begin Chat =====]\n";

	// Begin message exchange
	int alive = 1;
	char message[1024]; // Buffer for received message
	std::string converter; // Actual message for user
	while(alive >= 0)
	{
		// Receive as a character array
		alive = recv(sock, message, sizeof(message), 0);

		// Prevent previous message getting dumped to console 
		// or from trying to send a msg when peer terminated connection
		if(alive >= 0) 
		{
			// Convert to string for cout (recv() doesn't null terminate?
			// & we need a string for input anyways)
			converter = message;
			std::cout << "[" << dest_ip << "] " << converter << "\n";

			// Get message from user
			std::cout << "[Me] ";
			std::getline(std::cin, converter);

			// Convert from string to char array & send
			char to_send[converter.length() + 1];
			strcpy(to_send, converter.c_str());
			alive = send(sock, to_send, sizeof(to_send), 0);
		}
	}

	return 0;
}

// dest_ip member functions
namespace destIp
{
	// Takes in a str & delimiter, tokenize & return
	std::vector<std::string> tokenize(std::string ip, std::string delimiter)
	{
		// Locals
		std::vector<std::string> tokens;

		// Iterate through the string delimiter positions until there aren't any more
		int pos;
		while ((pos = ip.find(delimiter)) != std::string::npos)
		{
			// Extract a token as a substring of ip, push onto tokens vector, remove from ip
			tokens.push_back(ip.substr(0, pos));
			ip.erase(0, pos + delimiter.length());
		}

		// Push final token & return
		tokens.push_back(ip);
		return tokens;
	}

	// Take in a vector of strings and validate, return judgement
	bool validate(std::vector<std::string> tokens)
	{
		// Create a lambda to check that the string isn't empty & if any characters don't match 0-9
		auto is_num = [&] (std::string l_str) 
					  {return !l_str.empty() && (l_str.find_first_not_of("0123456789") == std::string::npos); };

		// Check that ip is 4 groups
		if (tokens.size() == 4)
		{
			// Validate each token
			for (std::string str: tokens)
			{
				// Check that each token is within range & is a number
				if (!is_num(str) || stoi(str) > 255 || stoi(str) < 0)
				{
					return false;
				}
			}
			return true;
		}
	}
}

// Start here
int main()
{
	// Locals
	const int port = 444;

	// Update user & initialize winsock - takes versioning word (2.1) & WSAData ref
	std::cout << "[===== Welcome =====]\n\n";
	std::cout << "[APP] Initializing socket service... ";
	WSAData WinSockData;
	if (WSAStartup(SCK_VERSION, &WinSockData) != 0)
	{
		std::cout << "[FAILED]\n";
		return 1;
	}
	std::cout << "[PASSED]\n";

	// Get destination ip from user
	std::string dest_ip = "";
	std::string get_ip_state = "[APP] P";
	while(!destIp::validate(destIp::tokenize(dest_ip, ".")))
	{
			std::cout << get_ip_state << "lease specify an ip address to connect to...\n      > ";
			getline(std::cin, dest_ip);
			get_ip_state = "\n[APP] Invalid IP, p";
	}

	// Try to connect as client
	if (client(port, dest_ip) != 0)
	{
		// If client fails, begin server by first getting an adapter address
		std::cout << "[===== Init Server =====]\n";
		std::cout << "[APP] Getting available adapters...\n";
		std::vector<std::string> local_ips = localIp::get();

		// Have user choose an address from available addresses
		std::string choice;
		int choice_num = -1;
		get_ip_state = "[APP] P";
		while(choice_num == -1)
		{
			std::cout << get_ip_state << "lease select an address from available adapters...\n";
			for(int i = 0; i < local_ips.size(); i++)
			{
				std::cout << "      " << i+1 << ") " << local_ips[i] << std::endl;
			}
			std::cout << "      > ";
			getline(std::cin, choice);

			choice_num = localIp::validate(choice, local_ips.size()); // Validate input & get str as int
			get_ip_state = "\n[APP] Invalid option, p";
		}

		// Begin server
		server(port, local_ips[choice_num-1]);
	}

	// Clean up, update & return
	WSACleanup();
	std::cout << "\n[APP] Connection has been terminated by [" << dest_ip << "],\n      please press any key to continue...";
	std::cin.get();
	return 0;
}