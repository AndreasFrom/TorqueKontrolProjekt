import pandas as pd
import glob
import os
import numpy as np
import plotly.graph_objects as go
import webbrowser

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
            df.columns = df.columns.str.strip()

            if not all(col in df.columns for col in current_columns):
                print(f"{os.path.basename(file)}: missing required columns, skipping.")
                continue

            avg = df[current_columns].mean()
            avg['TotalCurrent'] = avg.sum()
            #avg['GroupA'] = avg['MU0current'] + avg['MU2current']
            #avg['GroupB'] = avg['MU1current'] + avg['MU3current']

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
        df_plot = pd.DataFrame(data, index=filenames)

        all_columns = ['MU0current', 'MU1current', 'MU2current', 'MU3current', 'TotalCurrent']#, 'GroupA', 'GroupB']

        color_map = {
            'MU0current': '#1f77b4',
            'MU1current': '#ff7f0e',
            'MU2current': '#2ca02c',
            'MU3current': '#d62728',
            'TotalCurrent': '#9467bd',
            'GroupA': '#8c564b',
            'GroupB': '#e377c2'
        }

        fig = go.Figure()
        x_vals = np.arange(len(filenames))
        annotations = []

        # Add bar traces
        for col in all_columns:
            fig.add_trace(go.Bar(
                x=filenames,
                y=df_plot[col],
                name=col,
                marker_color=color_map[col]
            ))

        # Add linear regression lines + equation annotations
        for col in all_columns:
            y_vals = df_plot[col].values
            coeffs = np.polyfit(x_vals, y_vals, 1)
            y_fit = np.polyval(coeffs, x_vals)

            a, b = coeffs
            equation = f"y = {a:.3f}x + {b:.3f}"

            fig.add_trace(go.Scatter(
                x=filenames,
                y=y_fit,
                mode='lines',
                name=f'{col} trend',
                line=dict(dash='dash', color=color_map[col])
            ))

            annotations.append(dict(
                x=filenames[-1],
                y=y_fit[-1],
                text=equation,
                showarrow=False,
                font=dict(color=color_map[col], size=12),
                xanchor='left'
            ))

        fig.update_layout(
            title='Average Currents per File (with Group Trends and Formulas)',
            xaxis_title='CSV File',
            yaxis_title='Current',
            barmode='group',
            legend_title='Current Type',
            annotations=annotations,
            height=700,
            width=1100
        )

        output_path = "interactive_currents_plot.html"
        fig.write_html(output_path)
        print(f"\nInteractive plot saved as: {output_path}")
        webbrowser.open(f"file://{os.path.abspath(output_path)}")
