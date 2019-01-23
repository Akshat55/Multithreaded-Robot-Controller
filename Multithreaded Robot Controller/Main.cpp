#include <iostream>
#include <string>
#include <iomanip>
#include <thread>
#include "Packet.h"
#include "MySocket.h"


using namespace std;

bool ExeComplete = false;


//Displays choice and returns it
int getCommand() {
	int choice = 0;

	//Ensures value is correct before sending
	do {
		cout << "Enter a option: " << endl;
		cout << "1. Drive" << endl;
		cout << "2. Sleep" << endl;
		cout << "3. Arm" << endl;
		cout << "4. Claw" << endl;
		cin >> choice;

		cin.ignore();

		//Ensures a valid option is entered
		if (choice <= 0 || choice >= 5) {
			cout << "Invalid Input, please try again." << endl;
			choice = 0;
		}

	} while ((choice <= 0 || choice >= 5));

	return choice;
}



//Displays the options and gets the command (int)
int getDriveCommand() {
	int choice = 0;

	//Ensure value is correct before sending
	do {
		cout << "Enter a option: " << endl;
		cout << "1. FORWARD" << endl;
		cout << "2. BACKWARD" << endl;
		cout << "3. RIGHT" << endl;
		cout << "4. LEFT" << endl;
		cin >> choice;

		cin.ignore();

		//Ensures a valid option is entered
		if (choice <= 0 || choice >= 5) {
			cout << "Invalid Input, please try again." << endl;
			choice = 0;
		}

	} while ((choice <= 0 || choice >= 5));

	return choice;
}



//Displays the options and gets the command (int)
int getArmCommand() {
	int choice = 0;

	//Ensure value is correct before sending
	do {
		cout << "Enter a option: " << endl;
		cout << "5. UP" << endl;
		cout << "6. DOWN" << endl;
		cin >> choice;

		cin.ignore();

		//Ensures a valid option is entered
		if (choice <= 4 || choice >= 7) {
			cout << "Invalid Input, please try again." << endl;
			choice = 0;
		}

	} while ((choice <= 4 || choice >= 7));

	return choice;
}



//Displays the options and gets the command (int)
int getClawCommand() {
	int choice = 0;

	//Ensure value is correct before sending
	do {
		cout << "Enter a option: " << endl;
		cout << "7. OPEN" << endl;
		cout << "8. CLOSE" << endl;
		cin >> choice;

		cin.ignore();

		//Ensures a valid option is entered
		if (choice <= 6 || choice >= 9) {
			cout << "Invalid Input, please try again." << endl;
			choice = 0;
		}

	} while ((choice <= 6 || choice >= 9));

	return choice;
}



//The command thread is responsible for the command socket interface, 
//collecting user drive information, generating packets and monitoring acknowledgements from the robot.

void CommandThread(string ip, int port) {

	cout << "===== New Command Thread =====" << endl;

	MySocket teleSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, 100);
	//Perform the 3-way handshake to establish a reliable connection with the robot’s command
	teleSocket.ConnectTCP();

	Packet packet;
	CmdType cmd;
	MotorBody newBody;
	ActuatorBody myAction;

	while (ExeComplete == false) {

		//Initialization & Declaration
		int duration = 0;
		int direction = 0;
		char *recvBuffer = nullptr;
		char txBuffer[100] = { 0 };		//Set every element to 0


		cout << endl;
		cout << "===============================" << endl;
		cout << endl;

		//Get User Command
		cmd = (CmdType)getCommand();

		//Clear any errors before getting user input
		cin.clear();
		cout.clear();

		//Query Information
		if (cmd != SLEEP) {

			if (cmd == DRIVE) {
				//Querying in user input
				direction = getDriveCommand();
				cout << "Please enter the duration: " << endl;
				cin >> duration;
				cin.ignore();

				//Cast int to its respective char value
				newBody.Direction = static_cast<char>(direction);
				newBody.Duration = static_cast<char>(duration);

				//Set Motor Body
				packet.SetBodyData((char*)&newBody, 2);
			}
			else if (cmd == CLAW) {
				//Querying in user input
				myAction.Action = (actions)getClawCommand();

				packet.SetBodyData((char*)&myAction, 1);
			}
			else if (cmd == ARM) {
				//Querying in user input
				myAction.Action = (actions)getArmCommand();

				packet.SetBodyData((char*)&myAction, 1);
			}

		}
		else if (cmd == SLEEP) {
			//Sets length to 0 and body to empty
			packet.SetBodyData((char*)&newBody, 0);
			ExeComplete = true;
		}

		//Sets all the information (cmd, packetCount, CalcCRC)
		packet.SetCmd(cmd);
		int counter = packet.GetPktCount() + 1;
		packet.SetPktCount(counter);
		packet.CalcCRC();
		packet.GenPacket();

		//Send the buffer after creating it
		teleSocket.SendData(packet.GenPacket(), packet.GetLength());

		//Recv the buffer from server
		int length = teleSocket.GetData(txBuffer);
		//allocated, memcpy and memset
		recvBuffer = new char[length];
		memset(recvBuffer, 0, length);
		memcpy(recvBuffer, txBuffer, length);


		//Creates a packet out of the received data
		Packet another = Packet(recvBuffer);
		if (cmd == SLEEP && another.GetAck() == true) {
			teleSocket.DisconnectTCP();
			ExeComplete = true;

		}

		delete[] recvBuffer;
	}

}



//The telemetry thread is responsible for the telemetry socket interface, 
//it collects data packets that are continuously transmitted by the robot, 
//validates the packet and parses and displays the information to the screen
void TelemtryThread(string ip, int port) {

	cout << "===== New Telemery Thread =====" << endl;
	MySocket teleSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, 100);

	//Perform the 3-way handshake to establish a reliable connection with the robot’s command
	teleSocket.ConnectTCP();
	char buffer[100];


	while (ExeComplete == false) {

		cout << endl;
		cout << "===============================" << endl;
		cout << endl;

		//Clears the buffer
		memset(buffer, 0, 100);

		int bufLength = teleSocket.GetData(buffer);

		//Create a packet object out of what is recieved
		Packet packet(buffer);

		//CRC VALIDATION
		if (packet.CheckCRC(buffer, bufLength)) {

			cout << "CRC Validation: TRUE" << endl;

			//Status is set to true
			if (packet.GetStatus()) {

				cout << "Status bit is set to: TRUE" << endl;

				//MS1 - Testing
				cout << "Displaying the Raw Data Package :" << endl;
				cout << showbase
					<< internal
					<< setfill('0');

				char *ptr = packet.GenPacket();
				for (int x = 0; x < (int)packet.GetLength(); x++)
				{
					cout << hex << setw(4) << (unsigned int)*(ptr++) << ", ";
				}
				cout << dec << endl;
				cout.clear();

				//Retrieves the body data
				short int* sensorData = (short int*)packet.GetBodyData();
				//Displays the body data
				cout << "Sensor Data		: " << (short int)*sensorData++ << endl;
				cout << "Arm Data		: " << (short int)*sensorData++ << endl;


				char* bits = (char*)sensorData;
				//Displays the drive bit
				//((bool)((*bits) & 1) ? "DRIVING" : "STOPPED")
				cout << "Drive bit		: " << ((*bits) & 1) << endl;

				//Checks arm
				//Note: Ensure to not have endline, to make a complete sentence with the claw
				if (((*bits >> 1) & 1) == 1) {
					cout << "Arm is Up, ";
				}
				else {
					cout << "Arm is Down, ";
				}

				//End line with claw
				if (((*bits >> 3) & 1) == 1) {
					cout << "Claw is Open" << endl;
				}
				else {
					cout << "Claw is Closed" << endl;
				}

			}
			else {
				cout << "Status is FALSE" << endl;
			}
		}
		else {
			cout << "CRC Validation: FALSE" << endl;
		}

	}

}


int main(int argc, char *argv[])
{
	std::string ip = "127.0.0.1";
	int connectionport = 27000;
	int telport = 27501;

	//Ensures that ip, connectionport and telport aren't hard coded.
	if (argc > 1) {
		ip = argv[0];
		connectionport = std::atoi(argv[1]);
		telport = std::atoi(argv[2]);
	}

	//Start threads
	std::thread(CommandThread, ip, connectionport).detach();

	//To Test RANDOM input from robot, uncomment the following line
	//std::thread(TelemtryThread, ip, telport).detach();

	//Loop until ExeComplete == true
	while (ExeComplete == false) {};
}




