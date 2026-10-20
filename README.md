# KernelLab — Setup and Run Guide

KernelLab targets the **ESP32-C3 (RISC-V)**. You can run its firmware on a
physical board or emulate the chip locally with QEMU

## Tools

| Tool | Purpose |
| --- | --- |
| **ESP-IDF** | Espressif's SDK, including libraries and tools for building firmware. |
| **`idf.py`** | Project command-line interface: configures, builds, flashes and debugs firmware using CMake, Ninja and other tools. |
| **`idf_tools.py`** | Installs and checks the SDK's compilers and supporting tools. |
| **`export.sh`** | Activates the ESP-IDF environment in the current terminal. |
| **Espressif QEMU** | Emulates the ESP32-C3 so the firmware can run without a physical board. |

## 1. Install prerequisites

Use a supported Python version, such as **Python 3.13**. Check yours with
`python3 --version`.

### macOS — Homebrew

Install Apple's Command Line Tools if needed:

```bash
xcode-select --install
```

Then install dependencies:

```bash
brew install git cmake ninja python@3.13 dfu-util libgcrypt glib pixman sdl2 libslirp
export PATH="$(brew --prefix python@3.13)/libexec/bin:$PATH"
```

### Linux — Ubuntu / Debian

```bash
sudo apt update
sudo apt install git wget flex bison gperf python3 python3-pip python3-venv \
    cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0 \
    libgcrypt20 libglib2.0-dev libpixman-1-0 libsdl2-2.0-0 libslirp0
```

On other distributions, install equivalent packages with your package manager.

## 2. Install ESP-IDF — once

This guide uses the SDK's installation scripts and ESP-IDF **v6.1**. If you
already have that version configured, skip to step 3 and use its `export.sh`.
For initial installation, use a new terminal without another ESP-IDF environment
already activated.

```bash
mkdir -p "$HOME/esp"
git clone --branch v6.1 --recursive https://github.com/espressif/esp-idf.git "$HOME/esp/esp-idf"
cd "$HOME/esp/esp-idf"
./install.sh esp32c3
```

Wait for installation to finish successfully. The SDK goes in `~/esp/esp-idf`;
its tools and Python environment are installed under `~/.espressif` by default.

Reference: [ESP-IDF setup scripts for Linux and macOS](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/get-started/linux-macos-setup-legacy.html).

## 3. Activate the environment

Run this **in every new terminal** before working on KernelLab:

```bash
source "$HOME/esp/esp-idf/export.sh"
idf.py --version
```

If you installed the SDK elsewhere, use your installation's `export.sh`.
Continue only when activation finishes successfully.

## 4. Configure and build the project

From the KernelLab repository root, where `CMakeLists.txt` is located:

```bash
idf.py set-target esp32c3
idf.py build
```

Run `set-target` once during initial project setup. It regenerates `sdkconfig`
and clears previous build configuration. After editing code, just run
`idf.py build`. Generated files go into `build/`.

## 5. Run locally with QEMU — no board required

Install Espressif's RISC-V emulator once, from an activated terminal:

```bash
python "$IDF_PATH/tools/idf_tools.py" install qemu-riscv32
source "$IDF_PATH/export.sh"
```

Then, from the KernelLab repository root:

```bash
idf.py qemu monitor
```

This builds the firmware, starts the emulated ESP32-C3 and displays the console
output. It runs the existing C and RISC-V Assembly code without adding source
files. QEMU emulates only supported peripherals; hardware timing may differ.

Press **Ctrl + ]** to exit the monitor.

Reference: [ESP32-C3 QEMU documentation](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-guides/tools/qemu.html).

## 6. Run on a physical ESP32-C3

Connect an **ESP32-C3 board** using a USB data cable. Find its serial port:

```bash
# macOS
ls /dev/cu.*

# Linux
ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null
```

Compare the list before and after connecting the board. Typical ports are
`/dev/cu.usbmodem1101` on macOS and `/dev/ttyACM0` or `/dev/ttyUSB0` on Linux.

From the repository root, replace `PORT` with the board's actual port:

```bash
idf.py -p PORT flash monitor
```

This builds if needed, writes the firmware to the board and opens its serial
console. Select the USB board's port rather than a Bluetooth device's port.
Press **Ctrl + ]** to exit the monitor.

On Linux, if accessing the port gives `Permission denied`, add your user to the
serial-access group required by your distribution (usually `dialout` on
Ubuntu/Debian), then log out and back in.

## Everyday commands

After activating ESP-IDF and opening the repository root:

| Goal | Command |
| --- | --- |
| Build only | `idf.py build` |
| Run locally in the emulator | `idf.py qemu monitor` |
| Flash and monitor a physical board | `idf.py -p PORT flash monitor` |
| Monitor a board already running firmware | `idf.py -p PORT monitor` |
| Configure project options | `idf.py menuconfig` |

**`qemu` runs the emulator. `flash` requires a connected microcontroller.**

Reference: [idf.py command reference](https://docs.espressif.com/projects/esp-idf/en/v6.1/esp32c3/api-guides/tools/idf-py.html).
