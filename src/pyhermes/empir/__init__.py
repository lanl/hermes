"""
pyhermes.empir package

This package contains modules and functions related to analysis with the EMPIR commercial code.
"""

# Import necessary modules or functions here
from .core import (
    empirConfig,
    zip_file,
    check_for_files,
    process_pixels_to_photons,
    process_photons_to_events,
    process_event_files_to_image_stack,
    export_pixel_activations,
    process_existing_tpx3_files,
    process_existing_photon_files,
    process_existing_event_files,
    unpack_pixel_activations
)

# Import models
from .models import (
    PixelToPhotonParams,
    PhotonToEventParams,
    EventToImageParams,
    DirectoryStructure
)

# Define package-level variables or functions if needed
__all__ = [
    'empirConfig',
    'zip_file',
    'check_for_files',
    'process_pixels_to_photons',
    'process_photons_to_events',
    'process_event_files_to_image_stack',
    'export_pixel_activations',
    'process_existing_tpx3_files',
    'process_existing_photon_files',
    'process_existing_event_files',
    'unpack_pixel_activations',
    'PixelToPhotonParams',
    'PhotonToEventParams',
    'EventToImageParams',
    'DirectoryStructure'
]
# Initialize package-level settings or configurations if needed
def initialize():
    pass

# Any other package-level code can go here