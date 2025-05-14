# Plotter Control GUI (Processing + Arduino)

This project is a graphical interface built using the Processing framework to control a 2D drawing plotter via an Arduino board.

## Features

- Select serial port to connect to Arduino.
- Load and stream a custom G-code-like command file.
- Manual control buttons (up, down, left, right).
- Pen control buttons (Pen Up / Pen Down).
- Reset position to origin.
- Progress bar for file execution.
- Visual status of current port, command, and position.

## Requirements

- [Processing IDE](https://processing.org/download/)
- [Arduino IDE](https://www.arduino.cc/en/software)
- Arduino connected to stepper motors or servos and programmed to handle the following commands:
  - `LINE X Y`
  - `PENUP`
  - `PENDOWN`
  - `RESET`
  - Must reply with `ok` after each command

## How to Use

1. Upload the corresponding Arduino code to your board.
2. Run this sketch in Processing.
3. Click **"Select Port"** to choose the correct serial port.
4. Click **"Load File"** to load your command file.
5. Click **"Start Streaming"** to begin automated drawing.
6. Use manual controls to test or fine-tune the plotter.

## Command File Format

Each line in the file should be a simple instruction, such as:

```
PENDOWN
LINE 50 50
LINE 100 100
PENUP
RESET
```

Lines starting with `;` are treated as comments and skipped.

## Notes

- Movement is constrained within a 300x300 grid.
- Arrow buttons move the pen by 5 mm steps.
- Designed for use with Arduino serial communication at 9600 baud rate.

## License

MIT License
