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
    export_photons
)

# Import models
from .models import (
    PixelToPhotonParams,
    PhotonToEventParams,
    EventToImageParams,
    DirectoryStructure,
    PixelActivation,
    ExportedPixels
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
    'export_photons',
    'PixelToPhotonParams',
    'PhotonToEventParams',
    'EventToImageParams',
    'DirectoryStructure',
    'PixelActivation',
    'ExportedPixels'
]
# Initialize package-level settings or configurations if needed
def initialize():
    pass

# Any other package-level code can go here