# HERMES: TPX3Cam Data Acquisition & Analysis

HERMES is a software framework designed for **facilitating the acquisition and analysis of TPX3Cam data**. It consists of **C++** modules for data unpacking and analysis, as well as **Python** scripts for data acquisition and interfacing with commercial tools.

## 📂 Project Structure

```
HERMES/
├── src/
│   ├── chermes/                 # C++ codebase for unpacking & analysis
│   │   ├── include/         # Header files (.h)
│   │   ├── unpackers/       # C++ TPX3Cam data unpackers
│   │   ├── analyzers/       # C++ data analyzers
│   │   ├── utils/           # Common C++ utilities
│   │   ├── main.cpp         # (Optional) Main entry for standalone execution
│   │   ├── CMakeLists.txt   # Build instructions (modern approach over Makefile)
│   ├── pyhermes/            # Python-based acquisition, analysis, and wrappers
│   │   ├── acquisition/     # Data acquisition scripts
│   │   ├── empir/           # Python interfaces EMPIR
│   │   |   ├── core.py      # core functions of EMPIR class
│   │   |   ├── export.py    # export functions builts around Wolfertz exporter. 
|   |   |   ├── models.py    # pydantic models for EMPIR class
│   │   ├── analysis/        # Python-based analysis scripts
│   │   ├── utils/           # Shared Python utilities (logging, config parsing, etc.)
|   |   |   ├── logger.py    # Logging functions for pyHERMES
│   │   ├── depreciated/     # Deprecated or unused scripts
│   │   ├── __init__.py      # Makes this a package
│   │   ├── main.py          # Main script for Python execution
│   ├── shared/              # Code and data shared between C++ and Python
│   │   ├── data/            # Shared data files
│   │   ├── config/          # Common configuration files (YAML, JSON, etc.)
│   │   ├── constants/       # Shared constants (error codes, settings)
│   │   ├── bindings/        # C++/Python bindings (ctypes, pybind11, etc.)
│   │   ├── utils/           # Cross-language utilities (e.g., C++ helper scripts, JSON parsers)
│
├── tests/                   # Unit and integration tests
│   ├── cpp/                 # C++ tests
│   ├── python/              # Python tests
│
├── examples/                # Example scripts for users
│   ├── example_acquire.py   # Example acquisition script
│   ├── example_unpack.cpp   # Example unpacking script
│
├── docs/                    # Documentation
│   ├── API.md               # API documentation
│   ├── Setup.md             # Installation & setup guide
│   ├── Usage.md             # How to use the software
│
├── scripts/                 # Build, deployment, and automation scripts
│   ├── build.sh             # Build automation script
│   ├── deploy.sh            # Deployment script
│
├── CMakeLists.txt           # Root CMake file for building C++ code
├── requirements.txt         # Python dependencies
├── setup.py                 # Python packaging setup (if applicable)
├── LICENSE
└── README.md
```