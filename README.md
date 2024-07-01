Modified code from Josh Kempe.
Takes readings of the voltage over a battery pack over time to determine the total watt-hours of the battery pack. Uses a LabJack T7.

Reads onto a CSV file for further analysis. To run, you must have CMake and VS code for C++. Open the CMake file in VS and Press Build<Build All. Wait for the build to complete then run battery.exe. 

Expected Connections Are As Follows:

AIN0 --> - Battery Terminal/Resistor Connection

AIN1 --> + Battery Terminal/Resistor Connection

(Battery is wired in series with the resistor, and the voltage is read in parralel.)
