// WaveSense Recorder.cpp : Defines the entry point for the application.
//

#define EEGO_SDK_BIND_DYNAMIC
#include <factory.h>
#include "wrapper.cc"

#include <chrono>
#include <thread>
#include <array>
#include <iostream> // console io
#include <fstream> // for file saving
#include <conio.h> // For _kbhit()

#include <lsl_cpp.h>
#include <time.h>
#include <stdlib.h>
using namespace std;


int main(int argc, char *argv[])
{
	// Open impedance stream
	using namespace eemagine::sdk;
	factory fact("eego-SDK.dll");;
	amplifier* amp = fact.getAmplifier(); // Get an amplifier
	stream* impStream = amp->OpenImpedanceStream();

	// Prompt user input for settings
	std::string name, type, input;
	string iFileName, vFileName;
	ofstream iFile;
	ofstream vFile;
	if (argc < 3) {
		std::cout
			<< "This opens a stream under some user-defined name and with a user-defined content "
			"type."
			<< std::endl;
		std::cout
			<< "Please enter the stream name and the stream type"
			<< std::endl;
		std::cin >> name >> type;
		std::cout << "Name: " << name << "\t Type: " << type << endl;
		cout << "Enter Y/N for local storage of recorded data: ";
		cin >> input;
		while (!(input == "Y" || input == "N")) {
			cout << "Must enter Y or N: ";
			cin >> input;
		}
	}
	else {
		name = argv[1];
		type = argv[2];
	}

	// Set impedance file name
	if (input == "Y") {
		cout << "Enter file name for impedance data: \n"; // be sure to include extension too
		cin >> iFileName;
		iFile.open(iFileName); //open file to start writing impedance data
	}

	int n_channels = argc > 3 ? std::stol(argv[3]) : 8;
	n_channels = n_channels < 8 ? 8 : n_channels;
	n_channels = 19;
	int samplingrate = argc > 4 ? std::stol(argv[4]) : 1; //set sampling rate (Hz)
	int max_buffered = argc > 5 ? std::stol(argv[5]) : 360;

	// Open LSL stream
	lsl::stream_info info(
		name, type, n_channels, samplingrate, lsl::cf_float32, std::string(name) += type);

	// make a new outlet
	lsl::stream_outlet outlet(info, 0, max_buffered);
	std::vector<float> sample(n_channels, 0.0);
	auto t_start = std::chrono::high_resolution_clock::now();
	auto next_sample_time = t_start;
	std::cout << "Now sending impedance data... " << std::endl;
	double starttime = ((double)clock()) / CLOCKS_PER_SEC;
	unsigned t = 0;

	char ch;

	while (true) // Loop forever until 'i' key gets pressed
	{
		buffer buf = impStream->getData(); // Retrieve data from stream
		for (int c = 0; c < n_channels; c++) {
			sample[c] = buf.getSample(c, 0);
			cout << "Channel " << c+1 << ": " << sample[c] << endl;
			if (input == "Y") {
				iFile << c + 1 << "," << sample[c] << "\n";
			}
		}
		outlet.push_sample(sample);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		// Need to sleep less than 1s otherwise data may be lost
		if (_kbhit()) { //check for key press
			ch = getch();
			if (ch == 'i') {
				iFile.close();
				break;
			}
		}
	}
	cout << "Impedance data stream stopped, starting voltage data stream." << endl;
	if (input == "Y") {
		cout << "Recorded impedance data saved in: " << iFileName << endl;
	}

	delete impStream;

	// Set voltage file name
	if (input == "Y") {
		cout << "Enter file name for voltage data: \n"; // be sure to include extension too
		cin >> vFileName;
		vFile.open(vFileName); //open file to start writing voltage data
	}

	stream* eegStream = amp->OpenEegStream(1000,1,2.5);

	while (true) // Loop forever until 'v' key gets pressed
	{
		buffer buf = eegStream->getData(); // Retrieve data from stream
		for (int c = 0; c < n_channels; c++) {
			sample[c] = buf.getSample(c, 0);
			cout << "Channel " << c+1 << ": " << sample[c] << endl;
			if (input == "Y") {
				vFile << c + 1 << "," << sample[c] << "\n";
			}
		}
		outlet.push_sample(sample);
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		// Need to sleep less than 1s otherwise data may be lost

		if (_kbhit()) { // check for key press
			ch = getch();
			if (ch == 'v') {
				vFile.close();
				break;
			}
		}
	}

	cout << "Voltage data stream stopped." << endl;
	if (input == "Y") {
		cout << "Recorded voltage data saved in: " << vFileName << endl;
	}

	delete eegStream;
	delete amp; // Make sure to delete the amplifier objects to release resources
	return 0;
	

	
}



