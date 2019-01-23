#include <iostream>
#include "Packet.h"

using namespace std;


//Constructor - Set member variables to safe empty state
Packet::Packet() {
	this->packet.head.PktCount = 0;
	this->packet.head.sleep = 0;
	this->packet.head.status = 0;
	this->packet.head.drive = 0;
	this->packet.head.claw = 0;
	this->packet.head.arm = 0;
	this->packet.head.ack = 0;
	this->packet.head.pad = 0;
	this->packet.head.Length = 0;
	this->packet.Data = nullptr;
	this->packet.CRC = 0;
	this->RawBuffer = nullptr;
};



//Overloaded Constructor - takes a RAW data buffer
Packet::Packet(char *buf) {

	this->RawBuffer = nullptr;
	this->packet.Data = nullptr;

	memcpy(&packet.head.PktCount, &buf[0], sizeof(packet.head.PktCount));
	char* ptr = &buf[4];

	//Parses & populates the Header, Body, and CRC of the packet
	packet.head.drive = (*ptr & 1);
	packet.head.status = ((*ptr >> 1) & 1);
	packet.head.sleep = ((*ptr >> 2) & 1);
	packet.head.arm = ((*ptr >> 3) & 1);
	packet.head.claw = ((*ptr >> 4) & 1);
	packet.head.ack = ((*ptr >> 5) & 1);
	packet.head.pad = ((*ptr >> 6) & 1);

	memcpy(&this->packet.head.Length, &buf[5], sizeof(char));
	int dataLength = this->packet.head.Length - HEADERSIZE - 1;

	SetBodyData(&buf[6], dataLength);
	memcpy(&packet.CRC, &buf[packet.head.Length - 1], sizeof(char));
}



//Deconstructor - Deallocates memory
Packet::~Packet() {
	delete[] this->RawBuffer;
	delete[] this->packet.Data;
}



//Sets the packets command flag based on CmdType
void Packet::SetCmd(CmdType type) {
	if (type != ACK) {
		this->packet.head.ack = this->packet.head.arm = this->packet.head.claw = this->packet.head.sleep = this->packet.head.drive = 0;
	}

	if (type == DRIVE) {
		this->packet.head.drive = 1;
	}
	else if (type == SLEEP) {
		this->packet.head.sleep = 1;
	}
	else if (type == ARM) {
		this->packet.head.arm = 1;
	}
	else if (type == CLAW) {
		this->packet.head.claw = 1;
	}
	else if (type == ACK) {
		this->packet.head.ack = 1;
	}
}



//Takes a pointer to a RAW data buffer and the size of the buffer in bytes
void Packet::SetBodyData(char *buffer, int length) {

	//Allocate the packets Body field and copies the provided data into the objects buffer
	this->packet.Data = nullptr;
	this->packet.Data = new char[length];

	for (int i = 0; i < length; i++) {
		memcpy(&packet.Data[i], &buffer[i], sizeof(char));
	}

	this->packet.head.Length = length + HEADERSIZE + 1;
}



//Setter - Set Packet count
void Packet::SetPktCount(int count) {
	this->packet.head.PktCount = count;
}



//Getter - Returns acknowledgement
bool Packet::GetAck() {
	return (bool)this->packet.head.ack;
}



//Getter - Returns Status
bool Packet::GetStatus() {
	return (bool)this->packet.head.status;
}



//Getter - Returns Length
int Packet::GetLength() {
	return (int)this->packet.head.Length;
}



//Returns Packet Data
char *Packet::GetBodyData() {
	return this->packet.Data;
}



//Returns the number of packets transmitted between client & robot
int Packet::GetPktCount() {
	return this->packet.head.PktCount;
}



//Returns Command 
CmdType Packet::GetCmd() {
	if (this->packet.head.ack == 1) {
		return (CmdType)(ACK + 1);
	}
	else if (this->packet.head.arm == 1) {
		return (CmdType)(ARM + 1);
	}
	else if (this->packet.head.claw == 1) {
		return (CmdType)(CLAW + 1);
	}
	else if (this->packet.head.drive == 1) {
		return (CmdType)(DRIVE + 1);
	}
	else if (this->packet.head.sleep == 1) {
		return (CmdType)(SLEEP + 1);
	}
}



//Checks CRC - Takes a pointer to a RAW data buffer and the size of the packet in bytes .
bool Packet::CheckCRC(char *buffer, int length) {
	int counter = 0;

	//Calcualtes the CRC & returns the a bool respective to the outcome
	for (int i = 0; i < length - 1; i++) {
		char *a = &buffer[i];
		for (int x = 0; x < 8; x++) {
			counter += (*a >> x) & 1;
		}
	}
	return (counter == buffer[length - 1]);
}



//Calcualtes the Cyclic Redundancy Check
void Packet::CalcCRC() {
	char* buffer = GenPacket();
	packet.CRC = 0;
	int counter = 0;
	for (int i = 0; i < this->packet.head.Length - 1; i++) {
		char a = buffer[i];
		for (int x = 0; x < 8; x++) {
			if (((a >> x) & 1) == 1) {
				counter++;
			}
		}
	}
	//Sets the packets CRC parameter
	this->packet.CRC = (char)counter;
}



//Generates a packet from the objects member variables into a Raw data packet (RawBuffer)
char *Packet::GenPacket() {

	//allocates the private RawBuffer and transfers the contents from the objects member 
	//variables into a RAW data packet for transmission.
	this->RawBuffer = new char[this->packet.head.Length];
	memset(RawBuffer, 0, packet.head.Length);
	memcpy(&RawBuffer[0], &this->packet.head.PktCount, (packet.head.Length * 1));
	char* ptr = ((char*)&this->packet.head.PktCount + sizeof(packet.head.PktCount));

	memcpy(&RawBuffer[4], ptr, sizeof(char));
	memcpy(&RawBuffer[5], &this->packet.head.Length, sizeof(char));

	int dataLength = this->packet.head.Length - HEADERSIZE - 1;
	memcpy(&RawBuffer[6], this->packet.Data, dataLength);
	memcpy(&RawBuffer[packet.head.Length - 1], &this->packet.CRC, sizeof(char));

	//Return address of allocated raw buffer
	return this->RawBuffer;
}
