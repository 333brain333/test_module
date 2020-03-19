MCP2515 mcp2515(10);
MCP2515 mcp2515_1(9);

struct can_frame canMsg;
struct can_frame canMsgToSend;
enum status_enum{
NORMAL,
ERR
};
enum status_enum status=NORMAL;

String errorNames[] = {
  "Normal",
  "ERROR"
};

double speedFbCAN = 0.0;
double speedFbCAN_1 = 0.0;
unsigned long prevSendTime = 0;
unsigned long errTimer = 0;
unsigned long canSendTimer = 0;
const int relay1 = 5; // relay 1 - according to principal scheme  - it connects "neutral -" line to +24V - in case of HIGH mode, or disconnets with any line in case of LOW mode
const int relay2 = 6; // relay 2 - according to principal scheme - it connecnts "signal" and "neutral -" lines to handle or to digital potentiometer and to relay 1 accordingly
const int neutral = 7; // using this pin one could get nutral line state in inversed logic  - "neutral -":+24 V ==> 7 pin is LOW or "neutral -": 0 V ==> 7 pin is HIGH
const int signal_pin = A0; // using this pin one could get analog value of the handle position. +0.5V - moving forward with max velocity; +2.5V - neutral position, no moving; +4.5V - moving backwards with max velocity
const int wiper = A5; // wiper of the digital potentiometer. The usage of this pin helps to check if sent reference speed to digital potentiometer is implemented or not.
const int interruptPin = 2;
int handle_pos = 0; // position of the handle within a range of 0-255
int wiper_pos = 0; //here contains position in range 0-255 of the wiper of the digital potentio
int incoming_ref_speed = 127; //in range 0-255
int errCounter = 0;
String incoming_mes = "";
String incoming_mode = "m";
String current_mode = "manual";
bool interrupt = false;