## Features



## Pin Diagram

### ESP-C3 Super Mini

- GPIO7 <-> MCP23017[SCL]
- GPIO6 <-> MCP23017[SDA]
- GPIO0 <-> B10K (B103) Analog Linear Potentiometer
- GPIO20 <-> WS2812 LED Strip 1 (Data)
- GPIO21 <-> WS2812 LED Strip 2 (Data)

                        ESP-C3 Super Mini
                        ┌──────---──────┐
                    5V  │5V            5│ - GPIO5
                   GND  │GND    Back   6│ - GPIO6 (SDA) ---- MCP23017[SDA]
                   3V3  │3.3           7│ - GPIO7 (SCL) ---- MCP23017[SCL]
                  GPIO4 │4             8│ - GPIO8
                  GPIO3 │3             9│ - GPIO9
                  GPIO2 │2            10│ - GPIO10
                  GPIO1 │1            20│ - GPIO20 (RX) ---- WS2812
 B10K (B103) ---- GPIO0 │0            21│ - GPIO21 (TX) ---- WS2812
                        └───────────────┘


### MCP23017

-
  - AM MCP23017 sind 5 der folgende Encoder angeschlossen:
  - An den Encodern ist jeweils einer der Taster Pins an GND angschlossen, der andere Pin geht jeweils zum MCP23017
  - Der mitllere der 3 rotary pins ist ebenfalls and GND angeschlossen
  - Die beiden rotary pins sind am MCP23017 angeschlossen

                            MCP23017
                        ┌──────---──────┐
                 GPB0 - │1            28│ - GPA7
                 GPB1 - │2            27│ - GPA6
                 GPB2 - │3            26│ - GPA5
                 GPB3 - │4            25│ - GPA4
                 GPB4 - │5            24│ - GPA3
                 GPB5 - │6            23│ - GPA2
                 GPB6 - │7            22│ - GPA1
                 GPB7 - │8            21│ - GPA0  ---- EC11(0)[A]
                 VDD  - │9            20│ - INTA
                 VSS  - │10           19│ - INTB
                 NC   - │11           18│ - RESET ---- 3V3
   ESP[SCL] ---- SCL  - │12           17│ - A2    ---- GND
   ESP[SDA] ---- SDA  - │13           16│ - A1    ---- GND
                 NC   - │14           15│ - A0    ---- GND
                        └───────────────┘

### [EC11 Rotary Encoder](https://arduino-projekte.info/products/ec11-rotary-encoder)

- GND <-> GND
- SW <-> MCP23017[GP..] (use A5/6/7 and B5/6)
- A <-> MCP23017[GPAx] (use A0...4)
- B <-> MCP23017[GPBx] (use B0...4)
- C <-> GND

                              EC11
                        ┌───────────────┐
                 GND  - │GND           A│ - MCP23017[GPAx]
                        │    Bottom    C│ - GND
      MCP23017[GP..]  - │SW            B│ - MCP23017[GPBx]
                        └───────────────┘
