from pydantic import BaseModel, Field
from typing import Optional

class DirectoryStructure(BaseModel):
    destination_dir: str = Field(..., description="Location of run directory")
    log_file_dir: str = Field(..., description="Location of log files")
    tpx3_file_dir: str = Field(..., description="Location of TPX3 files")
    list_file_dir: str = Field(..., description="Location of list files")
    event_file_dir: str = Field(..., description="Location of event files")
    final_file_dir: str = Field(..., description="Location of final image files")
    export_file_dir: str = Field(..., description="Location of exported info files")

class PixelToPhotonParams(BaseModel):
    d_space: int = Field(default=10, description="Distance in space for pixel search [px]")
    d_time: float = Field(default=1e-6, description="Distance in time for pixel search [s]")
    min_number: int = Field(default=2, description="Minimum number of pixels in a photon event")
    use_tdc1: bool = Field(default=False, description="Use TDC1 as trigger input")

class PhotonToEventParams(BaseModel):
    d_space: Optional[int] = Field(default=None, description="Distance in space for photon search [px]")
    d_time: Optional[float] = Field(default=None, description="Distance in time for photon search [s]")
    max_duration: Optional[float] = Field(default=None, description="Maximum duration to look for photons [s]")
    d_time_extF: Optional[float] = Field(default=None, description="Extents duration by multiple of time difference to last photon")

class EventToImageParams(BaseModel):
    size_x: int = Field(default=512, description="Number of pixels in x direction for the final image")
    size_y: int = Field(default=512, description="Number of pixels in y direction for the final image")
    nPhotons_min: int = Field(default=0, description="Minimum number of photons for image processing")
    nPhotons_max: int = Field(default=18446744073709551615, description="Maximum number of photons for image processing")
    time_extTrigger: bool = Field(default=False, description="Use time relative to external trigger for binning")
    time_res_s: Optional[float] = Field(default=None, description="Timing resolution in seconds for 3D image sequence")
    time_limit: Optional[int] = Field(default=None, description="Maximum of time bins for the 3D image sequence")
    psd_min: float = Field(default=0, description="Minimum PSD value")
    psd_max: float = Field(default=100, description="Maximum PSD value")