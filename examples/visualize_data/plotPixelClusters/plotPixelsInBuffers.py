import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.colors import LogNorm

# Define the path to your file
file_path = 'pixelID.txt'

# Define column names based on your data structure
column_names = ['buffer_number', 'signal_type', 'x_pixel', 'y_pixel', 'toa', 'tot', 'groupID']

# Read the file into a DataFrame, assuming whitespace delimiter
print("Loading pixel data!")
df = pd.read_csv(file_path, sep='\s+', header=None, names=column_names)

# Filtering for a specific buffer number
buffer_number = 19
filtered_df = df[df['buffer_number'] == buffer_number]

# Separating Pixels and TDCs
pixels_df = filtered_df[filtered_df['signal_type'] == 'Pixel'].copy()
tdcs_df = filtered_df[filtered_df['signal_type'] == 'TDC'].copy()

# Prepare color map for Pixels, including black for groupID = -1
color_map = {-1: [0, 0, 0]}  # Black in RGB
unique_groups = np.unique(pixels_df['groupID'])
for group in unique_groups:
    if group != -1:
        color_map[group] = np.random.rand(3,)  # Random color for each groupID
color_values = np.array([color_map[group] for group in pixels_df['groupID']])

# Initialize a 256x256 array with zeros for TOT integration
image_array = np.zeros((256, 256))

# Accumulate 'tot' values for each pixel position
for index, row in pixels_df.iterrows():
    x = int(row['x_pixel'])
    y = int(row['y_pixel'])
    tot = row['tot']
    # Assuming x and y pixel indices are within the 0-255 range
    image_array[y, x] += tot  # Sum 'tot' values for the pixel position

# Create the plots
fig = plt.figure(figsize=(14, 6))

# 3D plot for Pixels and TDCs
ax1 = fig.add_subplot(121, projection='3d')
ax1.scatter(pixels_df['x_pixel'], pixels_df['y_pixel'], pixels_df['toa'], c=color_values, label='Pixels')
ax1.scatter(tdcs_df['x_pixel'], tdcs_df['y_pixel'], tdcs_df['toa'], color='black', label='TDC', s=10)  # Adjust size as needed
ax1.set_xlabel('X Pixel')
ax1.set_ylabel('Y Pixel')
ax1.set_zlabel('TOA')
ax1.set_title('3D Plot of Pixels and TDCs')

# 2D Image plot for integrated TOT values with LogNorm down to zero
ax2 = fig.add_subplot(122)
# Using a small positive value close to zero for vmin in LogNorm
log_norm = LogNorm(vmin=np.min(image_array[image_array > 0]), vmax=np.max(image_array), clip=True)
img = ax2.imshow(image_array, cmap='viridis', norm=log_norm, aspect='equal')
ax2.set_title('Integrated TOT as Image')
ax2.set_xlabel('X Pixel')
ax2.set_ylabel('Y Pixel')
plt.colorbar(img, ax=ax2, label='Integrated TOT (Log Scale)')
ax2.set_xticks([])
ax2.set_yticks([])  # Remove ticks for a cleaner image

plt.tight_layout()
plt.show()
