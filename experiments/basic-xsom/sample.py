import numpy as np
import sys

banana_r     = .5
banana_r_2   = banana_r / 2.0
banana_r_3   = banana_r / 3.0
banana_2_r_3 = 2 * banana_r_3 
banana_L1    = np.pi * .5 * banana_r
banana_L2    = banana_2_r_3
banana_L3    = np.pi * banana_r_3
banana_L4    = banana_L2
banana_L5    = banana_L3 * .5
banana_l1    = banana_L1
banana_l2    = banana_l1 + banana_L2
banana_l3    = banana_l2 + banana_L3
banana_l4    = banana_l3 + banana_L4
banana_l5    = banana_l4 + banana_L5

racket_r = .25
racket_l1 = 3.2426 * racket_r # r*(3*sqrt(2) - 1)
racket_l2 = 2*np.pi*racket_r
racket_l1_ratio = racket_l1 / (racket_l1 + racket_l2)
racket_u1_coef = racket_l1 / racket_l1_ratio
racket_u2_coef = 2*np.pi / (1 - racket_l1_ratio)
racket_u2_theta_offset = - 3*np.pi/8
racket_u2_x_offset = 3*racket_r

def get(U, mode):
    if mode == 'circle':
        t = 2 * np.pi * U
        return (.5 * (1 + np.cos(t)),
                .5 * (1 + np.sin(t)))
    
    if mode == 'racket':
        if U < racket_l1_ratio:
            l = U * racket_l1 / racket_l1_ratio
            return (l, l)
        else:
            t = (U - racket_l1) * racket_u2_coef + racket_u2_theta_offset 
            return (racket_u2_x_offset + racket_r*np.cos(t), racket_u2_x_offset + racket_r*np.sin(t))
            
    if mode == 'banana':
        if U <= .5 :
            y_coef = 1
            L = 2 * U * banana_l5
        else:
            y_coef = -1
            L = 2 * (1 - U) * banana_l5
        if L < banana_l1:
            theta = np.pi * 0.5 * L / banana_L1
            x = banana_r * np.cos(theta)
            y = banana_r * np.sin(theta)
        elif L < banana_l2:
            x = - (L - banana_l1)
            y = banana_r
        elif L < banana_l3:
            theta = np.pi * (L - banana_l2) / banana_L3 + np.pi * .5;
            x = banana_r_3 * np.cos(theta) - banana_2_r_3
            y = banana_r_3 * np.sin(theta) + banana_2_r_3
        elif L < banana_l4:
            x = -banana_2_r_3 + (L - banana_l3)
            y = banana_r_3
        else:
            theta = np.pi * .5 * (1 - (L - banana_l4) / banana_L5)
            x =  banana_r_3 * np.cos(theta)
            y =  banana_r_3 * np.sin(theta)
        y *= y_coef
        return x + banana_r, y + banana_r
    else:
        print()
        print()
        print(f'Bad mode "{mode}"')
        print()
        print()
        sys.exit(0)
            
