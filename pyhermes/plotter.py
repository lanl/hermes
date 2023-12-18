import numpy as np
import matplotlib.pyplot as plt
from matplotlib import colors
import matplotlib.animation as animation

#------------------------------------------------------------------------------
#  	plotTimeResults:	Plots events after they are sorted into timeGroups
#
#	args: 		array of type signleEventData, epsilon, min number of samples
# 	returns: 	nothing. timeGroup types are updated in event array
#------------------------------------------------------------------------------
def plotTimeResults(data):
	fig = plt.figure()
	ax = fig.add_subplot(111, projection='3d')
	
	# Get unique timeGroup values
	unique_timeGroups = np.unique(data['timeGroup'])
	
	# Create a scatter plot for each timeGroup
	for timeGroup in unique_timeGroups:	
		
		# filter data based on timeGroups
		filteredData = data[data['timeGroup'] == timeGroup]
		
		ax.scatter(filteredData['xpixel'],filteredData['ypixel'],filteredData['ToA_final'],label=f'timeGroup = {timeGroup}')

	plt.show()


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
	

























