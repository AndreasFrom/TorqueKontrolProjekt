import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from datetime import datetime
import plotly.io as pio

# Load and prepare data
df = pd.read_csv('output_2025-04-15_12-31-15_1.csv')
df.columns = df.columns.str.strip()
motors = ['MU0', 'MU1', 'MU2', 'MU3']
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']


fig = make_subplots(
    rows=12,  
    cols=1,
    shared_xaxes=True,
    vertical_spacing=0.05,
    subplot_titles=(
        'Acceleration X (m/s²)',
        'Acceleration Y (m/s²)',
        'Gyro Z (deg°/s)',
        'Actual Velocity (m/s)',
        'Motor Setpoints vs Actual Values',
        'All Motor Currents',
        'ICO Learning error yaw',
        'ICO Learning updated yaw',
        'ICO Learning error velocity',
        'ICO Learning updated velocity',
        'ICO Learning omega yaw',
        'ICO Learning omega move'  # Now 12 titles for 12 rows
    )
)


# Add IMU data


fig.add_trace(go.Scatter(x=df['timestamp'], y=df['acc_x'], name='Acc X', line=dict(color='red')), row=1, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['acc_y'], name='Acc Y', line=dict(color='green')), row=2, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['gyro_z'], name='Gyro Z', line=dict(color='blue')), row=3, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['actual_velocity'], name='Actual Velocity', line=dict(color='purple')), row=4, col=1)
# Add motor setpoints vs actual values
for i, motor in enumerate(motors):
    fig.add_trace(go.Scatter(x=df['timestamp'], y=df[f'{motor}setpoint'], 
                  name=f'{motor} Setpoint', line=dict(color=colors[i], dash='dash')), row=5, col=1)
    fig.add_trace(go.Scatter(x=df['timestamp'], y=df[f'{motor}value'], 
                  name=f'{motor} Actual', line=dict(color=colors[i])), row=5, col=1)

# Add combined motor currents
for i, motor in enumerate(motors):
    fig.add_trace(go.Scatter(x=df['timestamp'], y=df[f'{motor}current'], 
                  name=f'{motor} Current', line=dict(color=colors[i])), row=6, col=1)

# Add ICO learning data
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['error_yaw'], name='Error Yaw', line=dict(color='purple')), row=7, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['updated_yaw'], name='Updated Yaw', line=dict(color='orange')), row=8, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['error_velocity'], name='Error Velocity', line=dict(color='pink')), row=9, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['updated_velocity'], name='Updated Velocity', line=dict(color='brown')), row=10, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['omega_yaw'], name='Omega Yaw', line=dict(color='cyan')), row=11, col=1)
fig.add_trace(go.Scatter(x=df['timestamp'], y=df['omega_move'], name='Omega Move', line=dict(color='magenta')), row=12, col=1)

# Update layout
fig.update_layout(
    height=1800,
    title_text=f"Robot Sensor Report - {datetime.now().strftime('%Y-%m-%d %H:%M')}",
    showlegend=True,
    hovermode='x unified'
)

# Save as HTML and display
pio.write_html(fig, file='robot_sensor_report.html', auto_open=True)
print("✅ Report successfully saved as robot_sensor_report.html")
