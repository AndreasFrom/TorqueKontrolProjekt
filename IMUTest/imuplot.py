import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from datetime import datetime, timedelta
import plotly.io as pio

# Load and prepare data
df = pd.read_csv('filtertest.CSV')
df.columns = df.columns.str.strip()

# Generate a synthetic timestamp column if it doesn't exist
# Assuming a fixed time interval of 1 second between rows
df['timestamp'] = pd.date_range(start=datetime.now(), periods=len(df), freq='S')

# Create a comprehensive figure with subplots
fig = make_subplots(
    rows=3, cols=1,
    shared_xaxes=True,
    vertical_spacing=0.05,
    subplot_titles=(
        'X Data Comparison (accel_x vs filter_x)',
        'Y Data Comparison (accel_y vs filter_y)',
        'Z Data Comparison (gyro_z vs filter_z)'
    ),
    row_heights=[0.33, 0.33, 0.33]
)

# Add X data comparison
fig.add_trace(
    go.Scatter(x=df['timestamp'], y=df['accel_x'], name='Accel X', line=dict(color='red')),
    row=1, col=1
)
fig.add_trace(
    go.Scatter(x=df['timestamp'], y=df['filter_x'], name='Filter X', line=dict(color='blue')),
    row=1, col=1
)

# Add Y data comparison
fig.add_trace(
    go.Scatter(x=df['timestamp'], y=df['accel_y'], name='Accel Y', line=dict(color='green')),
    row=2, col=1
)
fig.add_trace(
    go.Scatter(x=df['timestamp'], y=df['filter_y'], name='Filter Y', line=dict(color='orange')),
    row=2, col=1
)

# Add Z data comparison
fig.add_trace(
    go.Scatter(x=df['timestamp'], y=df['gyro_z'], name='Gyro Z', line=dict(color='purple')),
    row=3, col=1
)
fig.add_trace(
    go.Scatter(x=df['timestamp'], y=df['filter_z'], name='Filter Z', line=dict(color='brown')),
    row=3, col=1
)

# Update layout
fig.update_layout(
    height=1200,
    title_text=f"Sensor Data Comparison - {datetime.now().strftime('%Y-%m-%d %H:%M')}",
    showlegend=True,
    hovermode='x unified'
)

# Save as HTML
pio.write_html(fig, file='sensor_data_comparison.html', auto_open=True)
print("Report successfully saved as sensor_data_comparison.html")