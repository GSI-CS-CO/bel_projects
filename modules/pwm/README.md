This module serves as a connector to the included Pulse-Width-Modulation(PWM) module in [general_cores](bel_projects/ip_cores/general-cores/modules/wishbone/wb_simple_pwm).

The following is documentation for the general core module.
### How is the PWM Module configured?
  
There are three values that can be set:
- Counter Period $T_{counter}$
- Prescaler $P$
- Duty Cycle Ratio $D$
  
These are **not** used like in many PWM modules but follow the following rules:  
  
To calculate PWM period $T_{pwm}$ and using the input clocks cycle time $T_{clk}$  
```math
T_{pwm} = (T_{counter} + 1) \cdot (P + 1) \cdot T_{clk}
```
  
    
Assuming $T_{clk} = 16ns, T_{counter} = 4, P = 2$ yields a PWM cycle time $T_{pwm} = 240 ns$.
  
The third writable value, Duty Cycle Ratio $D$, describes the length the PWM signal is high in relation to the whole PWM period. The PWM duty cycle $T_{high}$

```math
T_{high} = D \cdot (P + 1) \cdot T_{clk}
```

  
Assuming $D=3$ and the other values as before yields $T_{high}=96ns$.
  
### How are the values set?

Counter Period and Prescaler values are set via Wishbone input data.  
With a bus width of 32-bit each would have 16-bit values.  
To set the values:  
WB Write to address `t_wishbone_address := x"00000000"` with data `t_wishbone_data := x"00040002"` would configure $T_{counter}$ to `4` and $P$ to $2$

The Duty Cycle Ratio $D$ is also set via Wishbone input data but only uses the higher 16-bit.
To set its value:
	 WB Write to address `t_wishbone_address := x"00000008"` with data `t_wishbone_data := x"DEAD0003"` would configure $D$ to `3`



> [!CAUTION]
> The  PWM duty cycle $T_{high}$ needs to be lower then the whole PWM period $T_{pwm}$!
> Unsound values will be ignored by the module without raising an error.


