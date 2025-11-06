
<h1><img src="firmware.elf.png" alt="Logo" width="30"/> Festival Totem Firmware <img src="firmware.elf.png" alt="Logo" width="30"/></h1>

Continue:
https://chatgpt.com/c/68fa7fba-aa5c-8329-8544-3c31ef0e5c5c


TODO for V0.0.1
- If the Strobe button (enc5) is pressed down and the user rotates enc1 the strobe color hue should change. And if enc1 is also pressed down at the same time the strobe color saturation should change.
- If enc4 (intensity) is pressed down special effect 2 should be activated (like a flashbang)
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

| Control | Mode                     | Result                     |
|---------|--------------------------|----------------------------|
| Enc1    | turn                     | Main hue                   |
| Enc1    | hold + turn              | Main saturation            |
| Enc2    | turn                     | Secondary hue              |
| Enc2    | hold + turn              | Secondary saturation       |
| Enc2    | press                    | Toggle secondary ON/OFF    |
| Enc3    | turn                     | Effect switch              |
| Enc4    | turn                     | Intensity                  |
| Enc5    | turn                     | Speed                      |
| Enc5    | press & hold             | Strobe ON instantly        |
| Enc5    | rotate while pressed     | Strobe speed adjust        |
| Pot     | move                     | Brightness                 |

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
