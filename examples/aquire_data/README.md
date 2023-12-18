# HERMES Examples for tpx3 data aquisiton
Here are several examples of how HERMES can be used to setup and aquire data (or images) using the TPX3Cam and SPIDR readout boards. HERMES 

## 1. daq_simple ##
This example illustrated a simple python script, aquireTpx.py, to first initiate and configure a TPX3Cam setup, and then take "n" exposures using tpx3serval library (located in pyhermes). Initial camera and needed DAQ configurations are stored in the config file: "aquireTpx3.ini".  