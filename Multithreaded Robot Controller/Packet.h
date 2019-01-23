
struct Header {
	//contains an integer number that is constantly 
	//incrementing each time a packet is transmitted between the client and the robot
	unsigned int PktCount;

	//Bits
	unsigned int sleep : 1;
	unsigned int status : 1;
	unsigned int drive : 1;
	unsigned int claw : 1;
	unsigned int arm : 1;
	unsigned int ack : 1;
	//Padding to complete 1 byte
	unsigned int pad : 2;

	//contains an unsigned char with the total number of bytes in the packet
	unsigned char Length;
};


const int HEADERSIZE = 6;
enum actions { FORWARD = 1, BACKWARD, RIGHT, LEFT, UP, DOWN, OPEN, CLOSE };
enum CmdType { NACK, DRIVE = 1, SLEEP, ARM, CLAW, ACK };

struct MotorBody {
	unsigned char Direction;
	unsigned char Duration;
};


struct ActuatorBody {
	unsigned char Action;
};


class Packet {

private:

	struct CmdPacket {
		Header head;
		char *Data;	 // A pointer to the parameters of the command being constructed
		char CRC;	 // The packet validation value to ensure correct transmission
	} packet;

	char *RawBuffer;

public:
	Packet();
	Packet(char *);
	~Packet();
	void SetCmd(CmdType);
	void SetBodyData(char *, int);
	void SetPktCount(int);
	CmdType GetCmd();
	bool GetAck();
	bool GetStatus();
	int GetLength();
	char *GetBodyData();
	int GetPktCount();
	bool CheckCRC(char *, int);
	void CalcCRC();
	char *GenPacket();

};

