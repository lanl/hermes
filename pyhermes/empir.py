
import os, sys, shutil
import subprocess
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
from multiprocessing import Pool, cpu_count
from functools import partial
import threading
from multiprocessing import Value, Lock
import zipfile



#-------------------------------------------------------------------------------------
class empirConfig:
    """ A configure class for the processing of tpx3 files using EMPIR binaries from 
        Adrian S. Losko at TUM. This analysis code has the following structures. 
        File structures: 
            {dest}/tpx3Files/   <- Where initial tpx3 files are saved
            {dest}/listFiles/   <- Photon "list" files are stored here
            {dest}/eventFiles/  <- Neutron "event" files are stored here
            {dest}/final/       <- Final tiff stack images are stored here.
    """
    def __init__(self, dest):
        # file structure
        self.log_file_dir = f"../data/{dest}/logFiles/"         # locating of logFiles
        self.tpx3_file_dir = f"../data/{dest}/tpx3Files/"       # locating of tpx3Files
        self.list_file_dir = f"../data/{dest}/listFiles/"       # locating of listFiles
        self.event_file_dir = f"../data/{dest}/eventFiles/"     # locating of eventFiles
        self.final_file_dir = f"../data/{dest}/final/"          # locating of final image files
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



#-------------------------------------------------------------------------------------
def zip_file(directory, filename):  
    with zipfile.ZipFile(os.path.join(directory, filename + '.zip'), 'w', zipfile.ZIP_DEFLATED) as zipf:
        zipf.write(os.path.join(directory, filename), arcname=filename)

#-------------------------------------------------------------------------------------

def process_pixels_to_photons(input_dir="", tpx3_zip_file_name="", dspace=10, dtime=1e-06, nPxMin=2, use_tdc1=False, output_dir="./", log_dir="./"):
    """
    Unzips the .tpx3 file, processes it, and logs the output. Then removes the unzipped file.

    Args:
        input_dir (str): Directory of the input file.
        tpx3_zip_file_name (str): Name of the zipped .tpx3 input file.
        dspace, dtime, nPxMin, use_tdc1: Parameters for empir_pixel2photon_tpx3spidr.
        output_dir (str): Directory to move output files to.
        log_dir (str): Directory for log files.
    """
    zip_file_path = os.path.join(input_dir, tpx3_zip_file_name)
    tpx3_file_name = tpx3_zip_file_name.replace('.zip', '')
    input_file = os.path.join(input_dir, tpx3_file_name)
    log_file_name = tpx3_file_name.split(".")[0] + ".pixel2photon"
    log_file = os.path.join(log_dir, log_file_name)
    list_file_name = tpx3_file_name.replace('.tpx3', '.list')
    output_file = os.path.join(output_dir, list_file_name)
    tdc_option = "-T" if use_tdc1 else ""

    # Log the unzipping process
    try:
        with zipfile.ZipFile(zip_file_path, 'r') as zip_ref:
            zip_ref.extractall(input_dir)
        unzip_log_msg = f"Successfully unzipped: {zip_file_path}"
    except Exception as e:
        unzip_log_msg = f"Error unzipping {zip_file_path}: {e}"

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

    with open(log_file, 'a') as log_output:
        log_output.write("<hermes> " + unzip_log_msg + "\n")
        log_output.write("<hermes> " + pixel_to_photon_run_msg + "\n")
        subprocess.run(pixels_to_photons_command, stdout=log_output, stderr=subprocess.STDOUT)

    # Remove the unzipped tpx3 file after processing
    os.remove(input_file)

    
    
#-------------------------------------------------------------------------------------
def process_photons_to_events(input_dir="", list_file_name="", output_dir="./", dspace=3, dtime=100e-6, durationMax=500e-6, log_dir="./"):
    """
    self.list_file_dir, list_file, self.event_file_dir, self.dspace, self.dtime, self.durationMax, self.log_file_dir
    Runs empir_photon2event with the user-defined parameters.
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
def process_existing_tpx3_files(config, process_pool, file_counter, lock):
    existing_tpx3_files = [f for f in os.listdir(config.tpx3_file_dir) if f.endswith(".tpx3.zip")]
    for tpx3_file in existing_tpx3_files:
        process_pool.apply_async(process_pixels_to_photons, args=(
            config.tpx3_file_dir, 
            tpx3_file, 
            config.pixel_d_space, 
            config.pixel_d_time, 
            config.pixel_min_number, 
            config.use_tdc1, 
            config.list_file_dir, 
            config.log_file_dir))

        # Increment the file counter
        with lock:
            file_counter.value += 1


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

