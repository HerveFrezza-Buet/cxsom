import matplotlib.pyplot as plt
import numpy as np

_pi_3  = 1.0471975511965976  # pi/3
_2pi_3 = 2.0943951023931953  # 2pi/3
_2pi   = 6.283185307179586   # 2pi
_3_pi  = 0.954929658551372   # 1/(pi/3) = 3/pi
_5pi_3 = 5.235987755982989   # 5pi/3

def grid_of_1D(ax, data, cmap='viridis'):
    ax.imshow(data, cmap=cmap, extent=(0., 1., 0., 1.))

def component_level(theta):
    theta[theta < 0] += _2pi
    theta[theta > _2pi] -= _2pi
    level = np.ones(theta.shape, dtype = float)

    theta_inf_2pi_3 = theta < _2pi_3
    theta_inf_pi    = theta < np.pi
    theta_sup_5pi_3 = theta >= _5pi_3

    theta_a = theta_inf_2pi_3
    theta_b = np.logical_and(np.logical_not(theta_inf_2pi_3), theta_inf_pi)
    theta_d = theta_sup_5pi_3

    level[theta_a] = 0
    level[theta_b] = (theta[theta_b] - _2pi_3) * _3_pi
    level[theta_d] = 1.0 - (theta[theta_d] - _5pi_3) * _3_pi
    return level

def grid_of_2D(ax, data):
    centered_data = 2*data - 1.0
    x = centered_data[..., 1]
    y = centered_data[..., 0]
    norm = np.sqrt(x**2 + y**2) * 0.7071067811865475
    _norm = 1.0 - norm
    theta = np.arctan2(y, x)

    r = component_level(theta)
    g = component_level(theta + _2pi_3)
    b = component_level(theta - _2pi_3)

    R = norm*r + _norm
    G = norm*g + _norm
    B = norm*b + _norm
    
    img = np.dstack((R, G, B))
    ax.imshow(img, extent=(0., 1., 0., 1.))
    
    
