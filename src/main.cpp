#include "initialisation.h"
#include "USB.h"
#include "MidiHandler.h"
#include "DACHandler.h"
#include "Config.h"
#include <string>
#include <sstream>
#include <iomanip>

USB usb;
uint32_t usbEvents[USB_DEBUG_COUNT];
uint16_t usbEventNo = 0;

uint8_t midiEventRead = 0;
uint8_t midiEventWrite = 0;
uint8_t eventOcc = 0;
uint16_t noteOnTest = 0;
volatile uint32_t SysTickVal;

bool noteDown = false;

MidiData midiArray[MIDIBUFFERSIZE];		// for debugging
MidiHandler midiHandler;
DACHandler dacHandler;
Config cfg;

uint32_t debugClock = 0;
uint32_t debugClDiff = 0;

volatile uint8_t uartCmdPos = 0;
volatile char uartCmd[50];
volatile bool uartCmdRdy = false;

extern "C" {
#include "interrupts.h"
}



std::string IntToString(const int32_t& v) {
	std::stringstream ss;
	ss << v;
	return ss.str();
}

std::string HexToString(const uint32_t& v, const bool& spaces) {
	std::stringstream ss;
	ss << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << v;
	if (spaces) {
		//std::string s = ss.str();
		return ss.str().insert(2, " ").insert(5, " ").insert(8, " ");
	}
	return ss.str();
}

std::string HexByte(const uint16_t& v) {
	std::stringstream ss;
	ss << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << v;
	return ss.str();
}

void uartSendStr(const std::string& s) {
	for (char c : s) {
		while ((USART3->SR & USART_SR_TXE) == 0);
		USART3->DR = c;
	}
}

void dumpArray() {
	uartSendStr("Event,Interrupt,Int Data,Endpoint,mRequest,Request,Value,Index,Length,PacketSize,XferBuff0,XferBuff1\n");

	for (int i = 0; i < usbEventNo; ++i) {
		//uartSendStr(IntToString(i) + "," + HexToString(usbEvents[i], false) + "," + HexByte(usbReqs[i].mRequest) + ", " + HexByte(usbReqs[i].Request) + ", " + HexByte(usbReqs[i].Value) + ", " + HexByte(usbReqs[i].Index) + ", " + HexByte(usbReqs[i].Length) + "\n");
		uartSendStr(IntToString(i) + ","
				+ HexToString(usb.usbDebug[i].Interrupt, false) + ","
				+ HexToString(usb.usbDebug[i].IntData, false) + ","
				+ IntToString(usb.usbDebug[i].endpoint) + ","
				+ HexByte(usb.usbDebug[i].Request.mRequest) + ", "
				+ HexByte(usb.usbDebug[i].Request.Request) + ", "
				+ HexByte(usb.usbDebug[i].Request.Value) + ", "
				+ HexByte(usb.usbDebug[i].Request.Index) + ", "
				+ HexByte(usb.usbDebug[i].Request.Length) + ", "
				+ HexByte(usb.usbDebug[i].PacketSize) + ", "
				+ HexToString(usb.usbDebug[i].xferBuff0, false) + ", "
				+ HexToString(usb.usbDebug[i].xferBuff1, false) + "\n");
	}
}



extern uint32_t SystemCoreClock;
int main(void)
{
	SystemInit();							// Activates floating point coprocessor and resets clock
	SystemClock_Config();					// Configure the clock and PLL
	SystemCoreClockUpdate();				// Update SystemCoreClock (system clock frequency) derived from settings of oscillators, prescalers and PLL
	InitUART();
	InitSysTick();
	usb.InitUSB();


//	dacHandler.initDAC();

//	cfg.RestoreConfig();
//	midiHandler.setConfig();

	// Bind the usb.dataHandler function to the midiHandler's event handler
	//usb.dataHandler = std::bind(&MidiHandler::eventHandler, &midiHandler, std::placeholders::_1);


	while (1)
	{
//		midiHandler.gateTimer();
//		cfg.SaveConfig();		// Checks if configuration change is pending a save

		// Check if a UART command has been received
		if (uartCmdRdy) {
			std::stringstream ss;
			for (uint8_t c = 0; c < 22; ++c) {
				if (uartCmd[c] == 10) {
					//can.pendingCmd = ss.str();
					//uartSendString("Received: " + ss.str());
					dumpArray();
					break;
				}
				else
					ss << uartCmd[c];
			}
			uartCmdRdy = false;
		}
	}
}

