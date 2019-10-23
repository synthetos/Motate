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
#include <atomic>
#include "MotateCommon.h"
#include "MotateServiceCall.h"


/* After some setup, we call the processor-specific bits, then we have the
 * any-processor parts.
 *
 * The processor specific parts MUST define:
 *
 * template<pin_number twiSCKPinNumber, pin_number twiSDAPinNumber> using TWIGetHardware<twiSCKPinNumber,
 * twiSDAPinNumber> = and whatever type that returns, such as: template<uint8_t peripheralNumber> struct _TWIHardware
 *
 */

namespace Motate {
enum class TWIDeviceAddressSize : uint8_t {
    // Address Size
    k7Bit,
    k10Bit
};  // TWIDeviceAddressSize

struct TWIAddress {
    const uint16_t             address = 0;
    const TWIDeviceAddressSize size    = TWIDeviceAddressSize::k7Bit;
};

enum class TWIInternalAddressSize : uint8_t {
    // Address Size
    kNone = 0,
    k1Byte,
    k2Bytes,
    k3Bytes
};  // TWIInternalAddressSize

struct TWIInternalAddress {
    uint32_t               address = 0;
    TWIInternalAddressSize size    = TWIInternalAddressSize::kNone;
};


class TWIInterrupt : public Interrupt {
   public:
    static constexpr uint16_t OnNACK          = 1 << 10;
    static constexpr uint16_t OnNoAcknowledge = 1 << 10;
};

struct TWIInterruptCause : public InterruptCause {
   public:
    bool isNACK() const { return value_ & TWIInterrupt::OnNACK; }
    void setNACK() { value_ |= TWIInterrupt::OnNACK; }
    void clearNACK() { value_ &= ~TWIInterrupt::OnNACK; }
};

struct TWIInterruptHandler {
    virtual void handleTWIInterrupt(const TWIInterruptCause& interruptCause);
};

}  // namespace Motate

#include <ProcessorTWI.h>

namespace Motate {

#pragma mark TWIBusDeviceBase
/**************************************************
 *
 * TWI Bus Device Base, a base class for a member of a TWI Bus
 *
 **************************************************/

struct TWIMessage;

struct TWIBusDeviceBase {
    // store a link to the next device on the bus (maintained by the Bus)
    TWIBusDeviceBase* _next_device = 0;

    // set device options
    // virtual void setOptions() {};
    // queue message
    virtual void queueMessage(TWIMessage* msg){};
    // return a value that can be used by hardware to select this device
    virtual const TWIAddress& getAddress() const;
};

// useful verbose enums
struct TWIMessage {
    enum class Instruction {
        kNormal,
        kWithoutStop,  // Don't send a STOP after, making the next message have a RESTART
    };

    enum class State { kIdle, kSetup, kSending, kDone };

    enum class Direction { kTX, kRX };

    uint8_t* buffer = nullptr;
    uint16_t size   = 0;

    TWIBusDeviceBase*        device                = nullptr;
    std::atomic<TWIMessage*> next_message          = nullptr;
    Instruction              instruction           = Instruction::kNormal;
    Direction                direction             = Direction::kTX;

    TWIInternalAddress internal_address;

    std::function<void(bool)> message_done_callback;
    std::atomic<State>        state = State::kIdle;

    TWIMessage(){};

    void setup(uint8_t*                   new_buffer,
               const uint16_t             new_size,
               const Direction            new_direction,
               const TWIInternalAddress&& new_ia           = {},
               const Instruction          new_instruction  = Instruction::kNormal,
               TWIMessage* const          new_next_message = nullptr) {
        auto state_snapshot = state.load();
        if (state_snapshot != State::kIdle && state_snapshot != State::kDone) {
            #ifdef IN_DEBUGGER
            __asm__("BKPT");  // about to send non-Setup message
            #endif
            // We're about to lose a message!
        }
        buffer       = new_buffer;
        direction    = new_direction;
        size         = new_size;
        instruction  = new_instruction;
        next_message = new_next_message;

        internal_address = std::move(new_ia);

        state = State::kSetup;
    }
};

#pragma mark TWIBus
/**************************************************
 *
 * SPI Bus with arbitration
 *
 **************************************************/

template <pin_number twiSCKPinNumber, pin_number twiSDAPinNumber>
struct TWIBus : virtual TWIInterruptHandler, virtual ServiceCallEventHandler {
    using bus_type = TWIBus<twiSCKPinNumber, twiSDAPinNumber>;
    // Note these asserts will need to be revisited if we make a bit-banged TWI
    static_assert(IsTWISCKPin<twiSCKPinNumber>(), "TWI SCK Pin is not on a hardware TWI.");

    static_assert(IsTWISDAPin<twiSDAPinNumber>(), "TWI SDA Pin is not on a hardware TWI.");

    static_assert((TWISCKPin<twiSCKPinNumber>::twiNum == TWISDAPin<twiSDAPinNumber>::twiNum),
                  "TWI SCK and SDA pins are not all on the same TWI hardware peripheral.");

    TWISCKPin<twiSCKPinNumber> sckPin{};
    TWISDAPin<twiSDAPinNumber> sdaPin{};

    TWIGetHardware<twiSCKPinNumber, twiSDAPinNumber> hardware;

    ServiceCall message_manager;

    TWIBusDeviceBase *_first_device, *_current_transaction_device;
    std::atomic<TWIMessage*> _first_message, _last_message;

    std::atomic<bool> sending = false;  // as long as this is true, sendNextMessage() does nothing

    TWIBus() : hardware{} {}

    void addDevice(TWIBusDeviceBase* new_next) {
        if (_first_device == nullptr) {
            _first_device = new_next;
            return;
        }

        TWIBusDeviceBase* walker      = nullptr;
        TWIBusDeviceBase* walker_next = _first_device;
        while (walker_next != nullptr) {
            if (new_next == walker_next) {
                return;
            }

            walker      = walker_next;
            walker_next = walker->_next_device;
        }
        walker->_next_device = new_next;
    }

    void removeDevice(TWIBusDeviceBase* old) {
        if (_first_device == nullptr) {
            return;
        }

        if (_first_device == old) {
            _first_device = old->_next_device;
            return;
        }

        TWIBusDeviceBase* walker      = _first_device;
        TWIBusDeviceBase* walker_next = _first_device->_next_device;
        while (walker_next != nullptr) {
            if (walker_next == old) {
                // WARNING: Assumes walker_next (a.k.a. old) still has a valid _next_device
                walker->_next_device = walker_next->_next_device;
                return;
            }

            walker      = walker_next;
            walker_next = walker->_next_device;
        }

        // we didn't find it. Oh well.
    }

    // WARNING!!
    // This must be called later, outside of the constructors, to ensure that all dependencies are constructed.
    void init() {
        // setup message_manager system call handler and priority
        message_manager.setInterruptHandler(this);   // will call this->handleServiceCallEvent()
        message_manager.setInterrupts(kInterruptPriorityLowest);

        // ask the hardware to init
        hardware.init();
        hardware.setInterruptHandler(this); // will call this->handleTWIInterrupt(...)
        hardware.setInterrupts(kInterruptPriorityLow);  // enable interrupts and set the priority
        // hardware.enable();
    };

    // This function uses a ServiceCall to jump to the correct interrupt level, which may be higher or lower than the
    // current level.
    void sendNextMessage() {
        message_manager.call();
        // sendNextMessageActual();
    }

    void handleServiceCallEvent() override {
        if (sending.load()) {
            return;
        }
        // We'll load this atomically, ensuring to get a fresh copy,
        // and cache a local copy to work on for the rest of this routine
        // (if it changes, somethng went wrong here.)
        auto first_message = _first_message.load(std::memory_order_acquire);
        if (first_message == nullptr) {
            return;
        }
#ifdef IN_DEBUGGER
        if (TWIMessage::State::kSetup != first_message->state) {
            __asm__("BKPT");  // about to send non-Setup message
        }
#endif
        if (TWIMessage::State::kSending == first_message->state) {
            return;
        }

        sending.store(true);
        first_message->state        = TWIMessage::State::kSending;
        _current_transaction_device = first_message->device;
        hardware.setAddress(_current_transaction_device->getAddress(), first_message->internal_address);
        if (!hardware.startTransfer(first_message->buffer, first_message->size,
                                    first_message->direction == TWIMessage::Direction::kRX)) {
            __asm__("BKPT");  // about to send non-Setup message
        }
    }

    void handleTWIInterrupt(const TWIInterruptCause& interruptCause) override {
        // This bears stating, even though it's somewhat obvious:
        // This entire function is in an interrupt (higher priority) context, and will occasionally
        // interupt other code that interacts with the same structures.

        // So, we have to be careful not to move something out from under the other code.

        if (!(interruptCause.isTxTransferDone() || interruptCause.isRxTransferDone() || interruptCause.isNACK() ||
            interruptCause.isRxError() || interruptCause.isTxError())) {
            __asm__("BKPT");  // not actually done?
        }
        // Check that we're done with all transmission...
        // if (!hardware.doneReading()) {
        //     return;
        // }
        // if (!hardware.doneWriting()) {
        //     return;
        // }

        // _first_message is done sending, cache the pointer itself and manipulate it
        auto this_message = _first_message.load();
#ifdef IN_DEBUGGER
        if (nullptr == this_message) {
            __asm__("BKPT");  // no first message!?
            return;
        }
#endif
        // Update the state
        this_message->state.store(TWIMessage::State::kDone);

        // IMPORTANT NOTE: the callback may call sendNextMessage(), so we
        //   keep sending at true to prevent issues.

        // Pop _first_message off

        // // Atomically set the new _first_message to the next message (may be nullptr)
        // _first_message.store(this_message->next_message.load());

        // Use Compare-And-Store (CAS) to atomically change _first_message safely
        while(!_first_message.compare_exchange_weak(this_message, this_message->next_message)) {
            __asm__("BKPT");  // missed the set!
        }

        // Use Compare-And-Store (CAS) to atomically change _last_message safely
        // This will ONLY change _last_message IF _last_message==this_message.
        // If it fails, this_message will be set to the value found at _last_message,
        // so be careful to either not use this_message anymore or know it may have changed!
        // _last_message.compare_exchange_weak(this_message, this_message->next_message);

        this_message->next_message.store(nullptr);

        if (this_message->message_done_callback) {
            this_message->message_done_callback(
                !(interruptCause.isNACK() || interruptCause.isRxError() || interruptCause.isTxError()));
        } else {
            __asm__("BKPT");  // no callback!?
        }

        sending.store(false);  // we can now allow more sending

        sendNextMessage();
    };

    void queueAndSendMessage(TWIMessage* new_message) {
        // TWIMessage* old_last_message = _last_message.load();

        // // the result of this next line is that:
        // // 1- _last_message is set to new_message
        // // 2- old_last_message is set to the previous value of _last_message
        // while(!_last_message.compare_exchange_weak(old_last_message, new_message)) { ; }

        // // now we can safely set this:
        // old_last_message->next_message = new_message;

        // // And to carefully set _first_message, we'll store a temporary nullptr
        // TWIMessage* null_temp = nullptr;
        // // Then, IF _first_message matches nullptr, it'll set it to the new_message
        // _first_message.compare_exchange_weak(null_temp, new_message);

        TWIMessage* message_walker = _first_message.load();

        // If _first_message != nullptr or we can't set it to new_message...
        if (message_walker != nullptr ||
            !_first_message.compare_exchange_weak(message_walker, new_message)) {
            TWIMessage* next_message = message_walker;
            while (next_message) {
                message_walker = next_message;
                next_message   = message_walker->next_message.load();
            }
            message_walker->next_message.store(new_message);
        }

        sendNextMessage();
    };



#pragma mark TWIBusDevice (inside TWIBus)
    /**************************************************
     *
     * TWI Bus Device, a member of an TWI Bus.
     *
     **************************************************/

    struct TWIBusDevice : TWIBusDeviceBase {
        // Since we are defining this INSIDE the TWIBus struct, we'll use TWIBus internals liberally
        bus_type* const _twi_bus;

        using parent_type = bus_type;

        TWIAddress _twi_address;  // the address of this device

        constexpr TWIBusDevice(bus_type* const parent_bus, const TWIAddress&& address)
            : _twi_bus{parent_bus}, _twi_address{std::move(address)} {
            _twi_bus->addDevice(this);
        };

        // prevent copying or deleting
        TWIBusDevice(const TWIBusDevice&) = delete;

        // update the bus upon deletion
        ~TWIBusDevice() { _twi_bus->removeDevice(this); };

        TWIBusDevice(TWIBusDevice&& other) = delete;

        // queue message
        void queueMessage(TWIMessage* msg) override {
            msg->device = this;
            _twi_bus->queueAndSendMessage(msg);
        };

        const TWIAddress& getAddress() const override { return _twi_address; };

        bus_type* const getBus() const { return _twi_bus; };
    };

    using Device_t = TWIBusDevice;

    // TWIBusDevice factory on TWIBus
    constexpr TWIBusDevice getDevice(const TWIAddress&& address) { return {this, std::move(address)}; }
};  // TWIBus

}  // namespace Motate
#endif /* end of include guard: MOTATETWI_H_ONCE */
