.. _kbchost:

Keyboard System Controller Host
###############################

Initially, the EC was conceived as a Keyboard System Controller (KSC), and its
purpose was to realize the 8042 spec from IBM `8042 IBM design guide`_. The
core function of keyboard controllers was to inform the CPU when a key was
pressed or released. They also managed auxiliary devices such as a mouse.

However, the KSC kept evolving to the point where it handles more
responsibilities than just PS/2 devices.  Therefore, today every 8042
command/behavior became a subset of the Embedded Controller chip.
In modern PC systems, the EC can receive 8042 commands from either BIOS
or the PS/2 windows driver. The picture below illustrates 8042 commands
triggered by a Windows driver.

  .. image:: kbchost_sequence_ps2.png
     :align: center

Here we can see that final software entity receiving the commands is the
Zephyr PS/2 driver which performs PS/2 communication with a mouse and/or
keyboard. The alternate flow shows how EC can also receive and process
8042 commands.

Theory of operation
*******************
This implementation of the 8042 spec is mainly performed by a software FSM
to handle host requests. These commands can be directed to 3 different
entities which can be the EC (8042 module) , PS/2 Keyboard or P/2 mouse.
The FSM is triggered by an ISR and at that precise moment port 0x60 is
read. This is where the FSM determines if it can process the command in the
DEFAULT_STATE or it requires to transition to a new state in order to
continue processing a given command.

Kbchost uses two threads to process input and return output values from
PS/2 devices and Scan matrix keyboard. One of them is exclusively used for
feeding input arguments to the FSM and returning dummy keyboard
configuration results. The dummy results are required in order to exercise
initialization commands for the scan matrix keyboard because
this device is just a grid of input/output wires. Therefore,
Kbchost code generates artificial host replies in order to maintain
an initialization sequence going.  The remaining thread is
used for handling data generated when a user interacts with
PS/2 keyboard/mouse and scan matrix keyboard.

Generally speaking, Kbchost gets the commands through port 0x60/0x64.
There are some variations in terms of how the commands and data arrive.
For example, in order to address the mouse, the host has to send a request
through port 0x64 to indicate device destination, then it sends a command
through port 0x60 and payload for the command, only if required, using port
0x60. On the other hand, when the host wishes to address the keyboard, then
both command and data arrive through port 0x60. The image below describes
the FSM implementation:

  .. image:: kbchost_transitions.png
     :align: center

Once a given transaction has been completely processed, then the FSM goes
back to DEFAULT_STATE and waits there for future requests.

PS/2 keyboard initialization sequence executed in BIOS
******************************************************

Below is an example of a 8042 command sequence needed to initialize
the keyboard in BIOS context.

+-------+---------+-------------------------------------+
| Port  | Payload | Action                              |
+=======+=========+=====================================+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF4   | Enable                              |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Mouse reply                         |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0xAA   | Reset self-test                     |
+-------+---------+-------------------------------------+
| 0x60  |  0x55   | Self-test response                  |
+-------+---------+-------------------------------------+
| 0x64  |  0x60   | Write to command byte               |
+-------+---------+-------------------------------------+
| 0x60  |  0x67   | Command byte payload                |
+-------+---------+-------------------------------------+
| 0x60  |  0xED   | Set led                             |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Previous command reply              |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Led settings (turn off all 3 leds)  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Previous command reply              |
+-------+---------+-------------------------------------+

These are the commands expected to be triggered in order to
have a functional keyboard in EFI shell or BIOS menu.  The keyboard
is enabled while writing 0x67 to the command byte, whereas the mouse
remains disabled. Bear in mind these commands may vary in terms of
the BIOS driving 8042 commands.

PS/2 keyboard and mouse initialization sequence executed in OS
**************************************************************
OS initialization for PS/2 devices is a little more complex since both
keyboard and mouse require a finer calibration dictated by PS/2 driver
running in OS context.

+-------+---------+-------------------------------------+
| Port  | Payload | Action                              |
+=======+=========+=====================================+
| 0x60  |  0xED   | Set leds                            |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Command reply                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x02   | Turn led corresponding to bit 1     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Command reply                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xED   | Set leds                            |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Command reply                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x02   | Turn one led on                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Command reply                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x67   | Command byte from EC                |
+-------+---------+-------------------------------------+
| 0x64  |  0x60   | Write command byte to EC            |
+-------+---------+-------------------------------------+
| 0x60  |  0x67   | Command byte from host              |
+-------+---------+-------------------------------------+
| 0x60  |  0xED   | Set leds                            |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Command reply                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Turn off all the leds               |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Command reply                       |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x77   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0x60   | Write command byte                  |
+-------+---------+-------------------------------------+
| 0x60  |  0x44   | Command byte from host              |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x74   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x60  |  0xFF   | Reset(in this case keyboard)        |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | Reply from keyboard                 |
+-------+---------+-------------------------------------+
| 0x60  |  0xAA   | BAT from keyboard                   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x74   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0x60   | Write command byte                  |
+-------+---------+-------------------------------------+
| 0x60  |  0x04   | Command byte from host              |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x34   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set typematic rate and delay        |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Typematic settings                  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0xED   | Set leds                            |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Turn off all the leds               |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x34   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0x60   | Write command byte                  |
+-------+---------+-------------------------------------+
| 0x60  |  0x44   | Command byte from host              |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x74   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send command to mouse               |
+-------+---------+-------------------------------------+
| 0x60  |  0xFF   | Reset command                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0xAA   | BAT                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Mouse ID                            |
+-------+---------+-------------------------------------+
| 0x60  |  0xED   | Set leds                            |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Turn off all the leds               |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x74   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0x60   | Write command byte                  |
+-------+---------+-------------------------------------+
| 0x60  |  0x47   | Command byte payload                |
+-------+---------+-------------------------------------+
| 0x64  |  0xAD   | Disable keyboard                    |
+-------+---------+-------------------------------------+
| 0x64  |  0xA7   | Disable mouse                       |
+-------+---------+-------------------------------------+
| 0x64  |  0x20   | Read command byte                   |
+-------+---------+-------------------------------------+
| 0x60  |  0x77   | Command byte returned to the host   |
+-------+---------+-------------------------------------+
| 0x64  |  0xAE   | Enable keyboard                     |
+-------+---------+-------------------------------------+
| 0x64  |  0xA8   | Enable mouse                        |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xFF   | Reset                               |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0xAA   | BAT                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Mouse ID                            |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF2   | Read mouse ID                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Mouse ID                            |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE8   | Set resolution                      |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Resolution payload                  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE6   | Set scaling 1:1                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE6   | Set scaling 1:1                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE6   | Set scaling 1:1                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE9   | Status request                      |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Status reply                        |
+-------+---------+-------------------------------------+
| 0x60  |  0x00   | Status reply                        |
+-------+---------+-------------------------------------+
| 0x60  |  0x64   | Status reply                        |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE8   | Set resolution                      |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x03   | Resolution payload                  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xC8   | Sample rate                         |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x64   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x50   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF2   | Read device type                    |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x03   | Device type                         |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xC8   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xC8   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x50   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF2   | Read device type                    |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x04   | Device type                         |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x64   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xE8   | Set resolution                      |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0x03   | Resolution payload                  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x64  |  0xD4   | Send to mouse                       |
+-------+---------+-------------------------------------+
| 0x60  |  0xF4   | Enable                              |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x08   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x03   | Resolution payload                  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x20   | Resolution payload                  |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0xF3   | Set sample rate                     |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+
| 0x60  |  0x20   | Sample rate value                   |
+-------+---------+-------------------------------------+
| 0x60  |  0xFA   | ACK                                 |
+-------+---------+-------------------------------------+

EC hotkeys
##########

EC early key sequence detection allows to reuse Keyboard matrix driver logic
to intercept key sequence combination like OS hotkeys but for EC internal
usage while leaving the any other hotkey communication to OS undisrupted.

They are intended to be used by other EC modules features where runtime behavior
is desired change when EC HW strap is not available in the design or not
desirable.

The pre-defined key sequences are configured via KConfig and there is
default to disable EC timeout mechanism for power sequencing.
They also have a reduce set of modifiers to ease distinction from OS.

.. note:: If keys are pressed prior to power on the system the notification
   comes ~50ms after keyboard configuration hence compilation time definitions
   guarantees that there is no race condition within the EC modules.

  .. image:: ec_hotkeys.png
     :align: center

References
**********
.. target-notes::

.. _8042 IBM design guide:
    http://64bitos.tistory.com/attachment/cfile29.uf@02784B4D50F966F12C3160.pdf
