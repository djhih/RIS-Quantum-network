import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Create empty lists for each category
user_positions = []
ris_positions = []

# Read file and categorize data
try:
    with open('/home/waa/RIS/code/data/output/loc_data/raw/dataset_2_9.txt', 'r') as file:
        for line in file:
            if line.strip():  # Skip empty lines
                parts = line.strip().split()
                try:
                    # 檔案格式為: [category] [x] [y]
                    # 例如: user 1.0 2.0 或 ris 3.0 4.0
                    if len(parts) >= 3 and parts[0].lower() in ["user", "ris"]:
                        category = parts[0].lower()
                        x, y = float(parts[1]), float(parts[2])
                        
                        if category == "user":
                            user_positions.append((x, y))
                        elif category == "ris":
                            ris_positions.append((x, y))
                except ValueError:
                    print(f"Warning: Could not parse values in line: {line.strip()}")
    
    if not user_positions and not ris_positions:
        print("Warning: No valid data found, using default data")
        user_positions = [(1, 2), (3, 4), (2, 5)]
        ris_positions = [(6, 1), (7, 3)]
except FileNotFoundError:
    print("Warning: Location file not found, using default data")
    user_positions = [(1, 2), (3, 4), (2, 5)]
    ris_positions = [(6, 1), (7, 3)]

# Extract x, y coordinates for each category
user_x = [p[0] for p in user_positions]
user_y = [p[1] for p in user_positions]
ris_x = [p[0] for p in ris_positions]
ris_y = [p[1] for p in ris_positions]

# Create scatter plot with different colors
plt.figure(figsize=(6, 6))
plt.scatter(user_x, user_y, c='blue', marker='o', label='User')
plt.scatter(ris_x, ris_y, c='red', marker='^', label='RIS')

# Add circles with radius 20 around each RIS point
for x, y in zip(ris_x, ris_y):
    circle = plt.Circle((x, y), 20, fill=False, color='red', linestyle='--', alpha=0.7)
    plt.gca().add_patch(circle)

# Add labels and grid
plt.title("Location Distribution")
plt.xlabel("X")
plt.ylabel("Y")
plt.grid(True)
plt.axis("equal")  # Keep x, y ratio consistent
plt.legend()  # Add legend to distinguish categories

# Save figure
plt.savefig('/home/waa/RIS/code/data/output/loc_distribution.png', dpi=300, bbox_inches='tight')
plt.close()  # Close the figure to free memory
