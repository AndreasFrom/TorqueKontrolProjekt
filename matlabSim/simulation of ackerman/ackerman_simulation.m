% Parameters
wheelbase = 0.29;     % [m]
trackWidth = 0.2;     % [m] - distance between left and right wheels
turnRadius = 0.5;     % [m]
steeringAngle = atan(wheelbase / turnRadius);  % [rad]
velocity = 1;         % [m/s] 
sampleTime = 0.01;    % [s]
timeEnd = 10;         % [s]
oversteerThreshold = 0.9;
slipFactor = 0.01;
tVec = 0:sampleTime:timeEnd;

% Initial pose: [x, y, theta, phi]
initPose = [0; 0; 0; steeringAngle];

% Create Ackermann kinematic object
car = ackermannKinematics(WheelBase=wheelbase);

% Preallocate state
pose = zeros(length(tVec), 4);
pose(1,:) = initPose';

% Store wheel velocities (for analysis/visualization)
wheelSpeeds = zeros(length(tVec), 4); % [v_FL, v_FR, v_RL, v_RR]

% Set up the figure
figure;
hold on;
axis equal;
xlim([-1.2 1.2]);  
ylim([-1.2 1.2]);  
xlabel('X [m]');
ylabel('Y [m]');
title('Ackermann Steering with Wheel Speed Estimation');

% Initialize animated line
trajLine = animatedline('LineWidth', 2);

% Simulate using Ackermann kinematics but estimate wheel speeds
for i = 2:length(tVec)
    % Current steering angle
    phi = pose(i-1, 4);
    
    % Inputs (same as before)
    v = velocity;
    if velocity > oversteerThreshold
        psidot = slipFactor;  % oversteer behavior
    else
        psidot = 0;
    end
    
    % Compute derivative (using ackermannKinematics)
    dydt = derivative(car, pose(i-1,:)', [v psidot]);
    
    % Euler integration
    pose(i,:) = pose(i-1,:) + sampleTime * dydt';
    
    % --- Estimate individual wheel speeds ---
    if abs(phi) > 0.001 % If steering (not straight)
        % Instantaneous turning radius (from rear axle)
        R = wheelbase / tan(phi);
        
        % Angular velocity (omega = v / R)
        omega = v / R;
        
        % Front left and right wheel speeds (Ackermann geometry)
        v_FL = omega * sqrt((R + trackWidth/2)^2 + wheelbase^2);
        v_FR = omega * sqrt((R - trackWidth/2)^2 + wheelbase^2);
        
        % Rear left and right wheel speeds (same as turning radius)
        v_RL = omega * (R + trackWidth/2);
        v_RR = omega * (R - trackWidth/2);
    end
    
    % Apply slip effects (if oversteering)
    if v > oversteerThreshold
        v_RL = v_RL * (1 - slipFactor); % Rear left slips
        v_RR = v_RR * (1 - slipFactor); % Rear right slips
    end
    
    % Store wheel speeds (optional, for plotting/logging)
    wheelSpeeds(i,:) = [v_FL, v_FR, v_RL, v_RR];
    
    % Add point to animated line
    addpoints(trajLine, pose(i,1), pose(i,2));
    
    % Optional: Display wheel speeds in real-time
    if mod(i, 10) == 0 % Update every 10 steps to reduce lag
        clc;
        fprintf('Wheel Speeds:\n');
        fprintf('FL: %.2f m/s, FR: %.2f m/s\n', v_FL, v_FR);
        fprintf('RL: %.2f m/s, RR: %.2f m/s\n', v_RL, v_RR);
    end
    
    % Draw current state
    drawnow limitrate;
    pause(0.01);
end