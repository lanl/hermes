# HERMES Examples for TPX3 Data Acquisition

This repository contains examples demonstrating the use of HERMES for setting up and acquiring data using TPX3Cam and SPIDR readout boards. The primary Python library for data acquisition with HERMES is `tpx3serval.py`.

## Getting Started

To use these examples, first ensure you have the HERMES system and the necessary hardware components set up. You will need TPX3Cam and SPIDR readout boards configured and connected to your system. Please see the ASI TPX3Cam manual for hardware setup and connections. 

Additionaly we suggest using a directory structure adopted from EMPIR. 

Working Directory
├── README.md
├── scripts
├── initFiles
├── [sub_run_1]
    ├── imageFiles
    ├── previewFiles
    ├── statusFiles
    ├── tpx3Files
    ├── tpx3Logs
├── [sub_run_2]
    ├── imageFiles
    ├── previewFiles
    ├── statusFiles
    ├── tpx3Files
    ├── tpx3Logs
├── [sub_run_3]
├── [sub_run_...]

### Prerequisites

List any prerequisites or dependencies required for running the examples, such as Python versions, libraries, or other software.

### Installation

Provide instructions on how to install any necessary libraries or dependencies. For example:

Here are several examples of how HERMES can be used to setup and aquire data (or images) using the TPX3Cam and SPIDR readout boards. 

For all of these examples the main python library for data aquisition with hermes is tpx3serval.py. It can be loaded using the following:

```python
from pyhermes import tpx3serval
```

# 1. daq_simple #
This example illustrated a simple python script, aquireTpx.py, to first initiate and configure a TPX3Cam setup, and then take "n" exposures using functions from the tpx3serval library (located in pyhermes). Initial camera and needed DAQ configurations are stored in the config file: "aquireTpx3.ini".  

# 2. daq_ctscan #