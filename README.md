# Battery Pack Testing

## Project Description
This project measures the energy capacity of battery packs by sampling the voltage across a load resistor over time. The total watt-hours (Wh) are calculated to evaluate battery performance. Data is collected using a LabJack T7 and analyzed through custom-developed software.

---

## Development

### 🔍 Lead Error Analysis
When performing battery tests, lead wires (positive and negative) connect the battery to a known resistor. Voltage is measured across the resistor to calculate energy output. However, some energy is dissipated in the lead wires themselves, introducing a slight measurement error.

- **Wire Specs**: 18-gauge copper, 15 inches each (30 inches total).
- **Expected Resistance (25–65°C)**: ~16.275–18.775 mΩ.
- **Measured Example**:
  - Lead voltage drop: 14.77 mV  
  - Load voltage: 5.11109 V  
  - Load resistor: 5.13 Ω  
  - Current: ≈ 0.996 A  
  - Lead resistance: ≈ 14.824 mΩ (single wire), ≈ 29.6 mΩ (both wires)

**Findings**:
- Measured resistance was ~1.82× higher than theoretical—likely due to alligator clip connections.
- This introduces a **0.58% error** in energy measurements, which is considered minor for most practical tests.

---

### 🧪 Resistor Error Analysis
- **Long-Term Testing**: Resistors were tested daily over several months; no degradation was found.
  ![Resistance Vs Time](/Images/ResistanceVsTime_Resistors.PNG)

- **Thermal Behavior**:
  - Resistance increases with temperature (linear trend).
  - Power increases cause temperature increases, which in turn raise resistance.
  ![Resistance vs Temp / Temp vs Power](/Images/TemperatureVsPower_Resistors.PNG)

- **Thermal Performance**:
  - Resistor temperatures were logged under >5W loads.
  ![Temp vs Time](Images/TemparatureVsTime_Batteries.PNG)

---

## 🧑‍💻 Code Development
Custom C++ code was developed to:
- Perform continuous voltage reads at a set interval.
- Save readings to a `.csv` file.
- Automatically stop and compute total watt-hours when voltage falls below a threshold (battery drained).

---

## 🧪 Testing Procedure

1. **Hardware Setup**:
   - Connect the **LabJack T7** to your computer via USB.
   - Connect the battery to a **known resistor** using **18-gauge wire**.
   - Wire as follows:
     - Negative side of resistor → GND
     - Positive side of resistor → AIN0

2. **Software Setup**:
   - Download and unzip the `BatteryPackTesting` project folder.
   - Open **VS Code** and select:
     ```
     File → Open → CMakeLists.txt (in BatteryPackTesting folder)
     ```
   - Build the project:
     ```
     Build → Build All
     ```
   - Press the green **Run** arrow labeled `battery.exe`.

3. **Running the Test**:
   - Follow the on-screen prompts to input test details.
   - Connect the battery when prompted. The program will begin printing voltage readings (~5V typical).
   - When the battery is fully discharged, the test ends automatically and a `.csv` is generated.

4. **Data Upload & Review**:
   - Upload the `.csv` file to the shared Google Drive folder (organized by battery pack).
   - Open the file with **Google Sheets**:
     ```
     Right-click → Open With → Google Sheets
     ```
   - Review and save the following values:
     - Average Voltage
     - Total Watt-Hours (Wh)
     - Total Milliamp-Hours (mAh)

---

## 📊 Battery Pack Results
Across multiple tests, actual Wh output was consistently **higher than advertised** values—indicating conservative manufacturer estimates or better-than-expected performance.

---

## 📝 Acknowledgments
- Developed as part of a project at **LabJack Corporation**
- Additional contributions by **Josh Kempe** (LabJack Intern)
- **Libraries & Tools**:
  - LabJack T7
  - LJM Library
  - 200W Load Resistors
  - C++ / CMake
  - Visual Studio Code
