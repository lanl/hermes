
import os, shutil, glob
import subprocess
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from multiprocessing import Pool, cpu_count
from functools import partial
import threading
from multiprocessing import Value, Lock
import zipfile


######################################################################################
# Class for configuring the processing of tpx3 files using EMPIR binaries
#-------------------------------------------------------------------------------------
class empirConfig:
    """ A configure class for the processing of tpx3 files using EMPIR binaries from 
        Adrian S. Losko at TUM. This analysis code has the following structures. 
        File structures: 
            {dest}/tpx3Files/   <- Where initial tpx3 files are saved
            {dest}/listFiles/   <- Photon "list" files are stored here
            {dest}/eventFiles/  <- Neutron "event" files are stored here
            {dest}/final/       <- Final tiff stack images are stored here.
            
        Additionally, there are empir_export binaries that can be used to extract pixel,
        photon, and event information from the tpx3 files. These can be stored in a
        separate export directory
            {dest}/export/      <- Exported pixel, photon, and event information is stored here.
    """
    def __init__(self, dest):
        # file structure
        self.destination_dir = f"{dest}"                # locating of run directory
        self.log_file_dir = f"{dest}/logFiles/"         # locating of logFiles
        self.tpx3_file_dir = f"{dest}/tpx3Files/"       # locating of tpx3Files
        self.list_file_dir = f"{dest}/listFiles/"       # locating of listFiles
        self.event_file_dir = f"{dest}/eventFiles/"     # locating of eventFiles
        self.final_file_dir = f"{dest}/final/"          # locating of final image files
        self.export_file_dir = f"{dest}/exportFiles/"   # locating of exported info files
        
        # --- pixel-to-photon parameters
        self.pixel_d_space = 10                     # Distance in space for pixel search [px] (default: 10)
        self.pixel_d_time = 1E-6                    # Distance in time for pixel search [s] (default: 1e-06)
        self.pixel_min_number = 2                   # Minimum number of pixels in a photon event (default: 2)
        self.use_tdc1 = False                       # Use TDC1 as trigger input (TDC1 is ignored by default)
        
        # --- photon-to-event parameters
        self.photon_d_space = None                  # Distance in space for photon search [px]
        self.photon_d_time = None                   # Distance in time for photon search [s]
        self.photon_max_duration = None             # Maximum duration to look for photons [s] (default: inf)
        self.photon_d_time_extF = None              # Extents duration by multiple of time difference to last photon (default: 0)
        
        # --- event-to-image parameters
        self.size_x = 512                           # Number of pixels in x direction for the final image (default: 512 px)
        self.size_y = 512                           # Number of pixels in y direction for the final image (defaults to match size in x dimension)                                                      
        self.nPhotons_min = 0                       # Only events with at least this number of photons will be used for the image (default: 0)
        self.nPhotons_max = 18446744073709551615    # Only events with at most this number of photons will be used for the image (default: inf)
        self.time_extTrigger = False                # Use the time relative to the external trigger for binning (default: false and use absolute time)
        self.time_res_s = None                      # Timing resolution in seconds for creating an 3D image sequence (default: 2D image is created, integrated in time)
        self.time_limit = None                      # Maximum of "time_res_s" bins for the 3D image sequence
        self.psd_min = 0                            # Minimum PSD value
        self.psd_max = 100                          # Maximum PSD value
        
    def set_pixel_to_photon_params(self, d_space, d_time, min_number, use_tdc1):
        """
        Set parameters related to pixel to photon processing.

        Args:
            d_space (int): Distance in space for pixel search [px].
            d_time (float): Distance in time for pixel search [s].
            min_number (int): Minimum number of pixels in a photon event.
            use_tdc1 (bool): Use TDC1 as trigger input.
        """
        self.pixel_d_space = d_space
        self.pixel_d_time = d_time
        self.pixel_min_number = min_number
        self.use_tdc1 = use_tdc1

    def set_photon_to_event_params(self, d_space, d_time, max_duration, d_time_extF=None):
        """
        Set parameters related to photon to event processing.

        Args:
            d_space (int): Distance in space for photon search [px].
            d_time (float): Distance in time for photon search [s].
            max_duration (float): Maximum duration to look for photons [s].
            d_time_extF (float, optional): Extents duration by multiple of time difference to last photon.
        """
        self.photon_d_space = d_space
        self.photon_d_time = d_time
        self.photon_max_duration = max_duration
        self.photon_d_time_extF = d_time_extF if d_time_extF is not None else self.photon_d_time_extF

    def set_event_to_image_params(self, size_x=512, size_y=512, nPhotons_min=0, nPhotons_max=None, time_extTrigger=False, time_res_s=None, time_limit=None, psd_min=0, psd_max=100):
        """
        Set parameters related to event to image processing.

        Args:
            size_x (int): Number of pixels in x direction for the final image.
            size_y (int): Number of pixels in y direction for the final image.
            nPhotons_min (int): Minimum number of photons for image processing.
            nPhotons_max (int): Maximum number of photons for image processing.
            time_extTrigger (float, optional): Use time relative to external trigger for binning.
            time_res_s (float, optional): Timing resolution in seconds for 3D image sequence.
            time_limit (int, optional): Maximum of time bins for the 3D image sequence.
            psd_min (float): Minimum PSD value.
            psd_max (float): Maximum PSD value.
        """
        self.size_x = size_x
        self.size_y = size_y
        self.nPhotons_min = nPhotons_min
        self.nPhotons_max = nPhotons_max
        self.time_extTrigger = time_extTrigger
        self.time_res_s = time_res_s
        self.time_limit = time_limit
        self.psd_min = psd_min
        self.psd_max = psd_max

    def check_or_create_sub_dirs(self,verbose_level=0):
        """
        Check if the subdirectories exist, and create them if they don't.
        """
        for dir_name in [self.log_file_dir, self.tpx3_file_dir, self.list_file_dir, self.event_file_dir, self.final_file_dir, self.export_file_dir]:
            if(verbose_level>=1):
                print("Checking directory: ", dir_name) 
            if not os.path.exists(dir_name):
                print(f"Could not find {dir_name}... now creating {dir_name}")
                os.makedirs(dir_name)
            else:
                if(verbose_level>=1):
                    print(f"Found {dir_name}")
######################################################################################





######################################################################################
# Functions for processing tpx3 files using EMPIR binaries
#-------------------------------------------------------------------------------------

#-------------------------------------------------------------------------------------
def zip_file(directory, filename):  
    """Zip a file in a specified directory.

    Args:
        directory (str): The directory containing the file to zip.
        filename (str): The name of the file to zip.
    """
    with zipfile.ZipFile(os.path.join(directory, filename + '.zip'), 'w', zipfile.ZIP_DEFLATED) as zipf:
        zipf.write(os.path.join(directory, filename), arcname=filename)


#-------------------------------------------------------------------------------------
def check_for_files(directory, extension,verbose_level=0):
    """Check if any file with a specific extension exists in a directory.

    Args:
        directory (str): The directory to check.
        extension (str): The file extension to look for.

    Returns:
        bool: True if any file with the specified extension exists in the directory, False otherwise.
    """
    check_for_files = any(glob.glob(os.path.join(directory, f'*{extension}')))
    if verbose_level == 1:
        print(f"Checking for files with extension {extension} in {directory}")
        if check_for_files == True:
            print(f"Found files with extension {extension} in {directory}")
        else:
            print(f"No files found with extension {extension} in {directory}")
    
    return check_for_files


#-------------------------------------------------------------------------------------
def process_pixels_to_photons(input_dir="", tpx3_file_name="", dspace=10, dtime=1e-06, nPxMin=2, use_tdc1=False, output_dir="./", log_dir="./"):
    """ Runs empir_pixel2photon_tpx3spidr with the user-defined parameters. Input and output files are specified by the user.
    
    Note: You need to have the EMPIR binaries installed and in your PATH to use this function.

    Args:
        input_dir (str): Directory of the input file.
        tpx3_zip_file_name (str): Name of the zipped .tpx3 input file.
        dspace, dtime, nPxMin, use_tdc1: Parameters for empir_pixel2photon_tpx3spidr.
        output_dir (str): Directory to move output files to.
        log_dir (str): Directory for log files.
    """
    input_file = os.path.join(input_dir, tpx3_file_name)
    log_file_name = tpx3_file_name.split(".")[0] + ".pixel2photon"
    log_file = os.path.join(log_dir, log_file_name)
    list_file_name = tpx3_file_name.replace('.tpx3', '.empirphot')
    output_file = os.path.join(output_dir, list_file_name)
    tdc_option = "-T" if use_tdc1 else ""

    # Prepare the subprocess command for running "empir_pixel2photon_tpx3spidr"
    pixels_to_photons_command = [
        "empir_pixel2photon_tpx3spidr",
        "-s", str(dspace),
        "-t", str(dtime),
        "-k", str(nPxMin),
        tdc_option,
        "-i", input_file,
        "-o", output_file
    ]
    pixel_to_photon_run_msg = f"Running command: {' '.join(pixels_to_photons_command)}"

    print(f"EMPIR: Processing pixels to photons for {tpx3_file_name}")

    with open(log_file, '+w') as log_output:
        log_output.write("<HERMES> " + pixel_to_photon_run_msg + "\n")
        subprocess.run(pixels_to_photons_command, stdout=log_output, stderr=subprocess.STDOUT)
    
    
#-------------------------------------------------------------------------------------
def process_photons_to_events(input_dir="", list_file_name="", output_dir="./", dspace=3, dtime=100e-6, durationMax=500e-6, log_dir="./"):
    """Runs empir_photon2event with the user-defined parameters. Input and output files are specified by the user.
    
    Note: You need to have the EMPIR binaries installed and in your PATH to use this function.
    
    Here are the options of empir_photon2event:
        -i, --inputFile     Path to the input file, must not contain commas
        -o. --outputFile    Path to the output file, must not contain commas.
        -s, --dspace        Distance in space for photon search [px]
        -t, --dtime         Distance in time for photon search [s]
        -D, --durationMax   Maximum duration to look for photons [s] (default: inf)

    Args:
        input_path (str): Path to the input file (.list).
        dest (str): Destination directory name.
        output_path (str): Path to the output file (.event).
        dspace (int): Distance in space for photon search [px].
        dtime (float): Distance in time for photon search [s].
        durationMax (float): Maximum duration to look for photons [s].
    """
    
    input_file = os.path.join(input_dir, list_file_name)                    # creating full path+name for input tpx3 file. 
    log_file_name = os.path.splitext(list_file_name)[0] + ".photon2event"   # creating name for corresponding log file  
    log_file = os.path.join(log_dir, log_file_name)                         # creating full path+name for log file.
    event_file_name = list_file_name.split(".")[0]+".event"                 # creating name for output event file
    output_file = os.path.join(output_dir, event_file_name)                 # creating full path+name for output event file.
    
    # Prepare the subprocess command
    photons_to_events_command = [
        "empir_photon2event",
        "-i", input_file,
        "-o", output_file,
        "-s", str(dspace),
        "-t", str(dtime),
        "-D", str(durationMax)
        ]
    
    photons_to_events_run_msg = f"Running command: {' '.join(photons_to_events_command)}"
    
    print("EMPIR: Processing photons to events for {}".format(os.path.splitext(list_file_name)[0]))
    
    with open(log_file, 'a') as log_output:
        log_output.write("<hermes> " + photons_to_events_run_msg + "\n")
        subprocess.run(photons_to_events_command, stdout=log_output, stderr=subprocess.STDOUT)

#-------------------------------------------------------------------------------------
def export_pixel_activations(input_dir="",input_file="",output_dir="",output_file="",log_dir="./"):
    """ This function exports pixel activations or hits from a tpx3 file using the binary empir_export_pixelActivations. 
    
    Note1: You need to have the EMPIR binaries installed and in your PATH to use this function.
    
    Note2: Requires a .tpx3 file as input. The output info of each event will be contained in 5 consecutive doubles: 
    - x coordinate in pixels on the imaging chip
    - y coordinate in pixels on the imaging chip
    - absolute time in seconds
    - time over threshold in arbitrary units
    - time relative to the last trigger (nan if the event occurred before the first trigger)

    Args:
        input_dir (str, optional): Input directory containing all tpx3 files. Defaults to "".
        input_file (str, optional): A specific tpx3 file. Defaults to "".
        output_dir (str, optional): Output directory for all exported files. Defaults to "".
        output_file (str, optional): Specific output file name. Defaults to "".
        log_dir (str, optional): Directory for all empir logs. Defaults to "./".
    """
    
    # Prepare the subprocess command for running "empir_export_pixelActivations"
    # Note there is no "-i" or "-o" flag for this command, so we need to pass the input and output file paths as arguments
    
    export_pixel_activations_command = [
        "empir_export_pixelActivations",
        os.path.join(input_dir, input_file),
        os.path.join(output_dir, output_file)
    ]
    
    log_file_name = input_file.split(".")[0] + ".export_pixel_activations"
    export_pixel_activations_run_msg = f"Running command: {' '.join(export_pixel_activations_command)}"
    #print(export_pixel_activations_run_msg)

    print(f"EMPIR: Exporting pixel activations for {input_file}")
    
    with open(os.path.join(log_dir, log_file_name), 'w') as log_output:
        log_output.write("<hermes> " + export_pixel_activations_run_msg + "\n")
        subprocess.run(export_pixel_activations_command, stdout=log_output, stderr=subprocess.STDOUT)


#-------------------------------------------------------------------------------------
def unpack_pixel_activations(config, process_pool, file_counter, lock):
    """This function takes the configuration object, process pool, file counter, and lock as arguments. It reads TPX3 files from the specified directory in the configuration object and exports the pixel activations to corresponding pixel activation files in the log file directory.

    Args:
        config (object): The configuration object containing TPX3 file directory and log file directory.
        process_pool (object): The process pool used for parallel processing.
        file_counter (object): The shared counter for tracking the number of processed files.
        lock (object): The lock used for synchronizing access to the file counter.
    """
    # check if empir_export_pixelActivations is in your path
    if shutil.which("empir_export_pixelActivations") is None:
        raise FileNotFoundError("empir_export_pixelActivations not found in path. Please check if EMPIR binaries are installed.")
    
    # Get a list of existing TPX3 files in the directory
    existing_tpx3_files = [f for f in os.listdir(config.tpx3_file_dir) if f.endswith(".tpx3")]
    
    # Process each TPX3 file in parallel
    for tpx3_file in existing_tpx3_files:
        # Apply the process_pixels_to_photons function asynchronously
        process_pool.apply_async(export_pixel_activations, args=(
            config.tpx3_file_dir, 
            tpx3_file, 
            config.export_file_dir, 
            tpx3_file.replace(".tpx3", ".pixelActivations"), 
            config.log_file_dir)
        )

        # Increment the file counter
        with lock:
            file_counter.value += 1
        
    process_pool.close()
    process_pool.join()
    

#-------------------------------------------------------------------------------------
def process_existing_tpx3_files(config, process_pool, file_counter, lock):
    """
    Process existing TPX3 files in the specified directory.

    Args:
        config (object): An object containing configuration parameters.
        process_pool (object): A process pool for parallel processing.
        file_counter (object): A shared counter for tracking the number of processed files.
        lock (object): A lock for synchronizing access to the file counter.
    """
    existing_tpx3_files = [f for f in os.listdir(config.tpx3_file_dir) if f.endswith(".tpx3")]
    total_files = len(existing_tpx3_files)
    print(f"Found {total_files} existing TPX3 files")

    async_results = []

    for tpx3_file in existing_tpx3_files:
        async_result = process_pool.apply_async(process_pixels_to_photons, args=(
            config.tpx3_file_dir, 
            tpx3_file, 
            config.pixel_d_space, 
            config.pixel_d_time, 
            config.pixel_min_number, 
            config.use_tdc1, 
            config.list_file_dir, 
            config.log_file_dir))
        async_results.append(async_result)

        with lock:
            file_counter.value += 1

    # Wait for all tasks to complete and handle exceptions
    for async_result in async_results:
        try:
            async_result.get()  # This will re-raise any exceptions encountered in the worker process
        except Exception as e:
            print(f"An error occurred: {e}")


#-------------------------------------------------------------------------------------
def process_existing_photon_files(config, process_pool, file_counter, lock):
    """
    Process existing TPX3 files in the specified directory.

    Args:
        config (object): An object containing configuration parameters.
        process_pool (object): A process pool for parallel processing.
        file_counter (object): A shared counter for tracking the number of processed files.
        lock (object): A lock for synchronizing access to the file counter.
    """
    existing_photon_files = [f for f in os.listdir(config.list_file_dir) if f.endswith(".empirphot")]
    total_files = len(existing_photon_files)
    print(f"Found {total_files} existing empirphot files")

    async_results = []

    for photon_file in existing_photon_files:
        async_result = process_pool.apply_async(process_photons_to_events, args=(
            config.list_file_dir, 
            photon_file, 
            config.event_file_dir, 
            config.photon_d_space, 
            config.photon_d_time, 
            config.photon_max_duration, 
            config.log_file_dir)
        async_results.append(async_result)
        
        with lock:
            file_counter.value += 1

    # Wait for all tasks to complete and handle exceptions
    for async_result in async_results:
        try:
            async_result.get()  # This will re-raise any exceptions encountered in the worker process
        except Exception as e:
            print(f"An error occurred: {e}")

######################################################################################




######################################################################################
# Classes for handling file system events
#-------------------------------------------------------------------------------------
class Tpx3FilesHandler(FileSystemEventHandler):
    def __init__(self, config, observer, file_counter, lock, stop_delay=30):
        self.file_counter = file_counter
        self.lock = lock
        self.d_space = config.pixel_d_space
        self.d_time = config.pixel_d_time
        self.nPxMin = config.pixel_min_number
        self.use_tdc1 = config.use_tdc1
        self.list_file_dir = config.list_file_dir
        self.log_file_dir = config.log_file_dir
        self.observer = observer
        self.stop_delay = stop_delay
        self.stop_timer = None

    def reset_stop_timer(self):
        if self.stop_timer:
            self.stop_timer.cancel()
        self.stop_timer = threading.Timer(self.stop_delay, self.observer.stop)
        self.stop_timer.start()

    def on_created(self, event):
        if event.is_directory:
            return
        elif event.src_path.endswith(".tpx3.zip"):
            print(f"New tpx3 file detected: {event.src_path}")
            tpx3_file = event.src_path.split("/")[-1]
            input_path = os.path.dirname(event.src_path)

            # Submit the processing function to the global pool
            global global_pool
            if global_pool is not None:
                global_pool.apply_async(process_pixels_to_photons, args=(
                    input_path, tpx3_file, self.d_space, self.d_time, self.nPxMin, 
                    self.use_tdc1, self.list_file_dir, self.log_file_dir))
            
            # Increment the file counter
            with self.lock:
                self.file_counter.value += 1
        
        # Reset the stop timer each time a file is processed
        self.reset_stop_timer()


#-------------------------------------------------------------------------------------
class ListFilesHandler(FileSystemEventHandler):
    def __init__(self, config, observer, file_counter, lock, stop_delay=30):
        self.config = config
        self.observer = observer
        self.file_counter = file_counter
        self.lock = lock
        self.stop_delay = stop_delay
        self.stop_timer = None

    def reset_stop_timer(self):
        if self.stop_timer:
            self.stop_timer.cancel()
        self.stop_timer = threading.Timer(self.stop_delay, self.observer.stop)
        self.stop_timer.start()

    def on_created(self, event):
        if event.is_directory:
            return
        elif event.src_path.endswith(".list"):
            print(f"New list file detected: {event.src_path}")
            list_file = event.src_path.split("/")[-1]
            input_path = os.path.dirname(event.src_path)

            global global_pool
            if global_pool is not None:
                global_pool.apply_async(process_photons_to_events, args=(
                    input_path, list_file, self.config.event_file_dir, 
                    self.config.photon_d_space, self.config.photon_d_time, 
                    self.config.photon_max_duration, self.config.log_file_dir))

            # Increment the file counter
            with self.lock:
                self.file_counter.value += 1

            self.reset_stop_timer()

#-------------------------------------------------------------------------------------
def monitor_tpx3_files(config, file_counter, lock):
    observer = Observer()
    tpx3_handler = Tpx3FilesHandler(config, observer, file_counter, lock)

    observer.schedule(tpx3_handler, path=config.tpx3_file_dir, recursive=False)
    observer.start()
    observer.join()

    # Return the final count of processed files
    with lock:
        return file_counter.value


#-------------------------------------------------------------------------------------
def monitor_list_files(config, file_counter, lock):
    observer = Observer()
    list_files_handler = ListFilesHandler(config, observer, file_counter, lock)
    print(config.list_file_dir)
    observer.schedule(list_files_handler, path=config.list_file_dir, recursive=False)
    observer.start()
    observer.join()

    # Return the final count of processed files
    with lock:
        return file_counter.value

######################################################################################