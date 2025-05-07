import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import glob
import os

# Modify this pattern to match your data location
file_list = glob.glob('Torque-Measurements/*.csv')[:8]

if not file_list:
    print("No CSV files found. Check the path or pattern.")
else:
    current_columns = ['MU0current', 'MU1current', 'MU2current', 'MU3current']
    data = []
    filenames = []

    for file in file_list:
        try:
            df = pd.read_csv(file)
            df.columns = df.columns.str.strip()  # Remove extra spaces in column names

            # Check if all required columns exist
            if not all(col in df.columns for col in current_columns):
                print(f"{os.path.basename(file)}: missing required columns, skipping.")
                continue

            # Compute per-column averages and total
            avg = df[current_columns].mean()
            total = avg.sum()
            avg['Sum'] = total

            data.append(avg)
            filenames.append(os.path.basename(file))

            print(f"\nAverages for {os.path.basename(file)}:")
            for col, val in avg.items():
                print(f"  {col}: {val:.3f}")

        except Exception as e:
            print(f"Error processing {file}: {e}")

    if not data:
        print("No valid data to plot.")
    else:
        # Convert to DataFrame for plotting
        df_plot = pd.DataFrame(data, index=filenames)

        print("\nPlotting...")
        ax = df_plot.plot(kind='bar', figsize=(12, 6), colormap='tab10')
        plt.title('Average Currents')
        plt.xlabel('CSV File')
        plt.ylabel('Current')
        plt.xticks(rotation=45, ha='right')
        plt.legend(title='Current Type', bbox_to_anchor=(1.05, 1), loc='upper left')
        plt.grid(True, axis='y')

        # Linear regression for the 'Sum' column
        x = np.arange(len(df_plot))  # X-axis values (indices of the files)
        y = df_plot['Sum'].values   # Y-axis values (Sum column)
        coeffs = np.polyfit(x, y, 1)  # Linear regression (degree 1)
        regression_line = np.polyval(coeffs, x)  # Compute regression line

        # Plot the regression line
        #plt.plot(x, regression_line, color='red', linestyle='--', label='Linear Regression (Sum)')

        # Add regression formula as text on the plot
        #formula = f"y = {coeffs[0]:.2f}x + {coeffs[1]:.2f}"
        #plt.text(0.5, max(y) + 0.1, formula, color='red', fontsize=12, ha='center')

        # Adjust legend position to avoid overlapping
        plt.legend(loc='upper left', bbox_to_anchor=(0.01, 1.0))

        plt.tight_layout()
        plt.show()



#file_list = glob.glob('Torque-Measurements/*.csv')[:8]
"""
v0.3
MU0current    0.321632
MU1current    0.329432
MU2current    0.047084
MU3current    0.204800

v0.4
MU0current    0.051925
MU1current    0.433703
MU2current    0.045612
MU3current    0.548809 

v0.5
MU0current    0.092777
MU1current    0.447740
MU2current    0.058730
MU3current    0.468464

v0.6
MU0current    0.411851
MU1current    0.441740
MU2current    0.061326
MU3current    0.211906

v0.7
MU0current    0.318747
MU1current    0.391499
MU2current    0.075577
MU3current    0.411327

v0.8
MU0current    0.297090
MU1current    0.548320
MU2current    0.094102
MU3current    0.237500

v0.9
MU0current    0.344591
MU1current    0.600216
MU2current    0.119784
MU3current    0.179310

v1.0
MU0current    0.362463
MU1current    0.552960
MU2current    0.107188
MU3current    0.244375

"""