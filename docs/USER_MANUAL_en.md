# EOL CAN Tool User Manual

## 1. Overview

EOL_CAN_Tool is a Qt-based CAN-bus EOL (End-Of-Line) test host application, used for production testing and debugging of products such as radar. It supports CAN devices from multiple vendors, message transmit/receive and parsing, data replay, EOL debugging and firmware upgrade.

Supported CAN device vendors:

- ZLG (full series)
- GCAN (CAN / CAN-FD)
- TOSUN
- Kvaser (full series)

## 2. Installation and Startup

### 2.1 Installer

Run `EOL_CAN_Tool_Setup_vX.Y.Z.exe` and follow the wizard. The default install directory is `%LOCALAPPDATA%\EOL_CAN_Tool`; Start Menu and desktop shortcuts are created. No administrator privileges are required.

### 2.2 Upgrading

The application checks for updates automatically on startup (see section 10). You can also simply run the new installer to upgrade in place; existing configuration files are preserved.

## 3. CAN Device Connection

### 3.1 Select a Device

After connecting the CAN device, select in order: **Vendor** → **Device Model** → **Channel** (channel numbers start from 0; you can open a single channel or all channels).

- **Device Index**: distinguishes multiple CAN devices of the same model.
- **Device Info**: reads the device SN, channel count, etc.; click to show a popup when readable.

### 3.2 Communication Parameters

Set the baud rate and other communication parameters as required by the device under test.

### 3.3 Start Connection

Click in order: **Open** → **Init** → **Start**. Click **More** to enter the main CAN communication test page; the top window shows whether the device opened/started successfully.

## 4. CAN Communication Test

This page supports:

- Transmit/receive on two or more channels
- Mask configuration
- Converting received data to string output (selectable CAN channel and CAN ID)
- Manual message sending
- Dual-color display of sent/received messages with timestamps
- Buttons to enter other menus
- Auto-appending a common CRC value to the end of the data in the manual-send window (click the corresponding CRC button to compute and append)
- Timed / counted sending (leave the frame count empty for unlimited timed sending; minimum period is 1 ms)
- Plot/chart forwarding (check the box to forward data to the chart display)

### 4.1 Data Replay

Data replay requires a replay data file, usually in txt or csv format. You can configure:

- The CAN ID field index
- The data start index
- Inter-frame interval (wait time after each frame, in ms)
- Special-wait CAN ID (wait time after this CAN ID is sent; optional)
- Special-wait data (wait a specific time when this data is encountered; optional)
- Special-wait data index (default 0; optional)

## 5. EOL Debugging

The EOL debugging page provides:

- Entering / exiting EOL mode and restarting the device
- Writing and reading various tables, with progress display
- Message transmission return value display
- Buttons to sub-pages (info read/write, antenna calibration, RCS calibration)

### 5.1 Info Read/Write

- Version, SN and mounID read/write tests
- DTC detection
- VCAN test

### 5.2 Antenna Calibration (2D Data)

- Set the target simulator (RTS) parameters
- Set turntable horizontal-rotation parameters
- Set turntable vertical-tilt parameters
- Simulate sending turntable info to the radar to request 2DFFT data
- Generate 2DFFT data CSV

> "Add Config" is incremental; to reconfigure, clear the existing configuration first. To collect 2D data for only one direction, leave the other direction's conditions empty.

### 5.3 RCS Calibration / Target View

- Set thresholds
- Continuously acquire target data
- Filter targets by the specified conditions via "Refresh Display Filter"
- "Target Statistics" further filters targets into the statistics list

> Prerequisite: add at least one condition to the threshold list before starting the target view.

## 6. SHELL Debug

- Console font color switching
- Copy/paste (after copying, right-click to paste)
- `TAB` key command completion
- Up/Down keys to browse command history
- Enter key to execute a command

## 7. Network Debug

Two network devices are provided: RTS and PLC.

### 7.1 RTS Network

RTS uses UDP communication with separate transmit and receive ports, so the PC side needs two ports (one listening for receive, one for sending control).

- Client port (send port) and server port (receive port)
- Set the communication IP to the local LAN IP (same subnet as the RTS)
- Set the work mode to "Client"
- Click "Start"; once started, RTS control buttons appear under "More → EOL Debug Page"

### 7.2 PLC Network

PLC uses Modbus TCP (coming soon).

## 8. Firmware Upgrade

1. Click "Firmware Select" and choose a bin file
2. Click "Start Upgrade" and reset the radar (you can enter EOL mode and click "Restart Device" for a soft reset)
3. A prompt appears when the upgrade succeeds

> Closing the page during an upgrade aborts the upgrade process.

## 9. Plot / Chart

The plot/chart input format is: `$1 2 3;`

- Starts with `$`
- Ends with `;`
- Numbers in the middle separated by spaces
- Each number is the value of the corresponding channel at the current time (1 = channel 0, 2 = channel 1, 3 = channel 2)

Data channel selection:

- **vcom**: virtual channel; data comes from a non-serial device (check the chart box to forward the data stream)
- **non-vcom**: data uses the serial port

## 10. Software Update

When a new version is released, the application prompts for an update on every startup. The update downloads the new installer and prompts to install it.

## 11. Troubleshooting

- **Device cannot be opened**: check that the device driver is installed and that the device is not occupied by another program.
- **No messages received**: check that the communication parameters and baud rate are correct, and that "Open → Init → Start" has been performed in order.
- **Upgrade failed**: check the network and installer integrity, or install manually with the new installer.
