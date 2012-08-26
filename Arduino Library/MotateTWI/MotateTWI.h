/*
  MotateTWI.hpp - TWI/I2C Library for the Arduino-compatible Motate system
  http://tinkerin.gs/

  Copyright (c) 2012 Robert Giseburt

	This file is part of the Motate Library.

	The Motate Library is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	The Motate Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with the Motate Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOTATETWI_H_ONCE
#define MOTATETWI_H_ONCE

#include <inttypes.h>
#include <MotateCircularBuffer.h>
#include <Stream.h>


/*

	Example usage:
	
	using Motate;
	TWI::Master &master = TWI::getMaster();
	
	master.open(i2caddr);
	const uint8_t command1[] = {port_addr, data};
	master.write(command1); // stores command, starts sending data from it
	master.write((uint8_t)command2); // when command1 has been sent, stores command2 and starts sending
	master.write((uint8_t)command3); // sends or wait for command2 to send, stores command3 or starts sending
	master.sync(); // waits until all bytes have been sent
	master.close(); // NACK/STOP
	
	// Equals:
	Wire.beginTransmission(_i2caddr);
	Wire.write(port_addr);
	Wire.write(data);
	Wire.endTransmission();


	TWI::Slave &slave = TWI::getSlave();
	void slaveCallback(bool allCall) { ... }
	slave.setCallback(slaveCallback);
	slave.open(i2caddr, i2cmask); // addr = 0x00 -> all call
	// upon a master addressing (i2caddr & mask), slaveCallback(allCall) is called
	
	// alternative interface, poll for incoming connection and react
	if (slave.accept(BLOCKING)) {
		const uint8_t portAddr[2];
		slave.read(portAddr);       // reads two bytes into portAddr, blocking
	}

	// possible slaveCallback definition
	void dataRXCallBack(size_t count) { ... }
	void slaveCallback(bool allCall) {
		const uint8_t portAddr[2];
		slave.read(portAddr, dataRXCallBack); // reads two bytes into portAddr, returning immediately
		                                      // dataRXCallBack(count) will be called when portAddr is full
		                                      // or the master NACKed or STOPped. count tells how many bytes
	}
	
	const uint8_t command1[] = {port_addr, data};
	slave.write(command1);          // stores command1, starts sending data from it, up till last byte
	while (slave.sending()) { ; }   // force wait, but unnecessary
	slave.sync();                   // same as previous line
	slave.write((uint8_t)command2); // waits for command1 to sends and sends last byte of command1, stores command2
	slave.write((uint8_t)command3); // sends command2, stores command3
	slave.close();                  // sends command3 + NACK
	

*/

/*
template <typename T, size_t N>
inline size_t writeTest( const T(&buffer)[ N ], size_t x = N )
{
  Serial.write((const uint8_t *)buffer, N);
  return N;
}


template <typename T>
inline size_t writeTest( const T *(&buffer), size_t x )
{
  Serial.write((const uint8_t *)buffer, x);
  return x;
}
*/

#ifndef MOTATE_TWI_RX_BUFSIZE
#define MOTATE_TWI_RX_BUFSIZE 8
#endif

#ifndef MOTATE_TWI_TX_BUFSIZE
#define MOTATE_TWI_TX_BUFSIZE 8
#endif

#ifndef TWI_FREQ
#define TWI_FREQ 100000L
// #define TWI_FREQ 800000L
#endif

namespace Motate {
	namespace TWI {
		extern volatile uint8_t twi_error;
		
		class TWIBase /*: public Stream*/ {
		public:
			// public, because we need to be able to access these from the interrupt
			// and we can't exactly friend an interrupt (that I know of) -Rob G
			volatile bool is_open;
			volatile bool is_busy;
			volatile bool do_stop;
			volatile bool reopen;
			volatile bool ready_to_send;
			volatile bool holding_on_receive;
			volatile bool bump_tx;
			volatile bool in_flush;
			volatile bool writing;
			volatile int8_t expecting;

			CircularBuffer<char, MOTATE_TWI_TX_BUFSIZE, uint8_t> tx_buffer;
			CircularBuffer<char, MOTATE_TWI_RX_BUFSIZE, uint8_t> rx_buffer;
			
			TWIBase();
						
			// virtual is regrettable...
			/*virtual */ int available(void);
			/*virtual */ int peek(void);
			/*virtual */ int read(void);
			/*virtual */ void flush(void);

			size_t write(uint8_t data, bool last = false);
			size_t write(const uint8_t *buffer, size_t length) {
				size_t total_sent = 0;
				if (length <= 0)
					return 0;
				size_t sent;
				do {
					total_sent += sent;
					sent = write((uint8_t)*buffer++, /* last: */(length == 1));
					length -= sent;
				} while (length);

				return sent;
			};

			size_t write( const char *buffer)
			{
				return write((const uint8_t *)buffer, strlen(buffer));
			}

			template <typename T, size_t N>
			size_t write( const T(&buffer)[ N ], size_t x = N )
			{
				return write((const uint8_t *)buffer, N);
			}
			
			template <typename T>
			size_t write( const T *(&buffer), size_t x)
			{
			  return write((const uint8_t *)buffer, x);
			}
			
			size_t read(uint8_t *buffer, size_t length) {
				// blocking!
				expecting = length;

				if (buffer == NULL) { // simply requesting this number of bytes
					return 0;
				}

				size_t amt_read = 0;
				do {
					do {} while (expecting && rx_buffer.isEmpty()); // BLOCK! (wait for more data to come in, or close)
					// Note that the other side may NACK, setting expecting to 0

					if (!rx_buffer.isEmpty()) {
						*buffer++ = rx_buffer.getTail();
						++amt_read;

						// holding_on_receive shouldn't happen if the buffer is empty
						if (holding_on_receive) {
							_ackIfExpecting();
							holding_on_receive = false;
						}
					}
				} while (expecting);

				if (amt_read < length)
					*buffer = 0; // if we have room, add a NULL byte for string safety

				return amt_read;
			};
			
			// int read(int count);
			void setExpecting(int8_t count);
			
			template <typename T, size_t N>
			size_t read( T(&buffer)[ N ], size_t x = N )
			{
				return read((char *)buffer, N);
			}

			template <typename T>
			size_t read( T *(&buffer), size_t x )
			{
			  return read((uint8_t *)buffer, x);
			}
			
			void sync() { flush(); };
			
			// virtual size_t write(uint8_t) = 0;
			// using Print::write;
			
			// internal use
			void _receive(const uint8_t byte);
			void _ackIfExpecting();
			void _send();
			void _done();
		  
		}; // class Master

		enum { AllCall = 0x00 };

		class MasterType : public TWIBase {
		public:
			static uint8_t to_addr;
			
			MasterType();
			
			void open(uint8_t addr, uint8_t write_flags); // start talking (or be passive aggresive)
			// aliases
			void start(uint8_t addr, bool write) { open(addr, write); };
			void restart(uint8_t addr, bool write) { open(addr, write); };

			
			void close(); // stop talking (or being passive aggresive)
			// aliases
			void stop(uint8_t addr) { close(); };
		};
		
		enum {
			Writing           = 0,
			Reading           = 1 << 0,
			RestartAfter      = 1 << 1
		};
		
		
		enum SlaveCallbackMaskEnum {
			SlaveCallbackRXStart     = 1 << 1,
			SlaveCallbackTXStart     = 1 << 2,
			SlaveCallbackRXByte      = 1 << 3,
			SlaveCallbackRXFull      = 1 << 4,
			SlaveCallbackRXDone      = 1 << 5,
			SlaveCallbackTXEmpty     = 1 << 6,
			SlaveCallbackTXDone      = 1 << 7,
		};
		
		typedef uint8_t SlaveCallbackMask;

		class SlaveType : public TWIBase {
		public:
			SlaveCallbackMask mask;
			
			SlaveType();

			void open(uint8_t addr = 0x00); // start listening (speak when spoken to)

			void close(); // stop listening
			
			void setCallbackMask(SlaveCallbackMask mask);
		};
		
		extern MasterType Master;
		extern SlaveType Slave;
		
	} // namespace TWI
} // namespace Motate

#endif /* end of include guard: MOTATETWI_H_ONCE */
