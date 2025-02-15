# HERMES Examples for TPX3 Data Acquisition

This repository contains examples demonstrating the use of HERMES for setting up and acquiring data using TPX3Cam and SPIDR readout boards. The primary Python library for data acquisition with HERMES is `tpx3serval.py`.

## Getting Started

To use these examples, first ensure you have the HERMES system and the necessary hardware components set up. You will need TPX3Cam and SPIDR readout boards configured and connected to your system. Please see the ASI TPX3Cam manual for hardware setup and connections. 

Additionaly we suggest using a directory structure adopted from EMPIR. There are built-in functions in tpx3serval that will create the `[run_N]` directory and its sub-directories. Please make sure that the python scripts used to aquire data are located in the scripts folder of your working directory. 

```
Working Directory
├── README.md
├── scripts
    └── aquire_python_script.py
    └── aquire_config.ini
├── initFiles
├── [run_1]
    ├── imageFiles
    ├── previewFiles
    ├── statusFiles
    ├── tpx3Files
    ├── tpx3Logs
├── [run_2]
    ├── imageFiles
    ├── previewFiles
    ├── statusFiles
    ├── tpx3Files
    ├── tpx3Logs
├── [run_3]
├── [run_N]
```

For all of the data aquisition examples, the main python library used within hermes is `tpx3serval.py`. It can be loaded using the following:

```python
from pyhermes import tpx3serval
```

### Prerequisites

List any prerequisites or dependencies required for running the examples, such as Python versions, libraries, or other software.

### Installation

# 1. daq_simple/ #
This example illustrated a simple python script, aquireTpx.py, to first initiate and configure a TPX3Cam setup, and then take "n" exposures using functions from the tpx3serval library (located in pyhermes). Initial camera and needed DAQ configurations are stored in the config file: "aquireTpx3.ini".  

# 2. daq_ctscan/ #