
<h1><img src="firmware.elf.png" alt="Logo" width="30"/> Festival Totem Firmware <img src="firmware.elf.png" alt="Logo" width="30"/></h1>

Continue:
https://chatgpt.com/c/68fa7fba-aa5c-8329-8544-3c31ef0e5c5c


TODO for V0.0.1
-
...

## Special Effects

enc5 (Press down): Strobe
enc4 (Press down): Energy Buildup Flash
enc3 (Press down): [Open Slot]
enc2 No special effect possible
enc1 No special effect possible

Normal Effects:
- Solid Color (Two colors static)
- Ambulance Light (Blue/Red Flash)


## Controls

### Control Behavior Matrix

Each mode (Default + 3 Special Effects) has its own independent config that can be adjusted and persisted.

| Encoder | Default Mode | Special1 (Strobe) | Special2 (Energy) | Special3 (Emergency) |
|---------|-------------|-------------------|-------------------|---------------------|
| **Enc1** turn | Main hue | **Strobe hue** | Main hue | Main hue |
| **Enc1** hold+turn | Main sat | **Strobe sat** | Main sat | Main sat |
| **Enc2** turn | Secondary hue | Secondary hue | Secondary hue | Secondary hue |
| **Enc2** hold+turn | Secondary sat | Secondary sat | Secondary sat | Secondary sat |
| **Enc2** press | **Toggle secondary** | *Disabled* | *Disabled* | *Disabled* |
| **Enc3** turn | **Effect switch** | *Disabled* | *Disabled* | *Disabled* |
| **Enc3** hold | - | - | - | **Activate Special3** |
| **Enc4** turn | Intensity | Intensity | **Energy level** | Intensity |
| **Enc4** press | - | - | **Toggle state** | - |
| **Enc5** turn | Speed | **Strobe speed** | Speed | **Emergency speed** |
| **Enc5** hold | **Activate Special1** | - | - | - |
| **Pot** move | Brightness (global) | Brightness | Brightness | Brightness |

**Special Combo:**
- **Enc4 + Enc5** held together for **5 seconds** → Save all configs to flash (green strobe feedback)

### Special Effect Details

**Special1 - Strobe (Enc5 hold):**
- Adjustable color strobe (default white)
- Release Enc5 → return to Default mode
- Adjusted parameters persist to Special1 config

**Special2 - Energy Burst (Enc4 press):**
- **State 1:** Press Enc4 → Enter BuildingUp mode
  - Spinning point rises with intensity (0=bottom, 255=top)
  - Pixels below = 50% brightness, above = off
- **State 2:** Press Enc4 again
  - If intensity ≤ 95% → Stop, return to Default
  - If intensity > 95% → Explode (2s fast main LED alternating), then auto-return

**Special3 - Emergency Lights (Enc3 hold):**
- Blue/red rotating pattern on hanging LEDs
- Main LEDs smoothly fade blue ↔ red
- Speed adjustable
- Release Enc3 → return to Default mode

Interaction feels snappy + musical 🎶

## Hardware

- ESP32-WROOM-32D N4
- Outputs: 2x WS2812 LED strips
- Inputs:
  - 5x [EC11 Rotary Encoders](https://www.amazon.de/WayinTop-Potentiometer-Drehwinkelgeber-Automobilelektronik-Multimedia-Audio/dp/B08728PS6N) (with push button)
  - 1x [Linear Potentiometer](https://www.amazon.de/Schiebepotentiometer-Zweikanaliger-gerader-Schiebemischer-mehrere/dp/B09PBXB47T/ref=sr_1_33?__mk_de_DE=%C3%85M%C3%85%C5%BD%C3%95%C3%91&sr=8-33) B10k (B103)

### PCB Layout

```
┌─────────────┐
│ Poti   Enc1 │
│┌───┐  ┌───┐ │
││ │ │  | ◯ | │
││ │ │  └───┘ │
││ │ |   Enc2 │
││ │ │  ┌───┐ │
││ │ │  | ◯ | │
│| │ |  └───┘ │
││ │ |   Enc3 │
││ │ │  ┌───┐ │
││ │ │  | ◯ | │
│└───┘  └───┘ │
│ Enc5   Enc4 │
│┌───┐  ┌───┐ │
││ ◯ │  | ◯ | │
│└───┘  └───┘ │
└─────────────┘
```

### ESP32-WROOM-32D N4

- GPIO7 <-> MCP23017[SCL]
- GPIO6 <-> MCP23017[SDA]
- GPIO0 <-> B10K (B103) Analog Linear Potentiometer
- GPIO20 <-> WS2812 LED Strip 1 (Data)
- GPIO21 <-> WS2812 LED Strip 2 (Data)

```
                         ESP32-WROOM-32D N4
                        ┌───────---───────┐
                  3V3 - │3V3            VN│ -
                  GND - │GND    Top    GND│ -
                        │D15           D13│ - Enc 3 A
                        │D2            D12│ -
                        │D4            D14│ - Enc 3 B
              Enc 2 A - │D16           D27│ - LED 1 DATA
              Enc 2 B - │D17           D26│ - LED 2 DATA
                        │D5            D25│ - Enc 5 B
              Enc 4 A - │D18           D33│
              Enc 4 B - |D19           D32│
              Enc 1 A - |D21           D35│
                        |RX0           D34│
                        |TX0            VN│
              Enc 1 B - |D22            VP│ - Poti B10K (B103)
              Enc 5 A - |D23            EN│
                        └─────────────────┘
```

### EC11 Rotary Encoder

- GND <-> GND
- SW <-> ESP GPIO?
- A <-> ESP GPIO?
- B <-> ESP GPIO?
- C <-> GND

```
                              EC11
                        ┌───────────────┐
                 GND  - │GND           A│ - ESP GPIO
                        │               │
                        │    Bottom    C│ - GND
                        │               │
            ESP GPIO  - │SW            B│ - ESP GPIO
                        └───────────────┘

```



Backlog:
- Add small speaker for audio feedback that matches the effects.
- [Auto Hupe für den Krankenwagen Blaulicht Effekt](https://www.youtube.com/watch?v=Dqc6yRIHiW0)

<img src="firmware.elf.png" alt="Logo" width="400"/>

Greetings from Firmware.elf
