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
	    double watt_hours=0;
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
    Voltage_readings << "Resistance:," << Battery.resistance << "\n";
    Voltage_readings << "Total Watt-Hours:," << "\n\n";
    Voltage_readings << "Minutes,Hours,Voltage (V), Power (W), Watt-Hours" << "\n";

    read_LJM.read();

    double init_voltage = Battery.voltage;
    int count = 0;

    // checks to see if the battery is outputing below an arbitrary amount of watts so that recording stops once battery is dead
    while (Battery.voltage > init_voltage/10.0)
    {
        // updates values for data
        read_LJM.read();

        // takes the voltage and power and time
	    Battery.time = (count*(60.0/Battery.reads_per_minute))/60.0;
        Battery.voltage = read_LJM.values[0]-read_LJM.values[1];
	    Battery.power = (Battery.voltage*Battery.voltage)/Battery.resistance;	
	    //calculate total watt hours
	    if((count != 0) && (Battery.voltage > init_voltage / 10.0)){
		    Battery.wh = Battery.power*((Battery.time/60)-(((count-1)*(60.0/Battery.reads_per_minute))/60.0/60.0));
	    }else{
		    Battery.wh = 0;
	    }
	    Battery.watt_hours = Battery.watt_hours + Battery.wh;

        // adds data to data file
        Voltage_readings << Battery.time << "," << Battery.time/60 << "," << Battery.voltage << "," << Battery.power << "," << Battery.wh << "\n";
        cout << Battery.voltage << "\n";
	
	    count += 1;
        // waits until next reading is needed
        Sleep(1000*(60.0/Battery.reads_per_minute));
	
    }

    // Close the voltage readings file
    Voltage_readings.close();
    
    // Sanitize file name components
    string batt_name_sanitized = Battery.batt_name;
    string date_of_test_sanitized = Battery.date_of_test;
    batt_name_sanitized.erase(remove_if(batt_name_sanitized.begin(), batt_name_sanitized.end(), [](char c) { return isspace(c) || ispunct(c); }), batt_name_sanitized.end());
    date_of_test_sanitized.erase(remove_if(date_of_test_sanitized.begin(), date_of_test_sanitized.end(), [](char c) { return isspace(c) || ispunct(c); }), date_of_test_sanitized.end());

    // Create new file
    ostringstream fileNameStream;
    fileNameStream << "../" << batt_name_sanitized << "_test_" << date_of_test_sanitized << ".csv";
    string fileName = fileNameStream.str();
    ofstream Final_file(fileName);

    // Open the voltage readings file for reading
    ifstream Voltage_readings_in("../temp.csv");

    //copy the voltage_readings file into the final file, and add the total watt hours
    string line;
    while (getline(Voltage_readings_in, line)){
    	if(line.find("Total Watt-Hours:,") != string::npos){
	        Final_file << "Total Watt-Hours:," << Battery.watt_hours << "\n";
	    }else{
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
