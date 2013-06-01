/*
  utility/AvrUSB.h - Library for the Motate system
  http://tinkerin.gs/

  Copyright (c) 2013 Robert Giseburt

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

#ifndef AVRUSB_ONCE
#define AVRUSB_ONCE


namespace Motate {

	static const uint16_t kUSBControlEnpointSize = 0x10;
	static const uint16_t kUSBNormalEnpointSize = 0x200;

	// USBDeviceHardware actually talks to the hardware, and marshalls data to/from the interfaces.
	class USBDeviceHardware
	{
		uint32_t _inited;
		uint32_t _configuration;

	public:
		// Init
		USBDeviceHardware()
		{
			// if (UDD_Init() == 0UL)
			// {
			// 	_inited = 1UL;
			// }
			// _configuration = 0UL;
		};

		bool attach() {
			// if (_inited) {
			// 	UDD_Attach();
			// 	_configuration = 0;
			// 	return true;
			// }
			return false;
		};

		bool detach() {
			// if (_inited) {
			// 	UDD_Detach();
			// 	return true;
			// }
			return false;
		};

		int32_t available(const uint8_t ep) {
			// return UDD_FifoByteCount(ep & 0xF);
			return 0;
		}

		int32_t readByte(const uint8_t ep) {
			// uint8_t c;
			// if (USBD_Recv(ep & 0xF, &c, 1) == 1)
			// 	return c;
			return -1;
		};

		/* Data is const. The pointer to data is not. */
		int32_t read(const uint8_t ep, const uint8_t * buffer, uint16_t length) {
//			if (!_usbConfiguration || len < 0)
//				return -1;
//
//			LockEP lock(ep);
			// uint32_t n = UDD_FifoByteCount(ep & 0xF);
			// length = min(n, length);
			// n = length;
			// uint8_t* dst = (uint8_t*)d;
			// while (n--)
			// 	*dst++ = UDD_Recv8(ep & 0xF);
			// if (length && !UDD_FifoByteCount(ep & 0xF)) // release empty buffer
			// 	UDD_ReleaseRX(ep & 0xF);
			// 
			// return length;
			return 0;
		};

		/* Data is const. The pointer to data is not. */
		int32_t write(const uint8_t ep, const uint8_t * data, uint16_t length) {
// 			uint32_t n;
// 			int r = length;
// 			const uint8_t* data = (const uint8_t*)d;
// 
// //			if (!_usbConfiguration)
// //			{
// //				TRACE_CORE(printf("pb conf\n\r");)
// //				return -1;
// //			}
// 
// 			while (length)
// 			{
// 				if(ep==0) n = EP0_SIZE;
// 				else n =  EPX_SIZE;
// 				if (n > length)
// 					n = length;
// 				length -= n;
// 
// 				UDD_Send(ep & 0xF, data, n);
// 				data += n;
// 			}
// 
// 			return r;
			return 0;
		};
	};
}

#endif
//AVRUSB_ONCE
