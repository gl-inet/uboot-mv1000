/*******************************************************************************
Copyright (C) 2017 Marvell International Ltd.

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the three
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

This program is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation, either version 2 of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************************
Marvell GNU General Public License FreeRTOS Exception

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the Lesser
General Public License Version 2.1 plus the following FreeRTOS exception.
An independent module is a module which is not derived from or based on
FreeRTOS.
Clause 1:
Linking FreeRTOS statically or dynamically with other modules is making a
combined work based on FreeRTOS. Thus, the terms and conditions of the GNU
General Public License cover the whole combination.
As a special exception, the copyright holder of FreeRTOS gives you permission
to link FreeRTOS with independent modules that communicate with FreeRTOS solely
through the FreeRTOS API interface, regardless of the license terms of these
independent modules, and to copy and distribute the resulting combined work
under terms of your choice, provided that:
1. Every copy of the combined work is accompanied by a written statement that
details to the recipient the version of FreeRTOS used and an offer by yourself
to provide the FreeRTOS source code (including any modifications you may have
made) should the recipient request it.
2. The combined work is not itself an RTOS, scheduler, kernel or related
product.
3. The independent modules add significant and primary functionality to
FreeRTOS and do not merely extend the existing functionality already present in
FreeRTOS.
Clause 2:
FreeRTOS may not be used for any competitive or comparative purpose, including
the publication of any form of run time or compile time metric, without the
express permission of Real Time Engineers Ltd. (this is the norm within the
industry and is intended to ensure information accuracy).

********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.

	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.

	* Neither the name of Marvell nor the names of its contributors may be
	  used to endorse or promote products derived from this software without
	  specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

HOW-TO Build
============
o Requirements for building WTP download tool for Linux:
1. The code has been built and tested on x86_64 architecture machine running Ubuntu 14.04.
2. The compiler used for biulding this code was g++ 4.8.2.

o Building instructions:
The tool was built using the command:
   make -f makefile.mk


HOW-TO Use
============
o UART downloading instructions:
The UART downloading procedure requires BootROM enters into UART R/W mode.
(See section - 'Escape sequence instructions' for details.)

1. UART downloading for non-secure boot
   UART image components:
	* TIM_ATF.bin
	* boot-image_h.bin
	* wtmi_h.bin
   UART download command:
	sudo stty -F /dev/ttyUSB<port #> clocal
	./WtpDownload_linux -P UART -C <port #> -R 115200 -B TIM_ATF.bin -I wtmi_h.bin \
		-I boot-image_h.bin -E

2. Trusted Images
   UART image components:
	* TIM_ATF_TRUSTED.bin
	* TIMN_ATF_TRUSTED.bin
	* boot-image_h.bin
	* wtmi_h.bin
   UART download command:
	sudo stty -F /dev/ttyUSB<port #> clocal
	./WtpDownload_linux -P UART -C <port #> -R 115200 -B TIM_ATF_TRUSTED.bin \
		-B TIMN_ATF_TRUSTED.bin -I wtmi_h.bin -I boot-image_h.bin -E

Notes:
1. WtpDownload application doesn't require 'sudoer' privilege over USB Serial port.
2. WtpDownload application requires to assign the userid to 'dailout' group access.
   (need re-login to take effect)
   sudo usermod -a -G dialout <username>
3. Please refer to ATF - doc/mvebu/build.txt for the build command of UART images.
4. The UART image components are archieved in ATF - build/a3700/[release|debug]/uart-images.tgz.

o Escape sequence instrustions:
When working with a mal-functional bootloader in flash, WTP download tool can be used to
trap BootROM into UART R/W mode via UART port.

1. Press the 'RESET' button on your system and hold on.
2. Run the following command on your host machine and releaese the 'RESET' button in the
   meanwhile.
   sudo stty -F /dev/ttyUSB<port #> clocal
   ./WtpDownload_linux -P UART -C <port #> -R 115200 -Y

o Combined instructions for UART downloading and escape sequence:
The WtpDownload application provides an option to execute escape sequence and UART downloading
in a single command.
	sudo stty -F /dev/ttyUSB<port #> clocal
	./WtpDownload_linux -P UART -C <port #> -R 115200 -B TIM_ATF.bin -I wtmi_h.bin \
		-I boot-image_h.bin -E -O

