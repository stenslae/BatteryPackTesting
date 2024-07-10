#include <iostream>
#include <fstream>
#include "LabJackM.h"
#include "LJM_Utilities.h"
#include <string>
#include <sstream>
#include <windows.h>
#include <cmath>
#include <algorithm> // for remove_if
#include <cctype> // for isspace

using namespace std;

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

class CircuitVariables {
    public: 
	//initialize variables and constructor
        int reads_per_minute;
        double resistance;
        double voltage;
	    double power;
	    double wh;
        double ah;
        double current;
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
            cout << "Reads per Minute: \n30\n";
            reads_per_minute = 30;
	        cout << "Load Resistor ID: ";
	        cin >> id;
            cout << "Resistor Value: ";
            cin >> resistance;
            cout << "Expected Initial Voltage: ";
            cin >> voltage;
            cout << "Name of Battery: ";
            cin >> batt_name;
	        cout << "Date of Test: ";
	        cin >> date_of_test;
        }
};

int main()
{
    CircuitVariables Battery;
    ReadInitialize read_LJM;

    // opens file for writing
    ofstream Voltage_readings("../temp.csv");

    //adds notes for testing info
    Voltage_readings << "Battery Name:," << Battery.batt_name << "\n";
    Voltage_readings << "Resistor ID:," << Battery.id << "\n";
    Voltage_readings << "Date:," << Battery.date_of_test << "\n";
    Voltage_readings << "Load Resistance:," << Battery.resistance << "\n";
    Voltage_readings << "Reads Per Minute:," << Battery.reads_per_minute << "\n";
    Voltage_readings << "Ave Volts:," << "\n" << "Ave Power:," << "\n" << "Ave Current:," << "\n" << "Total Watt-Hours:," << "\n" << "Total mAmp-Hours:," << "\n\n";
    Voltage_readings << "Minutes,Hours,Voltage (V), Current (mA), Power (W), Watt-Hours, mAmp-Hours" << "\n";

    read_LJM.read();

    double init_voltage = Battery.voltage;
    int count = 0;
    int avecount = 0;
    int finalreads = 0;

    // checks to see if the battery is outputing below an arbitrary amount of watts so that recording stops once battery is dead
    while (finalreads < 30) {
        // updates values for data
        read_LJM.read();

        // takes the voltage and power and time
	    Battery.time = (count*(60.0/Battery.reads_per_minute))/60.0;
        Battery.voltage = read_LJM.values[0]-read_LJM.values[1];
        Battery.current = 1000 * Battery.voltage / Battery.resistance;
	    Battery.power = (Battery.voltage*Battery.voltage)/Battery.resistance;

        //sum up the voltage for averaging
        if (Battery.voltage > init_voltage / 10.0) {
            avecount += 1;
            Battery.tot_volts += Battery.voltage;
            Battery.tot_curr += Battery.current;
                Battery.tot_pwr += Battery.power;
        }
        //calculate total watt hours
	    if((count != 0) && (Battery.voltage > init_voltage / 10.0)){
		    Battery.wh = Battery.power*((Battery.time/60)-(((count-1)*(60.0/Battery.reads_per_minute))/60.0/60.0));
            Battery.ah = Battery.current * ((Battery.time / 60) - (((count - 1) * (60.0 / Battery.reads_per_minute)) / 60.0 / 60.0));
	    }else{
		    Battery.wh = 0;
            Battery.ah = 0;
	    }
	    Battery.watt_hours = Battery.watt_hours + Battery.wh;
        Battery.mamp_hours = Battery.mamp_hours + Battery.ah;

        // adds data to data file
        Voltage_readings << Battery.time << "," << Battery.time/60 << "," << Battery.voltage << "," << Battery.current << "," << Battery.power << "," << Battery.wh << "," << Battery.ah << "\n";
        cout << Battery.voltage << "\n";

        if (Battery.voltage < init_voltage / 10.0) {
            finalreads += 1;
        }
	    count += 1;
        // waits until next reading is needed
        Sleep(1000*(60.0/Battery.reads_per_minute));
	
    }

    //Calculate the average voltage
    Battery.tot_volts = Battery.tot_volts / avecount;
    Battery.tot_curr = Battery.tot_curr / avecount;
    Battery.tot_pwr = Battery.tot_curr / avecount;

    // Close the voltage readings file
    Voltage_readings.close();
    
    // Sanitize file name components
    string batt_name_sanitized = Battery.batt_name;
    string date_of_test_sanitized = Battery.date_of_test;
    batt_name_sanitized.erase(remove_if(batt_name_sanitized.begin(), batt_name_sanitized.end(), [](char c) { return isspace(c) || ispunct(c); }), batt_name_sanitized.end());
    date_of_test_sanitized.erase(remove_if(date_of_test_sanitized.begin(), date_of_test_sanitized.end(), [](char c) { return isspace(c) || ispunct(c); }), date_of_test_sanitized.end());

    // Create new file
    ostringstream fileNameStream;
    fileNameStream << "../" << batt_name_sanitized << "_batteryTest_" << date_of_test_sanitized << ".csv";
    string fileName = fileNameStream.str();
    ofstream Final_file(fileName);

    // Open the voltage readings file for reading
    ifstream Voltage_readings_in("../temp.csv");

    //copy the voltage_readings file into the final file, and add the total watt hours
    string line;
    while (getline(Voltage_readings_in, line)){
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
        else{
	        Final_file << line << "\n";
	    }
    }
    
    // closes files and devices
    Voltage_readings.close();
    Final_file.close();
    remove("temp.csv");
    LJM_Close(read_LJM.handle);
    return 0;
}
