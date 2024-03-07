![HERMES](images/HERMES.jpg)
# HERMES: High-speed Event Retrieval and Management for Enhanced Spectral neutron imaging with TPX3Cams #

## What is HERMES? ##
HERMES is a repository of python and C/C++ libraries that are meant to acquire, process, and analyze, data from the TPX3Cam by [Amsterdam Scientific Instruments](https://www.amscins.com/buy-here/tpx3cam/ "ASI TPX3Cam"). It is capable of both aqcuiring and processing raw .tpx3 files.  

## Getting Started ## 
Before using HERMES, ensure your system meets the necessary requirements and set up the environment variable for a seamless experience.

### Prerequisites ###

- [pyHERMES] Python 3.8 or later
- [cHERMES] C/C++ Compiler (GCC for Linux/MacOS, MSVC for Windows)
- [cHERMES] CMake 3.15 or higher

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

## License ## 
HERMES is distributed as open-source software under an MIT License, with LANL open source approval (reference O4660). Please see LICENSE for more details. 