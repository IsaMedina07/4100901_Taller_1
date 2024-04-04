**4100901 - Taller 1 - Embedded Peripherals** 
This repo contains the impleentation of the directionals and stationary system, similar to a Tesla System.

**Pineout and name configuration of each led and button**

° A1 --> left directional button.
° A2 --> right directional button. 
° A3 --> stationary button. 

° D1 --> left directional LED.
° D2 --> right directional LED.
° D4 --> heartbeat light.

**Current functionality**

° Heartbeat (D4 LED) blinks every second. 

° When A1 button is pressed once: left directional (D1 LED) blinks 3 times.

° When A1 button is pressed twice in less than 300ms: left directional (D1 LED) blinks indefinitely.

° If the left light is still blinking and A2 button is pressed, left directional (D1 LED) turns off. 

° When A2 button is pressed once: righT directtional (D2 LED) blinks 3 times. 

° When A2 button is pressed twice in less than 300ms: right directional (D2 LED) blinks indefinitely.

° If the right light is still blinking and A1 button is pressed, right directional (D2 LED) turns off. 

° When A3 button is pressed, both left and right lights, ie, stationary lights, start blinking indefinitely until A3 button is pressed again. 

° If one of the directional lights are blinking and A3 button is pressed, both right and left directionals turns on at time and start blinking, like the usual behavior of the stationary lihts. 

**Team members:**

  Paulina Ruiz Bonilla.
  Maria Isabel Aristizabal Medina. 
