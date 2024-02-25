import pyhermes.plotter as ph_plotter   # pyhermes plotter class. 
import matplotlib.pyplot as plt

file_path = 'pixelID_example.txt'       # Define the path to your file
buffer_number = 6                       # Specify the buffer number you're interested in

# Initiate a pyhermes plotter class
plotter = ph_plotter.PlotPixelsInBuffer_3D(file_path, buffer_number)

# For plotting single plots
plotter.plot_pixels_vs_toa()
plt.show()

plotter.generate_buffer_image(log=True)
plt.show()

# For combined plotting you need to first set up the figure and axes. 
fig3 = plt.figure(figsize=(14, 7))              # Settup figure with layout (side-by)
ax1 = fig3.add_subplot(121, projection='3d')    # Define the first axes (if using plot_pixels_vs_toa specify 3D)
ax2 = fig3.add_subplot(122)                     # Define the second axes.
plotter.plot_pixels_vs_toa(ax=ax1)              # Plot on the first subplot
plotter.generate_buffer_image(log=True, ax=ax2) # Plot on the second subplot with log scale
plt.tight_layout()                              # Make a tight_layout
plt.show()                                      # show plot