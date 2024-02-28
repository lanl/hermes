import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import LogNorm
import matplotlib.animation as animation


"""
#------------------------------------------------------------------------------
Class PlotPixelsInBuffer_3D
Description:
    This class is designed to visualize pixel data from a given dataset for a 
    specific buffer number within the dataset.
    
Attributes:
    filepath (str): The file path to the dataset.
    buffer_number (int): The specific buffer number to visualize.
    df (DataFrame): The loaded dataset as a pandas DataFrame.
    
Methods:
    load_data: Loads the dataset from the specified file path.
    plot_pixels_vs_toa: Plots pixels in a 3D space based on their ToA.
    generate_buffer_image: Generates an image representation of pixels based on ToT values.
#------------------------------------------------------------------------------
"""
class PlotPixelsInSingeBuffer_3D:
    def __init__(self, filepath, buffer_number):
        # Initialize the class with file path and buffer number, and load the dataset.
        self.filepath = filepath
        self.buffer_number = buffer_number
        self.df = self.load_data()

    # TODO: Add capabiltiy to load in different types of data (csv vs. binary)
    def load_data(self):
        # Define column names for the dataset and load it into a pandas DataFrame.
        column_names = ['buffer_number', 'signal_type', 'x_pixel', 'y_pixel', 'toa', 'tot', 'groupID']
        df = pd.read_csv(self.filepath, sep='\s+', header=None, names=column_names)
        print("Pixel data loaded!")
        return df

    def plot_pixels_vs_toa(self, ax=None):
        # Filter the dataset for the specific buffer number and separate pixels and TDCs.
        filtered_df = self.df[self.df['buffer_number'] == self.buffer_number]
        pixels_df = filtered_df[filtered_df['signal_type'] == 'Pixel']
        tdcs_df = filtered_df[filtered_df['signal_type'] == 'TDC'].copy()

        # Prepare a color map for pixels, with a special case for groupID = -1 (black).
        color_map = {-1: [0, 0, 0]}  # Black in RGB for TDCs and groupID = -1 pixels.
        unique_groups = np.unique(pixels_df['groupID'])
        for group in unique_groups:
            if group != -1:
                color_map[group] = np.random.rand(3,)  # Assign a random color for each groupID.
        color_values = np.array([color_map[group] for group in pixels_df['groupID']])
        
        # Create a 3D plot for pixels and TDCs.
        if ax is None:
            fig = plt.figure(figsize=(10, 7))
            ax = fig.add_subplot(111, projection='3d')
        ax.scatter(pixels_df['x_pixel'], pixels_df['y_pixel'], pixels_df['toa'], c=color_values, label='Pixels')
        ax.scatter(tdcs_df['x_pixel'], tdcs_df['y_pixel'], tdcs_df['toa'], color='black', label='TDC', s=10)
        ax.set_xlabel('X Pixel')
        ax.set_ylabel('Y Pixel')
        ax.set_zlabel('TOA')
        ax.set_title(f'3D Plot of Pixels vs. ToA for Buffer Number {self.buffer_number}')

    def generate_buffer_image(self, log=False, ax=None):
        # Filter the dataset for the specific buffer number to visualize pixel intensities based on ToT.
        filtered_df = self.df[self.df['buffer_number'] == self.buffer_number]
        pixels_df = filtered_df[filtered_df['signal_type'] == 'Pixel']
        
        # Initialize a 256x256 array with zeros for ToT integration.
        image_array = np.zeros((256, 256))
        for index, row in pixels_df.iterrows():
            x, y, tot = int(row['x_pixel']), int(row['y_pixel']), row['tot']
            image_array[y, x] += tot  # Accumulate 'tot' values for each pixel position.
        
        if ax is None:
            fig, ax = plt.subplots()
        
        # Choose logarithmic or linear scale based on 'log' parameter.
        if log:
            if np.any(image_array > 0):
                log_norm = LogNorm(vmin=np.min(image_array[image_array > 0]), vmax=np.max(image_array), clip=True)
                img = ax.imshow(image_array, cmap='viridis', norm=log_norm, aspect='equal')
                label = 'Integrated TOT (Log Scale)'
            else:
                print("No non-zero TOT values found. Displaying in linear scale.")
                img = ax.imshow(image_array, cmap='viridis', aspect='equal')
                label = 'Integrated TOT'
        else:
            img = ax.imshow(image_array, cmap='viridis', aspect='equal')
            label = 'Integrated TOT'
        
        # Configure the plot appearance.
        ax.set_title(f'Integrated TOT as Image for Buffer Number {self.buffer_number}')
        ax.set_xlabel('X Pixel')
        ax.set_ylabel('Y Pixel')
        plt.colorbar(img, label=label)
        ax.set_xticks([])
        ax.set_yticks([])  # Hide axis ticks for a cleaner image presentation.



        


#------------------------------------------------------------------------------
#  	plotFianlResults:	Plots events after they are sorted into both timeGroups
#						and spaceGroups
#
#	args: 		array of type signleEventData,
# 	returns: 	nothing. timeGroup types are updated in event array
#------------------------------------------------------------------------------
def plotFinalResults(data):
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')
    
    # Get unique timeGroup values
    unique_timeGroups = np.unique(data['timeGroup'])
    
    # Create a scatter plot for each timeGroup
    for timeGroup in unique_timeGroups:	
        
        # filter data based on timeGroups
        filteredTimeData = data[data['timeGroup'] == timeGroup]

        # Get unique spaceGroup values
        unique_spaceGroups = np.unique(filteredTimeData['spaceGroup'])
        
        for spaceGroup in unique_spaceGroups:
            # filter data based on timeGroups
            filteredSpaceData = data[data['spaceGroup'] == spaceGroup]

            ax.scatter(filteredSpaceData['ToA_final'],filteredSpaceData['xpixel'],filteredSpaceData['ypixel'])

    plt.show()


def plotHistOneD(data, signalType, dataType,NumOfBins):
    signalsOfType = data[data['typeOfEvent'] == signalType]
    print(np.min(signalsOfType[dataType]))

    plt.hist(signalsOfType[dataType], bins=NumOfBins)
    plt.xlabel(dataType)
    plt.show()

#------------------------------------------------------------------------------
#  	animateData:	Creates an animation of the data by plotting x and y pixels
#					while stepping through ToA. 
#
#	args: 		array of type signleEventData,
# 	returns: 	Nothing. An animation gif is saved to file.
#------------------------------------------------------------------------------
def animateData(data,startTime=None,stopTime=None,numOfFrames=100):

    pixelData = data[data['typeOfEvent'] == 2] #grab only pixel signals

    if startTime == None:
        startTime = np.min(pixelData['ToA_final'])
        print("Start time:{}".format(startTime))

    if stopTime == None:
        stopTime = np.max(pixelData['ToA_final'])
        print("Stop time:{}".format(stopTime))

    frameBin = (stopTime-startTime)/numOfFrames
    print("Each frame bin is {} seconds".format(frameBin))

    # Create an array to store unique group IDs based on space-time combos
    groupIDs = np.zeros((len(pixelData),), dtype=np.int32)
    for i,signal in enumerate(pixelData):
        groupIDs[i] = int(str(signal['timeGroup']) + str(signal['spaceGroup']))

    # create frame spacing based on 
    frames=np.arange(startTime, stopTime, frameBin)

    # Initialize figure and axes
    fig, ax = plt.subplots()

    # Initial plot
    scatterPlot = ax.scatter([], [], c=[])

    # Set tolerance for each frame
    eps = frameBin*2

    def init():
        ax.set_xlim(0, 255)
        ax.set_ylim(0, 255)
        ax.set_title('ToA_final: 0.0')  # initial title
        
        return scatterPlot,

    def update(frame):

        # Only take events where 'ToA_final' falls within the frame +/- eps
        frame_indices = np.where((pixelData['ToA_final'] >= frame - eps) & (pixelData['ToA_final'] < frame + eps)) 
        currentEvents = pixelData[frame_indices]

        currentGroupIDs = groupIDs[frame_indices]
        
        scatterPlot.set_offsets(np.c_[currentEvents['xpixel'], currentEvents['ypixel']])
        scatterPlot.set_array(currentGroupIDs)  # Update colors based on groupID
        ax.set_title(f'ToA_final: {frame:.6f} seconds')  # update title with current ToaFinal
        
        return scatterPlot,

    ani = animation.FuncAnimation(fig, update, frames, init_func=init, blit=True)

    # Save the animation as a gif
    ani.save("animation.gif", writer='pillow', fps=10)
    

























