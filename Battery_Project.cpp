#include <iostream>
#include <fstream>
#include "LabJackM.h"
#include "LJM_Utilities.h"
#include <string>
#include <sstream>
#include <windows.h>
#include <cmath>
#include <chrono>
#include <ctime> 

using namespace std;

// Opens LabJack and takes readings of AIN1 and AIN0
class ReadInitialize {
    public:
	//initalize variables and constructor
        int LJMError;
        const char * channels[2] = {"AIN0", "AIN1"};
        int handle;
        int LJMERROR_Val = 0;
        int num_frames = 2;
        double values[2];

        ReadInitialize() {
            // opens handle when object from class is created
            LJMError = LJM_Open(LJM_dtT7, LJM_ctUSB, "470015117", &handle);
        }
        int read() {
            // updates values
            LJMError = LJM_eReadNames(handle, num_frames, channels, values, &LJMERROR_Val);
            return LJMError;
        }
};

// Stores calculations of data
class CircuitVariables {
    public: 
	//initialize variables and constructor
        int reads_per_minute = 60;
        double resistance;
        double voltage;
	    double power;
	    double wh;
        double ah;
        double current;
        double hours_elapsed = 0;
        double mamp_hours = 0;
	    double watt_hours=0;
        double tot_volts = 0;
        double tot_pwr = 0;
        double tot_curr  = 0;
        string batt_name;
	    string date_of_test;
	    string id;
	    double time=0;

        CircuitVariables() {
            //input values
	        cout << "Load Resistor ID: ";
	        cin >> id;
            cout << "Resistor Value: ";
            cin >> resistance;
            cout << "Expected Initial Voltage: ";
            cin >> voltage;
            cout << "Name of Battery (no whitespace or special symbols): ";
            cin >> batt_name;
	        cout << "Date of Test (no whitespace or special symbols): ";
	        cin >> date_of_test;
        }
};

int main(){
    // Create instances of classes
    CircuitVariables Battery;
    ReadInitialize read_LJM;

    // Opens file for writing
    ofstream Voltage_readings("../temp.csv");

    // Adds summary of tests
    Voltage_readings << "Battery Name:," << Battery.batt_name << "\n";
    Voltage_readings << "Resistor ID:," << Battery.id << "\n";
    Voltage_readings << "Date:," << Battery.date_of_test << "\n";
    Voltage_readings << "Load Resistance:," << Battery.resistance << "\n";
    Voltage_readings << "Reads Per Minute:," << Battery.reads_per_minute << "\n";
    Voltage_readings << "Hours Elapsed:," << "\n" << "Ave Volts:," << "\n" << "Ave Power:," << "\n" << "Ave Current:," << "\n" << "Total Watt-Hours:," << "\n" << "Total mAmp-Hours:," << "\n\n";
    Voltage_readings << "Time,Minutes,Hours,AIN0,AIN1,Voltage (V), Current (mA), Power (W), Watt-Hours, mAmp-Hours" << "\n";\

    // Initialize variables
    double init_voltage = Battery.voltage;
    int count = 0;
    int avecount = 0;
    int finalreads = 0;
    int divisor = 60;
    long long start = LJM_GetHostTick();

    // Initialize interval loop
    const int INTERVAL_HANDLE = 1;
    LJM_StartInterval(INTERVAL_HANDLE, (60/Battery.reads_per_minute)*1000000);

    // Checks to see if the battery has died and records a few of the dead values
    while (finalreads <= 60) {
        // Read the data and the time data is read at
        read_LJM.read();
        long long end = LJM_GetHostTick();
        double time = (double)(end - start) / 1000000;

        // Takes the voltage and power and time
        Battery.time = time / 60.0;
        Battery.voltage = read_LJM.values[0] - read_LJM.values[1];
        Battery.current = 1000 * Battery.voltage / Battery.resistance;
        Battery.power = (Battery.voltage * Battery.voltage) / Battery.resistance;

        // Sum up the voltage for averaging
        if (Battery.voltage > init_voltage / 10.0) {
            avecount += 1;
            Battery.tot_volts += Battery.voltage;
            Battery.tot_curr += Battery.current;
            Battery.tot_pwr += Battery.power;
        }

        // Calculate total watt hours and mamp hours
        if ((count != 0) && (Battery.voltage > init_voltage/10.0)) {
            Battery.wh = Battery.power * ((Battery.time / 60) - (((count - 1) * (60.0 / Battery.reads_per_minute)) / 60.0 / 60.0));
            Battery.ah = Battery.current * ((Battery.time / 60) - (((count - 1) * (60.0 / Battery.reads_per_minute)) / 60.0 / 60.0));
        }
        else {
            Battery.wh = 0;
            Battery.ah = 0;
        }
        Battery.watt_hours = Battery.watt_hours + Battery.wh;
        Battery.mamp_hours = Battery.mamp_hours + Battery.ah;

        // Every minute adds a line of data to the data file
        cout << Battery.voltage << "\n";
        if (count % divisor == 0) {
            Voltage_readings << time << "," << Battery.time << "," << Battery.time / 60 << "," << "," << read_LJM.values[0] << "," << "," << read_LJM.values[1] << "," << Battery.voltage << "," << Battery.current << "," << Battery.power << "," << Battery.wh << "," << Battery.ah << "\n";
        }

        // End case
        if (Battery.voltage < init_voltage / 10.0) {
            finalreads += 1;
        }
        else {
            Battery.hours_elapsed = time/60/60;
        }

        // Record the amount of reads taken
	    count += 1;

        // Waits until next interval
        int skip;
        LJM_WaitForNextInterval(INTERVAL_HANDLE, &skip);
    }

    // Calculate the average voltage
    Battery.tot_volts = Battery.tot_volts / avecount;
    Battery.tot_curr = Battery.tot_curr / avecount;
    Battery.tot_pwr = Battery.tot_pwr / avecount;

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
    LJM_Close(read_LJM.handle);
    return 0;
}
