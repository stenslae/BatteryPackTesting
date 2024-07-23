#include <iostream>
#include <fstream>
#include "LabJackM.h"
#include "LJM_Utilities.h"
#include <string>
#include <sstream>
#include <windows.h>

using namespace std;

// Stores variables
class Variables {
    public:
        //Constants:
        double resistance;
        string batt_name;
        string date_of_test;
        string id;
        int reads_per_minute = 60;
        int divisor = 60;
        int handle;
        double init_voltage;
        //Iterator:
        int count = 0;
        int avecount = 0;
        int finalreads = 0;
        double oldtime = 0;
        //Data variables:
        double hours_elapsed = 0;
        double mamp_hours = 0;
	    double watt_hours=0;
        double tot_volts = 0;
        double tot_pwr = 0;
        double tot_curr  = 0;

        Variables() {
            // Input values
            cout << "Welcome to the battery test. Your battery should have the positive and negative terminal connected to a load resistor.\nThe positive terminal should then be wired to AIN0, while the negative terminal is wired to GND.\nLoad Resistor ID: ";
	        cin >> id;
            cout << "Resistor Value: ";
            cin >> resistance;
            cout << "Expected Initial Voltage: ";
            cin >> init_voltage;
            cout << "Name of Battery (no whitespace or special symbols): ";
            cin >> batt_name;
	        cout << "Date of Test (no whitespace or special symbols): ";
	        cin >> date_of_test;
        }
};

int main(){
    // Declare/Initialize variables and open LabJack
    long long start, end;
    double time, mins, hrs, voltage, current, power, wh=0 , ah=0;
    Variables Battery;
    LJM_Open(LJM_dtT7, LJM_ctUSB, LJM_idANY, &Battery.handle);
    start = LJM_GetHostTick();

    // Opens file for writing
    ofstream Voltage_readings("../temp.csv");
    // Adds summary of tests
    Voltage_readings << "Battery Name:," << Battery.batt_name << "\n";
    Voltage_readings << "Resistor ID:," << Battery.id << "\n";
    Voltage_readings << "Date:," << Battery.date_of_test << "\n";
    Voltage_readings << "Load Resistance:," << Battery.resistance << "\n";
    Voltage_readings << "Reads Per Minute:," << Battery.reads_per_minute << "\n";
    Voltage_readings << "Hours Elapsed:," << "\n" << "Ave Volts:," << "\n" << "Ave Power:," << "\n" << "Ave Current:," << "\n" << "Total Watt-Hours:," << "\n" << "Total mAmp-Hours:," << "\n\n";
    Voltage_readings << "Time,Minutes,Hours,Voltage (V), Current (mA), Power (W), Watt-Hours, mAmp-Hours" << "\n";

    // Initialize interval loop
    const int INTERVAL_HANDLE = 1;
    LJM_StartInterval(INTERVAL_HANDLE, (60/Battery.reads_per_minute)*1000000);

    // Iterates while battery is still providing power
    while (Battery.finalreads <= 60) {

        // Reads the voltage
        LJM_eReadName(Battery.handle, "AIN0", &voltage);
        // Read the data and the time data is read at
        end = LJM_GetHostTick();
        time = (double)(end - start) / 1000000;

        // Calculations based off of readings
        mins = time / 60.0;
        hrs = mins / 60.0;
        current = 1000 * voltage / Battery.resistance;
        power = (voltage*voltage) / Battery.resistance;

        // If the battery is providing power
        if (voltage > Battery.init_voltage/10.0){
            // Sum up the values for averaging
            Battery.avecount += 1;
            Battery.tot_volts += voltage;
            Battery.tot_curr += current;
            Battery.tot_pwr += power;
            // Calculate watt-hours and mamp-hours
            wh = power * (hrs - Battery.oldtime);
            ah = current * (hrs - Battery.oldtime);
            Battery.watt_hours += wh;
            Battery.mamp_hours += ah;
            // Record the hours that the batter is powering for
            Battery.hours_elapsed = hrs;
            // Save the time of this read for calculations on the next read
            Battery.oldtime = hrs;
        }
        else {
            // Increase end case
            Battery.finalreads += 1;
        }

        // Every minute adds a line of data to the data file
        cout << voltage << "\n";
        if (Battery.count % Battery.divisor == 0) {
            Voltage_readings << time << "," << mins << "," << hrs << "," << voltage << "," << current << "," << power << "," << wh << "," << ah << "\n";
        }

        // Record the amount of reads taken
	    Battery.count += 1;
        // Waits until next interval
        int skip;
        LJM_WaitForNextInterval(INTERVAL_HANDLE, &skip);
    }

    // Calculate the average voltage
    Battery.tot_volts = Battery.tot_volts / Battery.avecount;
    Battery.tot_curr = Battery.tot_curr / Battery.avecount;
    Battery.tot_pwr = Battery.tot_pwr / Battery.avecount;

    // Close the voltage readings file
    Voltage_readings.close();

    // Create new file
    ostringstream fileNameStream;
    fileNameStream << "../" << Battery.batt_name << "_batteryTest_" << Battery.date_of_test << ".csv";
    string fileName = fileNameStream.str();
    ofstream Final_file(fileName);

    // Open the voltage readings file for reading
    ifstream Voltage_readings_in("../temp.csv");

    // Copies the file's data and adds any final testing resutls to the top of the final csv file
    string line;
    while(getline(Voltage_readings_in, line)){
    	if(line.find("Total Watt-Hours:,") != string::npos){
	        Final_file << "Total Watt-Hours:," << Battery.watt_hours << "\n";
	    }
        else if (line.find("Total mAmp-Hours:,") != string::npos) {
            Final_file << "Total mAmp-Hours:," << Battery.mamp_hours << "\n";
        }
        else if (line.find("Ave Volts:,") != string::npos) {
            Final_file << "Ave Volts:," << Battery.tot_volts << "\n";
        }
        else if (line.find("Ave Current:,") != string::npos) {
            Final_file << "Ave Current:," << Battery.tot_curr << "\n";
        }
        else if (line.find("Ave Power:,") != string::npos) {
            Final_file << "Ave Power:," << Battery.tot_pwr << "\n";
        }
        else if (line.find("Hours Elapsed:,") != string::npos) {
            Final_file << "Hours Elapsed:," << Battery.hours_elapsed << "\n";
        }
        else{
	        Final_file << line << "\n";
	    }
    }
    
    // Closes files and devices
    LJM_CleanInterval(INTERVAL_HANDLE);
    Voltage_readings.close();
    Final_file.close();
    remove("temp.csv");
    LJM_Close(Battery.handle);
    return 0;
}
