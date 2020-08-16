#include "initialisation.h"
#include "USB.h"
#include "MidiHandler.h"
#include "DACHandler.h"
#include "Config.h"
#include <string>
#include <sstream>
#include <iomanip>

USB usb;


uint8_t midiEventRead = 0;
uint8_t midiEventWrite = 0;
uint8_t eventOcc = 0;
uint16_t noteOnTest = 0;
volatile uint32_t SysTickVal;

bool noteDown = false;
bool dumped = false;

MidiData midiArray[MIDIBUFFERSIZE];		// for debugging
MidiHandler midiHandler;
DACHandler dacHandler;
Config cfg;

uint32_t debugClock = 0;
uint32_t debugClDiff = 0;

volatile uint8_t uartCmdPos = 0;
volatile char uartCmd[100];
volatile bool uartCmdRdy = false;

extern "C" {
#include "interrupts.h"
}



/*std::string IntToString(const int32_t& v) {
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
}*/

//void dumpArray(std::string loopback) {
//	usb.SendData((uint8_t*)loopback.c_str(), loopback.length());
//
//	uartSendStr("Event,Interrupt,Int Data,Endpoint,mRequest,Request,Value,Index,Length,PacketSize,XferBuff0,XferBuff1\n");
//	uint16_t evNo = usb.usbDebugEvent % USB_DEBUG_COUNT;
//
//	for (int i = 0; i < USB_DEBUG_COUNT; ++i) {
//		if (usb.usbDebug[evNo].Interrupt != 0) {
//			uartSendStr(IntToString(usb.usbDebug[evNo].eventNo) + ","
//					+ HexToString(usb.usbDebug[evNo].Interrupt, false) + ","
//					+ HexToString(usb.usbDebug[evNo].IntData, false) + ","
//					+ IntToString(usb.usbDebug[evNo].endpoint) + ","
//					+ HexByte(usb.usbDebug[evNo].Request.mRequest) + ", "
//					+ HexByte(usb.usbDebug[evNo].Request.Request) + ", "
//					+ HexByte(usb.usbDebug[evNo].Request.Value) + ", "
//					+ HexByte(usb.usbDebug[evNo].Request.Index) + ", "
//					+ HexByte(usb.usbDebug[evNo].Request.Length) + ", "
//					+ HexByte(usb.usbDebug[evNo].PacketSize) + ", "
//					+ HexToString(usb.usbDebug[evNo].xferBuff0, false) + ", "
//					+ HexToString(usb.usbDebug[evNo].xferBuff1, false) + "\n");
//		}
//		evNo = (evNo + 1) % USB_DEBUG_COUNT;
//	}
//}

void usbSerialdata(uint8_t* datain, uint32_t size) {
	uartSendStr(IntToString(size) + ":" + std::string((char*)datain, size) + "\n");
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

	// configure PC13 blue button
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13_0;			// Set pin to nothing:  01 Pull-up; 10 Pull-down; 11 Reserved
	GPIOC->MODER &= ~GPIO_MODER_MODE13_Msk;

	// PB7 is LD2 Blue
	GPIOB->MODER |= GPIO_MODER_MODER7_0;			// Set to output

	usb.cdcDataHandler = std::bind(usbSerialdata, std::placeholders::_1, std::placeholders::_2);

	while (1)
	{
//		midiHandler.gateTimer();
//		cfg.SaveConfig();		// Checks if configuration change is pending a save

		if (GPIOC->IDR & GPIO_IDR_ID13 && dumped == 0) {
			GPIOB->ODR |= GPIO_ODR_OD7;
			dumped = 1;
			usb.OutputDebug();
		} else {
			GPIOB->ODR &= ~GPIO_ODR_OD7;
			dumped = 0;
		}


		// Check if a UART command has been received
		if (uartCmdRdy) {
			std::stringstream ss;
			for (uint8_t c = 0; c < 100; ++c) {
				if (uartCmd[c] == 10) {
					if (ss.str().compare("x") == 0) {
						const char* transmit = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								"0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
								;
						usb.SendData((uint8_t*)transmit, strlen(transmit), usb.CDC_In);

					} else {
						usb.SendData((uint8_t*)uartCmd, c, usb.CDC_In);
					}
					break;
				}
				else
					ss << uartCmd[c];
			}
			uartCmdRdy = false;
		}
	}
}

