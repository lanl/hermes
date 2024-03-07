![HERMES](images/HERMES.jpg)
# HERMES: High-speed Event Retrieval and Management for Enhanced Spectral neutron imaging with TPX3Cams #

## What is HERMES? ##
HERMES is a repository of python and C/C++ libraries that are meant to acquire, process, and analyze, data from the TPX3Cam by [Amsterdam Scientific Instruments](https://www.amscins.com/buy-here/tpx3cam/ "ASI TPX3Cam"). It is capable of both aqcuiring and processing raw .tpx3 files. 

## Getting Started ## 
Before using HERMES, ensure your system meets the necessary requirements and set up the environment variable for a seamless experience.

### Prerequisites ###

HERMES requires Python 3.8 or later, a C/C++ Compiler, and CMake 3.15 or higher. Below are instructions for installing these prerequisites on Windows and Linux/macOS.

#### Windows ####

1. **Python 3.8 or later:**
   - Download and install Python from [https://www.python.org/downloads/](https://www.python.org/downloads/). Ensure you check the option to 'Add Python to PATH' during installation.

2. **Microsoft C++ Build Tools:**
   - Download and install the Build Tools for Visual Studio from [https://visualstudio.microsoft.com/visual-cpp-build-tools/](https://visualstudio.microsoft.com/visual-cpp-build-tools/). During installation, select the "C++ build tools" workload.

3. **CMake:**
   - Download and install CMake from [https://cmake.org/download/](https://cmake.org/download/). During installation, ensure you add CMake to the system PATH.

#### Linux/macOS ####

1. **Python 3.8 or later:**
   - Install Python using your distribution's package manager (e.g., `apt` on Ubuntu, `brew` on macOS).
     ```sh
     # Ubuntu/Debian
     sudo apt-get update
     sudo apt-get install python3.8
     ```
     ```sh
     # macOS
     brew install python@3.8
     ```

2. **GCC/G++ (Linux) or Xcode Command Line Tools (macOS):**
   - Linux:
     ```sh
     sudo apt-get install build-essential
     ```
   - macOS:
     - Install Xcode from the App Store, then open Terminal and run:
       ```sh
       xcode-select --install
       ```

3. **CMake:**
   - Install CMake using your package manager or from [https://cmake.org/download/](https://cmake.org/download/).
     ```sh
     # Ubuntu/Debian
     sudo apt-get install cmake
     ```
     ```sh
     # macOS
     brew install cmake
     ```

### Setting the Environment Variable ###

**Windows Users:**

1. Search for "Edit the system environment variables" and open it.
2. Click "Environment Variables."
3. Under "User variables," click "New."
4. Set the Variable name to `HERMES_HOME`.
5. Set the Variable value to the path of your HERMES installation.
6. Click OK to save.

**macOS/Linux Users:**

1. Open your shell profile file (`.bashrc`, `.zshrc`, etc.).
2. Add `export HERMES_HOME=/path/to/hermes`.
3. Replace `/path/to/hermes` with your installation path.
4. Save the file and apply changes with `source ~/.bashrc` or equivalent.

### Verifying Installation ###

To verify the environment variable:

**Windows:**

```cmd
echo %HERMES_HOME%
```

**macOS/Linux Users:**
```sh
echo $HERMES_HOME
```


## License ## 
HERMES is distributed as open-source software under an MIT License, with LANL open source approval (reference O4660). Please see LICENSE for more details. 