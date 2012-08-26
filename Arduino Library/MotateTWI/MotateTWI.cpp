/*
  Motate.cpp - Library for the Arduino-compatible Motate system
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

#include <MotateTWI.h>
#include <../Motate/MotatePins.h>
#include <avr/interrupt.h>
#include <pins_arduino.h>
#include <util/twi.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


namespace Motate {
	namespace TWI {
#if 0
	static volatile uint8_t twi_state;
	static volatile uint8_t twi_slarw;
	static volatile uint8_t twi_sendStop;			// should the transaction end with a stop
	static volatile uint8_t twi_inRepStart;			// in the middle of a repeated start

	static void (*twi_onSlaveTransmit)(void);
	static void (*twi_onSlaveReceive)(uint8_t*, int);
#endif

	enum {
		NACK = 0,
		ACK = 1
	};

	inline void _twi_reply(const uint8_t ack)
	{
		// transmit master read ready signal, with or without ack
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | (ack ? _BV(TWEA) : 0);
	}

	inline void _twi_stop()
	{
		// send stop condition
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTO);
	}

	volatile uint8_t twi_error = 0xFF;
	// bool Motate::TWI::TWIBase::is_open = false;
	uint8_t Motate::TWI::MasterType::to_addr = 0;

/**** TWIBase ****/
	TWIBase::TWIBase() : is_open(false), is_busy(false), do_stop(false), reopen(false), ready_to_send(false), holding_on_receive(false), bump_tx(false), writing(false), expecting(0), tx_buffer(), rx_buffer() {
		// static here means it's a global that can only be used here
		static bool did_init = false; // do this only once
		
		if (!did_init) {
			// turn on pullups
			Pin<SDA> sda = InputWithPullup;
			Pin<SCL> scl = InputWithPullup;
			
			// pin13 = Output;
			// pin13 = 0;
			// 
			// pin12 = Output;
			// pin12 = 0;
			// 
			// pin11 = Output;
			// pin11 = 0;
			
			// From twi.c in the Arduino distribuion:
			/* twi bit rate formula from atmega128 manual pg 204
			SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
			note: TWBR should be 10 or higher for master mode
			It is 72 for a 16mhz Wiring board with 100kHz TWI */
			
			/* Additional Note: ATMega32U4 datasheet says (p. 230)
			   SCL Freq = (CPU Clock Frequency)/(16 + (2*TWBR) * 4^(TWPS))
			   This means that the TWPS (bottom two bits of TWSR) is also a factor.
			   TWSR of 0b00 (default) is a prescale of 1, so for the ATMegs32U4
			    (and others?) we need to compensate.
			*/
		  cbi(TWSR, TWPS0);
		  cbi(TWSR, TWPS1);
			
			
			#if defined(__AVR_ATmega32U4__)
			uint8_t br = ((F_CPU / TWI_FREQ) - 16) / 8;
			#else
			uint8_t br = ((F_CPU / TWI_FREQ) - 16) / 2;
			#endif
			TWBR = br >= 10 ? br : 10;
			// enable twi module, acks, and twi interrupt
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA);

			did_init = true;
		}
	}
	
	int TWIBase::available(void)
	{
		return rx_buffer.getUsedCount();
	}
	
	int TWIBase::peek(void) {
		return !rx_buffer.isEmpty() ? rx_buffer.peek() : -1;
	}
	
	void TWIBase::setExpecting(int8_t count) {
		// We account for what may have come in between open() and setExpecting().
		expecting = count-rx_buffer.getUsedCount();
	}
	
	int TWIBase::read() {
		// non-blocking!
		// pin12 = 1;
		// pin12 = 0;
		
		if (!rx_buffer.isEmpty()) {
			// holding_on_receive shouldn't happen if the buffer is empty
			if (holding_on_receive) {
				_ackIfExpecting();
				holding_on_receive = false;
			}
			char c = rx_buffer.getTail();
			return c;
		}
		return -1;
	}
	
	size_t TWIBase::write(uint8_t data, bool last/* = false*/) {
		// pin12 = 1;
		// pin12 = 0;

		if (tx_buffer.isFull()) {
			return 0;
		}
		
		writing = !last;
		tx_buffer.setHead(data);
		
		return 1; // assume we'll write it
	}
	
	void TWIBase::flush(void) {
		expecting = 0;
		in_flush = true;
		// if (ready_to_send)
		// 	_send();

		// wait while the buffer is emptied by the interrupt
		do {
			// pin11 = 1;
			// pin12 = 1;
			// pin12 = 0;
			// pin11 = 0;
		} while(bump_tx || !tx_buffer.isEmpty());
		in_flush = false;
	}
	
	void TWIBase::_receive(const uint8_t byte) {
		rx_buffer.setHead(byte);
		--expecting;
		
		// Clock slewing:
		// We intentionally don't ACK/NACK until the buffer is no longer full.
		if (expecting && rx_buffer.isFull()) {
			holding_on_receive = true;
			// let the client know the buffer is full
			return;
		}

		_ackIfExpecting();
	}
	
	void TWIBase::_ackIfExpecting() {
		
		// pin12 = 1;
		// if (expecting == 1) {
		// 	pin11 = 1;
		// 	pin11 = 0;
		// }
		// pin12 = 0;
		
		
		// We want to send a NACK with *reception* of the next-to-last byte
		// That nack will be passed with the recption of the next byte -- the last one.
		_twi_reply(expecting != 1);
	}
	
	void TWIBase::_send() {
		if (bump_tx) {
			bump_tx = false;
			tx_buffer.bumpTail();
			// pin11 = 1;
			// pin11 = 0;
		}
		
		uint8_t used = tx_buffer.getUsedCount();
		
		// if there is data to send, send it, otherwise wait
		if (!writing && used > 0){
			// copy data to output register and ack
			TWDR = tx_buffer.peek(); // get out fast, the hardware is waiting
			_twi_reply(used > 0);
			bump_tx = true;
			// pin12 = 1;
			// pin12 = 0;
		} else if (!writing) {
			_done();
		}
	}

	void TWIBase::_done() {
		// _twi_reply(NACK);
		// pin11 = 1;
		// pin13 = 1;
		// pin13 = 0;
		// pin11 = 0;
		
		// We're done writing.
		// Here is where we handle STOP, START, RESTART
		
		// We are technically done writing, note that
		is_open = false;
		
		if (reopen) {
			reopen = false;

			// pin12 = 1;
			// pin12 = 0;

			// wait for any pending stop conditions to be exectued on bus
			// TWINT is not set after a stop condition!
			do {} while(TWCR & _BV(TWSTO));

			// Prepare the start
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);	// enable INTs
			
			return;
		} else
		if (do_stop) {
			// pin13 = 1;
			// pin13 = 0;
			_twi_stop();
			do_stop = false;
		}
		else {
			// pin11 = 1;
			// pin11 = 0;
		}
		
		is_busy = false;
	}
	
/**** Master ****/
	MasterType::MasterType() : TWIBase() {
		to_addr = 0;
	}
	
	void MasterType::open(uint8_t addr, uint8_t write_flags) { // start talking (or be passive aggresive)
		// pin13 = 1;
		// pin13 = 0;
		if (is_busy) {
			/* We're already open ... no problem.
			   If we're going the same direction, then we are forced to flush and block.
			   Otherwise, we just set flags to tell us when to de when we're done.
			*/
			
			// But first, we have to make sure we actually are open!
			do {
				// pin11 = 1;
				// pin11 = 0;
			} while (!is_open);
				
			if ((write_flags & 0x01) == (to_addr & 0x01)) {
				flush();
				reopen = false;
			} else {
				reopen = true;
			}
		}
		
		is_busy = true;
		
		// wait for any pending stop conditions to be exectued on bus
		// TWINT is not set after a stop condition!
		do {} while(TWCR & _BV(TWSTO));

		// store the address to send after START, with read/write bit set
		to_addr = ( addr<<1 ) | (write_flags & 0x01);
		
		if (!(write_flags & RestartAfter))
			do_stop = true;
		
		if (reopen)
			return;
		
		twi_error = 0xFF;
		// We DON'T set is_open here. We initialize it, and clear it when we really close.
		
		// send start condition
		TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);	// enable INTs
	}
	
	void MasterType::close() { // stop talking (or being passive aggresive)
		// pin12 = 1;
		// pin13 = 1;
		// pin13 = 0;
		// pin12 = 0;
		
		if (is_busy) {
			// All we do is suggest STOPping. :)
			// We might not even be is_open yet!
			do_stop = true;
			return;
		}

		// pin12 = 1;
		// pin12 = 0;
		
		// otherwise, we're done, so send the stop
		_twi_stop();
		do_stop = false;

		// // wait until we're done sending
		// if (twi_error == 0xFF) {
		// 	flush();
		// } else {
		// 	// clear the TX buffer :'O
		// 	tx_buffer.clear();
		// 	
		// 	// leave the RX buffer in case we got something
		// }
		// 
		// is_open = false;
		// // send a stop
		// _twi_stop();
	}


/**** Slave ****/
	SlaveType::SlaveType() : TWIBase(), mask(/*Callback for everything!*/ 0xFF) {
		is_open = false;
	}
	
	void SlaveType::open(uint8_t addr) { // start listening (speak when spoken to)
		twi_error = 0xFF;
		TWAR = addr == 0x00 ? 0x01 : (addr << 1);
		
		is_busy = true;

		// We re-use expecting here to indicate that we might still have writes
		// flush() sets expecting to 0
		expecting = 1;
	}
	
	void SlaveType::close() { // stop listening
		do_stop = true; // note that we want to stop listening after final rx
	}
	
	void setCallbackMask(SlaveCallbackMask mask) {
		
	}
	
	
	// Create the "singletons"
	MasterType Master;
	SlaveType Slave;
	
	void masterRx() __attribute__((weak));
	void masterRx() {}

	
	} // namespace TWI
} // namespace Motate

void slaveRx(Motate::TWI::SlaveCallbackMask mask) __attribute__((weak));
void slaveRx(Motate::TWI::SlaveCallbackMask mask) {}

using namespace Motate::TWI;

/* In order to optimize the compilation of this ISR, we
   need the swtich cases to be in order, with only 1 between
   between them, and in order. This optimizes code size over speed slightly.

  If we shift by three bits, they end up in order, in *this* order:
0x08 >> 3 = 001 (0x01) TW_START
0x10 >> 3 = 002 (0x02) TW_REP_START
0x18 >> 3 = 003 (0x03) TW_MT_SLA_ACK
0x20 >> 3 = 004 (0x04) TW_MT_SLA_NACK
0x28 >> 3 = 005 (0x05) TW_MT_DATA_ACK
0x30 >> 3 = 006 (0x06) TW_MT_DATA_NACK
0x38 >> 3 = 007 (0x07) TW_MR_ARB_LOST TW_MT_ARB_LOST
0x40 >> 3 = 008 (0x08) TW_MR_SLA_ACK
0x48 >> 3 = 009 (0x09) TW_MR_SLA_NACK
0x50 >> 3 = 010 (0x0a) TW_MR_DATA_ACK
0x58 >> 3 = 011 (0x0b) TW_MR_DATA_NACK
0x60 >> 3 = 012 (0x0c) TW_SR_SLA_ACK
0x68 >> 3 = 013 (0x0d) TW_SR_ARB_LOST_SLA_ACK
0x70 >> 3 = 014 (0x0e) TW_SR_GCALL_ACK
0x78 >> 3 = 015 (0x0f) TW_SR_ARB_LOST_GCALL_ACK
0x80 >> 3 = 016 (0x10) TW_SR_DATA_ACK
0x88 >> 3 = 017 (0x11) TW_SR_DATA_NACK
0x90 >> 3 = 018 (0x12) TW_SR_GCALL_DATA_ACK
0x98 >> 3 = 019 (0x13) TW_SR_GCALL_DATA_NACK
0xa0 >> 3 = 020 (0x14) TW_SR_STOP
0xa8 >> 3 = 021 (0x15) TW_ST_SLA_ACK
0xb0 >> 3 = 022 (0x16) TW_ST_ARB_LOST_SLA_ACK
0xb8 >> 3 = 023 (0x17) TW_ST_DATA_ACK
0xc0 >> 3 = 024 (0x18) TW_ST_DATA_NACK
0xc8 >> 3 = 025 (0x19) TW_ST_LAST_DATA
0xf8 >> 3 = 031 (0x1f) TW_NO_INFO
*/

SIGNAL(TWI_vect)
{
	// TW_STATUS masks off the prescale (bottom two) bits of the status
	switch(TW_STATUS >> 3){
		// All Master
		case TW_START >> 3:     // sent start condition
		case TW_REP_START >> 3: // sent repeated start condition
				// copy device address and r/w bit to output register and ack
			TWDR = Master.to_addr;
			_twi_reply(ACK);
			break;


		// Master Transmitter
		case TW_MT_SLA_ACK >> 3:  // slave receiver acked address
			// Open for business
			Master.is_open = true;
			// Motate::pin13 = 1;
			// Motate::pin12 = 1;
			// Motate::pin12 = 0;
			// Motate::pin13 = 0;
			
			// if there is data to send, send it, otherwise stop 
			Master._send();
			break;

		case TW_MT_SLA_NACK >> 3:  // address sent, nack received
			twi_error = TW_STATUS;
			Master.close();
			break;

		case TW_MT_DATA_ACK >> 3: // slave receiver acked data
			// if there is data to send, send it, otherwise stop 
			// Motate::pin13 = 1;
			// Motate::pin13 = 0;
			Master._send();
			break;

		case TW_MT_DATA_NACK >> 3: // data sent, nack received
			// twi_error = TW_STATUS;
			// Note that we're done writing
			Master.writing = false;
			
			// Now handle the "done writing" scenarios
			Master._done();
			break;
			
		case TW_MT_ARB_LOST >> 3: // lost bus arbitration
			// Record this, or ignore?
			// twi_error = TW_MT_ARB_LOST;
			
			// Reset (write to 1) TWINT to wait for the bus to become ready again.
			// Hardware will watch and try again. 
			TWCR = _BV(TWINT) | _BV(TWEA) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
			break;


		// Master Receiver
		case TW_MR_SLA_ACK >> 3:  // address sent, ack received
			Master._ackIfExpecting();
			break;

		case TW_MR_SLA_NACK >> 3: // address sent, nack received
			// Sometimes this will not be an error, but it needs to be detectable.
			twi_error = TW_MR_SLA_NACK;

			// we will get no data
			Master.expecting = 0;
			break;
			
		case TW_MR_DATA_ACK >> 3: // data received, ack sent
			// put byte into buffer
			// Motate::pin13 = 1;
			// Motate::pin13 = 0;
			Master._receive(TWDR);
			break;
			
		case TW_MR_DATA_NACK >> 3: // data received, nack sent
			// put final byte into buffer
			Master.rx_buffer.setHead(TWDR);
			// we will get no more
			Master.expecting = 0;
			
			Master._done();
			break;

		// TW_MR_ARB_LOST handled by TW_MT_ARB_LOST case


		// Slave Receiver
		case TW_SR_SLA_ACK >> 3:          // addressed, returned ack
			// enter slave receiver mode
			Slave.rx_buffer.clear();
			Slave.is_open = true;
			if (Slave.mask & SlaveCallbackRXStart)
				slaveRx(SlaveCallbackRXStart);
			_twi_reply(ACK); // Read the next byte then ACK
			break;

		// Someone called in while we were dialing out
		// NOTE THIS!?!?!
		case TW_SR_ARB_LOST_SLA_ACK >> 3:   // lost arbitration, returned ack
				// enter slave receiver mode
				Slave.rx_buffer.clear();
				Slave.is_open = true;
				if (Slave.mask & SlaveCallbackRXStart)
					slaveRx(SlaveCallbackRXStart);
				_twi_reply(ACK); // Read the next byte then ACK
				break;

		case TW_SR_GCALL_ACK >> 3:        // addressed generally, returned ack
			// enter slave receiver mode
			Slave.rx_buffer.clear();
			Slave.is_open = true;
			if (Slave.mask & SlaveCallbackRXStart)
				slaveRx(SlaveCallbackRXStart);
			_twi_reply(ACK); // Read the next byte then ACK
			break;

		// Someone called in while we were dialing out
		// NOTE THIS!?!?!
		case TW_SR_ARB_LOST_GCALL_ACK >> 3: // lost arbitration, returned ack
			// enter slave receiver mode
			Slave.rx_buffer.clear();
			Slave.is_open = true;
			if (Slave.mask & SlaveCallbackRXStart)
				slaveRx(SlaveCallbackRXStart);
			_twi_reply(ACK); // Read the next byte then ACK
			break;

		case TW_SR_DATA_ACK >> 3:         // data received, returned ack
			// put byte into buffer
			Slave._receive(TWDR);
			if (Slave.mask & SlaveCallbackRXByte)
				slaveRx(SlaveCallbackRXByte);
			break;

		case TW_SR_DATA_NACK >> 3:        // data received, returned nack
			Slave.expecting = 0;
			// nack back at master
			_twi_reply(NACK);
			break;

		case TW_SR_GCALL_DATA_ACK >> 3:   // data received generally, returned ack
			// put byte into buffer
			Slave._receive(TWDR);
			break;

		case TW_SR_GCALL_DATA_NACK >> 3:  // data received generally, returned nack
			Slave.expecting = 0;
			// nack back at master
			_twi_reply(NACK);
			break;

		case TW_SR_STOP >> 3:             // stop or repeated start condition received
			Slave.expecting = 0;
			Slave.is_open = false;
			Slave.is_busy = false;

			if (Slave.mask & SlaveCallbackRXDone)
				slaveRx(SlaveCallbackRXDone);
			
			// ack future responses, if told to
			_twi_reply(!Slave.do_stop);
			Slave.do_stop = false;
			break;


		// Slave Transmitter
		case TW_ST_SLA_ACK >> 3:          // addressed, returned ack
			// copy data to output register
			Slave._send();
			break;

		case TW_ST_ARB_LOST_SLA_ACK >> 3: // arbitration lost, addressed, returned ack
			//NOTE THIS!?!?
			// copy data to output register
			Slave._send();
			break;
		
		// transmit first byte from buffer
		case TW_ST_DATA_ACK >> 3:         // byte sent, ack returned
			// copy data to output register
			Slave._send();
			break;

		case TW_ST_DATA_NACK >> 3:       // received nack, we are done
		case TW_ST_LAST_DATA >> 3:       // received ack, but we are done already!
			Slave.tx_buffer.clear();
			Slave.is_open = false;
			Slave.is_busy = false;

			// ack future responses, if told to
			_twi_reply(!Slave.do_stop);
			Slave.do_stop = false;
			break;

		// All
		// NOTE: These last two break the off-by-one optimization (~30% code size reduction)
		// Se we simply add an if here. It's a little ugly, but SO worth it.
		
		default:   // no state information
			if (TW_STATUS == TW_BUS_ERROR) {
				twi_error = TW_BUS_ERROR;
				Master.is_open = false;
				Master.is_busy = false;
				Master.close();

				Slave.is_open = false;
				Slave.is_busy = false;
				Slave.close();
			}
			
			// TW_NO_INFO is all tha'ts left. We ignore it anyway.
			break;

		// case TW_BUS_ERROR: // bus error, illegal stop/start
		// 	twi_error = TW_BUS_ERROR;
		// 	Master.close();
		// 	Slave.close();
		// 	break;
	}
}
