# 🚀 Rapid Texter

**Rapid Texter** is a lightweight, terminal-based typing speed test game built with C++. Test your typing skills in multiple languages and programming syntax directly from your console.

## ✨ Features

- 📝 **Multiple Modes**: English, Indonesian, and Programming Syntax.
- ⏱️ **Real-time Stats**: Tracks Words Per Minute (WPM) and Accuracy.
- 💻 **Cross-Platform**: Runs on Windows and Linux.
- 🪶 **Lightweight**: Zero dependencies, just pure C++.

---

## ⚙️ Prerequisites

You need a C++ compiler to build this project.

### 🪟 Windows (MinGW-w64)
We recommend using the **MinGW-w64** build by Brecht Sanders (as tested with version 15.2.0).

1.  **Download**: Visit [WinLibs.com](https://winlibs.com/) and download the latest **GCC/G++ (UCRT)** version.
2.  **Install**: Extract the downloaded zip file to a location (e.g., `C:\\MinGW`).
3.  **Path**: Add the `bin` folder (e.g., `C:\\MinGW\\bin`) to your Windows **Environment Variables (PATH)**.
4.  **Verify**: Open PowerShell/CMD and type:
    ```powershell
    g++ --version
    ```

### 🐧 Linux (Debian/Ubuntu)
You need to install the `build-essential` package which includes GCC/G++ and Make.

```bash
sudo apt update && sudo apt install build-essential
```

---

## 🛠️ Build & Compile

Clone the repository and navigate to the project folder.

### On Windows
We have provided batch scripts for easy management.

**1. Compile:**
Double-click `compile_windows.bat` or run in terminal:
```powershell
.\\compile_windows.bat
```

**2. Clean (Optional):**
To remove compiled object files and the executable:
```powershell
.\\clean_windows.bat
```

### On Linux
Use the included `Makefile` for automated building.

**1. Compile:**
```bash
make
```

**2. Clean:**
```bash
make clean
```

---

## 🎮 How to Run

After compiling, an executable file will be created in the main directory.

* **Windows**:
    ```powershell
    .\\rapid-texter.exe
    ```

* **Linux**:
    ```bash
    ./rapid-texter
    ```

---

## 📂 Project Structure

```
rapid-texter/
├── include/        # Header files (.h)
├── src/            # Source files (.cpp)
├── assets/         # Text files (en.txt, id.txt, prog.txt)
├── Makefile        # Linux build script
├── *.bat           # Windows build scripts
└── README.md       # Documentation
```

## 📜 License

This project is open source. Feel free to contribute!