![HERMES](images/HERMES.jpg)
# HERMES: High-speed Event Retrieval and Management for Enhanced Spectral neutron imaging with TPX3Cams #

## What is HERMES? ##
HERMES is a repository of python and C/C++ libraries that are meant to acquire, process, and analyze, data from the TPX3Cam by [Amsterdam Scientific Instruments](https://www.amscins.com/buy-here/tpx3cam/ "ASI TPX3Cam"). It is capable of both aqcuiring and processing raw .tpx3 files.  

## Getting Started ## 
To use HERMES, you must first set an environment variable specific to your operating system.

### Setting the Environment Variable ###

**For Windows Users:**

1. Open the Start Search, type in "env", and choose "Edit the system environment variables"
2. In the System Properties window, click on the "Environment Variables..." button
3. In the Environment Variables window, click on "New..." under the "User variables" section
4. For the Variable name, enter `HERMES_HOME`
5. For the Variable value, enter the path to your HERMES installation directory
6. Click OK and Apply the changes

**For macOS/Linux Users:**

Open a terminal and add the following line to your `.bashrc`, `.zshrc`, or equivalent shell configuration file:

```sh
export HERMES_HOME=/path/to/your/hermes/installation
```

## License ## 
HERMES is distributed as open-source software under an MIT License, with LANL open source approval (reference O4660). Please see LICENSE for more details. 