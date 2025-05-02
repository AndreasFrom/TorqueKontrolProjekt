import pandas as pd
import numpy as np
import plotly.graph_objects as go
from plotly.subplots import make_subplots
from datetime import datetime
import plotly.io as pio

# Load and prepare data
df = pd.read_csv('REE3_Om0-00025_Om1-005_Eta-000000007_v09.csv')
df.columns = df.columns.str.strip()
motors = ['MU0', 'MU1', 'MU2', 'MU3']
colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728']

# Create main figure with 12 subplots
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
        'ICO Learning omega move'
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

# Save main report
pio.write_html(fig, file='robot_sensor_report.html', auto_open=True)
print("✅ Report successfully saved as robot_sensor_report.html")

# =========================
# FFT of gyro_z
# =========================

# Extract gyro_z data and compute sampling rate
gyro_z = df['gyro_z'].values
timestamps = pd.to_datetime(df['timestamp'])
time_deltas = (timestamps - timestamps.shift()).dt.total_seconds().dropna()
sampling_rate = 1 / time_deltas.mean()  # Hz

# Compute FFT
n = len(gyro_z)
fft_vals = np.fft.fft(gyro_z)
fft_freq = np.fft.fftfreq(n, d=1/sampling_rate)

# Keep only the positive frequencies
positive_freqs = fft_freq[:n // 2]
positive_magnitudes = np.abs(fft_vals[:n // 2])

# Create FFT Plot
fft_fig = go.Figure()
fft_fig.add_trace(go.Scatter(x=positive_freqs, y=positive_magnitudes,
                             mode='lines', name='FFT of Gyro Z', line=dict(color='blue')))
fft_fig.update_layout(
    title='FFT of Gyro Z',
    xaxis_title='Frequency (Hz)',
    yaxis_title='Amplitude',
    height=500
)

# Save FFT plot
pio.write_html(fft_fig, file='gyro_z_fft_plot.html', auto_open=True)
print("✅ FFT plot saved as gyro_z_fft_plot.html")
