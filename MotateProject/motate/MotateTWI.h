/*
  MotateTWI.hpp - TWI/I2C Library for the Motate system
  http://github.com/synthetos/motate/

  Copyright (c) 2018 Robert Giseburt

	This file is part of the Motate Library.

	This file ("the software") is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License, version 2 as published by the
	Free Software Foundation. You should have received a copy of the GNU General Public
	License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.

	As a special exception, you may use this file as part of a software library without
	restriction. Specifically, if other files instantiate templates or use macros or
	inline functions from this file, or you compile this file and link it with  other
	files to produce an executable, this file does not by itself cause the resulting
	executable to be covered by the GNU General Public License. This exception does not
	however invalidate any other reasons why the executable file might be covered by the
	GNU General Public License.

	THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
	WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
	SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MOTATETWI_H_ONCE
#define MOTATETWI_H_ONCE

#include <cinttypes>
#include "MotateCommon.h"
#include "MotateServiceCall.h"


/* After some setup, we call the processor-specific bits, then we have the
 * any-processor parts.
 *
 * The processor specific parts MUST define:
 *
 * template<pin_number twiSCKPinNumber, pin_number twiSDAPinNumber> using TWIGetHardware<twiSCKPinNumber, twiSDAPinNumber> =
 *  and whatever type that returns, such as:
 * template<uint8_t peripheralNumber> struct _TWIHardware
 *
 */

namespace Motate {
    enum TWIDeviceMode {
        // Address Size
        kTWI7BitAddress        = 0 << 2,
        kTWI10BitAddress       = 1 << 2
    }; // SPIDeviceMode

    struct TWIInterrupt : Interrupt {
    };

} // namespace Motate

#include <ProcessorTWI.h>

namespace Motate {


#pragma mark TWIBusDeviceBase
    /**************************************************
     *
     * TWI Bus Device Base, a base class for a member of a TWI Bus
     *
     **************************************************/

    struct TWIMessage;

    struct TWIBusDeviceBase
    {
        // store a link to the next device on the bus (maintained by the Bus)
        TWIBusDeviceBase *_next_device = 0;

        // set device options
        // virtual void setOptions() {};
        // queue message
        virtual void queueMessage(TWIMessage *msg) {};
        // return a value that can be used by hardware to select this device
        virtual uint32_t getAddress() { return 0; };
    };

    // useful verbose enums
    struct TWIMessage
    {
        enum {
            RemainAsserted = false,
            DeassertAfter = true,

            KeepTransaction = false,
            EndTransaction = true
        };

        enum class State {
            Idle = 0,
            Setup = 1,
            Sending = 2,
            Done = 3
        };

        uint8_t *tx_buffer;
        uint8_t *rx_buffer; // "pointer to uint8_t that is const"
        uint16_t size;
        bool deassert_after;
        bool immediate_deassert_after; // allows changing deassert_after from the callback
        bool ends_transaction;
        bool immediate_ends_transaction; // allows changing ends_transaction from the callback


        TWIBusDeviceBase *device;
        TWIMessage * volatile next_message;

        std::function<void(void)> message_done_callback;
        volatile State state = State::Idle;

        TWIMessage() {};

        TWIMessage *setup(uint8_t *new_tx_buffer, uint8_t *new_rx_buffer, const uint16_t new_size, const bool new_deassert_after, const bool new_ends_transaction) {
            tx_buffer = new_tx_buffer;
            rx_buffer = new_rx_buffer;
            size = new_size;
            deassert_after = new_deassert_after;
            ends_transaction = new_ends_transaction;
            state = State::Setup;

            return this;
        }
    };

#pragma mark TWIBus
    /**************************************************
     *
     * SPI Bus with arbitration
     *
     **************************************************/

    template<pin_number twiSCKPinNumber, pin_number twiSDAPinNumber, service_call_number svcCallNum>
    struct TWIBus
    {
        // Note these asserts will need to be revisited if we make a bit-banged TWI
        static_assert(IsTWISCKPin<twiSCKPinNumber>(),
                      "TWI SCK Pin is not on a hardware TWI.");

        static_assert(IsTWISDAPin<twiSDAPinNumber>(),
                      "TWI SDA Pin is not on a hardware TWI.");

        static_assert((TWISCKPin<twiSCKPinNumber>::twiNum == TWISDAPin<twiSDAPinNumber>::twiNum),
                      "TWI SCK and SDA pins are not all on the same TWI hardware peripheral.");

        TWISCKPin<twiSCKPinNumber> sckPin {};
        TWISDAPin<twiSDAPinNumber> sdaPin {};

        TWIGetHardware<twiSCKPinNumber, twiSDAPinNumber> hardware;

        ServiceCall<svcCallNum> message_manager;

        TWIBusDeviceBase *_first_device, *_current_transaction_device;
        TWIMessage * volatile _first_message;//, *_last_message;

        volatile bool sending = false; // as long as this is true, sendNextMessage() does nothing

        TWIBus() : hardware{} {
        }

        void addDevice(TWIBusDeviceBase *new_next) {
            if (_first_device == nullptr) {
                _first_device = new_next;
                return;
            }

            TWIBusDeviceBase *walker = nullptr;
            TWIBusDeviceBase *walker_next = _first_device;
            while (walker_next != nullptr) {
                if (new_next == walker_next) { return; }

                walker = walker_next;
                walker_next = walker->_next_device;
            }
            walker->_next_device = new_next;
        }

        void removeDevice(TWIBusDeviceBase *old) {
            if (_first_device == nullptr) {return;}

            if (_first_device == old) {
                _first_device = old->_next_device;
                return;
            }

            TWIBusDeviceBase *walker = _first_device;
            TWIBusDeviceBase *walker_next = _first_device->_next_device;
            while (walker_next != nullptr) {
                if (walker_next == old) {
                    // WARNING: Assumes walker_next (a.k.a. old) still has a valid _next_device
                    walker->_next_device = walker_next->_next_device;
                    return;
                }

                walker = walker_next;
                walker_next = walker->_next_device;
            }

            // we didn't find it. Oh well.
        }

        // WARNING!!
        // This must be called later, outside of the contructors, to ensure that all dependencies are contructed.
        void init() {
            // setup message_manager system call handler and priority
            message_manager.setInterruptHandler([&]() {
                this->sendNextMessageActual();
            });
            message_manager.setInterrupts(kInterruptPriorityLow);

            // ask the hardware to init
            hardware.init();
            hardware.setInterruptHandler([&](uint16_t interruptCause) { // use a closure
                this->twiInterruptHandler(interruptCause);
            });
            hardware.setInterrupts(kInterruptPriorityLow); // enable interrupts and set the priority
            hardware.enable();
        };

        // This function uses a ServiceCall to jump to the correct interrupt level, which may be higher or lower than the current level.
        void sendNextMessage() {
            message_manager.call();
            //sendNextMessageActual();
        }

        void sendNextMessageActual() {
            if (sending) { return; }
            while (_first_message && (TWIMessage::State::Done == _first_message->state)) {
                auto done_message = _first_message;
                _first_message = _first_message->next_message;
                done_message->next_message = nullptr;
            }
            if (_first_message == nullptr) { return;}
            if (TWIMessage::State::Sending == _first_message->state) { return; }

            if (_current_transaction_device != nullptr) {
                // the next message we send must be from the _current_transaction_device
                if (!(_first_message->device == _current_transaction_device)) {
                    // now we'll make a pass throught the messages, looking for one
                    // for the _current_transaction_device
                    TWIMessage *previous_message = _first_message;
                    TWIMessage *walker_message = _first_message->next_message;

                    while (walker_message != nullptr) {
                        if (walker_message->device == _current_transaction_device) {
                            // we have our actual next message, we'll pop it to the first position
                            previous_message->next_message = walker_message->next_message;
                            walker_message->next_message = _first_message;
                            _first_message = walker_message;
                            break;
                        }

                        previous_message = walker_message;
                        walker_message = walker_message->next_message;
                    }

                    if (walker_message == nullptr) {
                        // we have to wait for a new message to be queued up
                        return;
                    }
                }
            }

#ifdef IN_DEBUGGER
            if (TWIMessage::State::Setup != _first_message->state) {
                __asm__("BKPT"); // SPI about to send non-Setup message
            }
#endif

            sending = true;
            _first_message->state = TWIMessage::State::Sending;
            _current_transaction_device = _first_message->device;
            hardware.setChannel(_current_transaction_device->getChannel());
            hardware.startTransfer(_first_message->tx_buffer, _first_message->rx_buffer, _first_message->size);
        }

        void twiInterruptHandler(uint16_t interruptCause) {
            // This bears stating, even though it's somewhat obvious:
            // This entire function is in an interrupt (higher priority) context, and will occasionally
            // interupt other code that interacts with the same structures.

            // So, we have to be careful not to move something out from under the other code.

            if (interruptCause & SPIInterrupt::OnTxReady) {
                // ready to transfer...
            }

            if (interruptCause & SPIInterrupt::OnRxReady) {
                // something is in the buffer right now...
            }

            if (interruptCause & (SPIInterrupt::OnTxTransferDone | SPIInterrupt::OnRxTransferDone)) {
                // Check that we're done with all transmission...
                if (!hardware.doneReading()) {
                    return;
                }
                if (!hardware.doneWriting()) {
                    return;
                }

                // This needs to be cleaned up:
                hardware._disableOnTXTransferDoneInterrupt();
                hardware._disableOnRXTransferDoneInterrupt();


#ifdef IN_DEBUGGER
                if (nullptr == _first_message) {
                    __asm__("BKPT"); // no first message!?
                }
#endif

                // _first_message is done sending.
                // Go ahead and pop it from the list and reset (partially)
                auto this_message = _first_message;
                while (this_message && (TWIMessage::State::Done == this_message->state)) {
                    this_message = this_message->next_message;
                }

                if (this_message && (TWIMessage::State::Sending == this_message->state)) {
                    // Then grab the (only) Sending message and mark it Done, then call it's done callback.
                    this_message->state = TWIMessage::State::Done;

                    // Set the values for *this* message before the callback, so
                    // the callback can re-queue with different values AND tell us
                    // how to handle the rest of this transaction. With these defaulted
                    // like this, the callback can do nothing and get the original
                    // behavior the message was configured for.
                    this_message->immediate_ends_transaction = this_message->ends_transaction;
                    this_message->immediate_deassert_after = this_message->deassert_after;

                    // Call the message's callback, if any, THEN check immediate_ends_transaction
                    // and immediate_deassert_after, since the callback might decide to change those.

                    // Ignore ends_transaction and deassert_after, since those are for the next queueing
                    // of the message - which may happen in the callback as well.

                    // IMPORTANT NOTE: the callback may call sendNextMessage(), so we
                    //   keep sending at true to prevent issues.

                    if (this_message->message_done_callback) {
                        this_message->message_done_callback();
                    }

                    if (this_message->immediate_ends_transaction) {
                        _current_transaction_device = nullptr;
                    }

                    if (this_message->immediate_deassert_after) {
                        hardware.deassert();
                    }
                }
                sending = false; // we can now allow more sending
                //sendNextMessageActual();
                message_manager.call();
            }
        };


#pragma mark TWIBusDevice (inside TWIBus)
        /**************************************************
         *
         * TWI Bus Device, a member of an TWI Bus.
         *
         **************************************************/

        struct TWIBusDevice : TWIBusDeviceBase
        {
            // Since we are defining this INSIDE the TWIBus struct, we'll use TWIBus internals liberally
            TWIBus<twiSCKPinNumber, twiSDAPinNumber, svcCallNum> * const _spi_bus;

            uint32_t _twi_address; // the chip select number

            template <typename chipSelectType>
            constexpr TWIBusDevice(TWIBus<spiMISOPinNumber, spiMOSIPinNumber, spiSCKPinNumber, svcCallNum> *parent_bus, const chipSelectType &cs, const uint32_t baud, const uint16_t options, uint32_t min_between_cs_delay_ns, uint32_t cs_to_sck_delay_ns, uint32_t between_word_delay_ns) : _spi_bus {parent_bus}
            {
                _cs_number = cs.csNumber;
                _cs_value  = cs.csValue;

                _spi_bus->addDevice(this);
                _spi_bus->hardware.setUsingCSDecoder(cs.usesDecoder);

                this->setOptions(baud, options, min_between_cs_delay_ns, cs_to_sck_delay_ns, between_word_delay_ns);
            };

            // prevent copying or deleting
            TWIBusDevice(const TWIBusDevice&) = delete;

            // update the bus upon deletion
            ~TWIBusDevice() {
                _spi_bus->removeDevice(this);
            };

            // build move constructor
            TWIBusDevice(TWIBusDevice&& other) : _spi_bus{other._spi_bus}, _cs_number{other._cs_number}, _cs_value{other._cs_value} {
                // since we just changed addresses, we'll let the old one deregister, but we must register this one
                _spi_bus->addDevice(this);
            };

            // set device options
            void setOptions (const uint32_t baud, const uint16_t options, uint32_t min_between_cs_delay_ns, uint32_t cs_to_sck_delay_ns, uint32_t between_word_delay_ns) override {
                _spi_bus->hardware.setChannelOptions(_cs_number, baud, options, min_between_cs_delay_ns, cs_to_sck_delay_ns, between_word_delay_ns);
            };

            // queue message
            void queueMessage (TWIMessage *msg) override {
                msg->device = this;
                if (_spi_bus->_first_message == nullptr) {
                    _spi_bus->_first_message = msg;
                    //_spi_bus->_last_message = msg;
                }
                else {
                    TWIMessage *walker_message = _spi_bus->_first_message;
                    while ((walker_message != msg) && (walker_message->next_message != nullptr)) {
                        walker_message = walker_message->next_message;
                    }

                    if (walker_message != msg) {
                        walker_message->next_message = msg;
                    }
                }

                // Either we just queued the first message, OR we *might* have
                // just queued a message for the current transaction
                // that has stalled, waiting for more messages.

                // In either case, we want to:
                _spi_bus->sendNextMessage();
            };

            uint32_t getChannel() override { return _cs_value; };
        };

        template <typename chipSelectType>
        constexpr TWIBusDevice getDevice(chipSelectType &&cs, const uint32_t baud, const uint16_t options, uint32_t min_between_cs_delay_ns, uint32_t cs_to_sck_delay_ns, uint32_t between_word_delay_ns)
        {
            return {this, std::move(cs), baud, options, min_between_cs_delay_ns, cs_to_sck_delay_ns, between_word_delay_ns};
        }

//        void _TMP_setUsingCSDecoder(bool v) { hardware.setUsingCSDecoder(v); };
//
//        void _TMP_setChannelOptions(const uint8_t channel, const uint32_t baud, const uint16_t options, uint32_t min_between_cs_delay_ns, uint32_t cs_to_sck_delay_ns, uint32_t between_word_delay_ns) {
//        };
//
//        bool _TMP_startTransfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size) {
//            return hardware.startTransfer(tx_buffer, rx_buffer, size);
//        };
//
//        void _TMP_setChannel(const uint8_t channel) {
//            hardware.setChannel(channel);
//        };
//
//        int16_t _TMP_write(uint8_t value, int16_t &readValue, const bool lastXfer = false) {
//            return hardware.write(value, readValue, lastXfer);
//        };
//
//        void _TMP_setMsgDone(std::function<void(void)> &&handler) {
//            message_done_callback = std::move(handler);
//        }


    }; // TWIBus

} // namespace Motate
#endif /* end of include guard: MOTATETWI_H_ONCE */