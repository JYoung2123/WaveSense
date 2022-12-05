// AntNeuro Software.cpp : Defines the entry point for the application.
//

#define EEGO_SDK_BIND_DYNAMIC
#include <factory.h>
#include "wrapper.cc"

#include <chrono>
#include <thread>
#include <array>
#include <iostream> // console io
#include <conio.h> // For _kbhit()

#include <lsl_cpp.h>
#include <time.h>
#include <stdlib.h>
using namespace std;


const char* channels[] = { "C3","C4","Cz","FPz","POz","CPz","O1","O2" };

int main(int argc, char *argv[])
{
	// Open impedance stream
	using namespace eemagine::sdk;
	factory fact("eego-SDK.dll");;
	amplifier* amp = fact.getAmplifier(); // Get an amplifier
	stream* impStream = amp->OpenImpedanceStream();

	// Open LSL stream
	std::string name, type;
	if (argc < 3) {
		std::cout
			<< "This opens a stream under some user-defined name and with a user-defined content "
			"type."
			<< std::endl;
		//std::cout << "SendData Name Type [n_channels=8] [srate=100] [max_buffered=360]"
		//	<< std::endl;
		std::cout
			<< "Please enter the stream name and the stream type"
			<< std::endl;
		std::cin >> name >> type;
		std::cout << name;
		std::cout << type;
	}
	else {
		name = argv[1];
		type = argv[2];
	}
	int n_channels = argc > 3 ? std::stol(argv[3]) : 8;
	n_channels = n_channels < 8 ? 8 : n_channels;
	n_channels = 19;
	int samplingrate = argc > 4 ? std::stol(argv[4]) : 1; //set sampling rate (Hz)
	int max_buffered = argc > 5 ? std::stol(argv[5]) : 360;
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
		//amp->StartTriggerOut(amp->getChannelList());

		buffer buf = impStream->getData(); // Retrieve data from stream
		//std::cout << "Samples read: " << buf.getSampleCount() << std::endl;
		//std::cout << "Channel count: " << buf.getChannelCount() << std::endl;
		for (int c = 0; c < n_channels; c++) {
			sample[c] = buf.getSample(c, 0);
			cout << "Channel " << c << ": " << sample[c] << endl;
		}
		outlet.push_sample(sample);
		//std::cout << "Impedance: " << buf.getSample(0,0) << std::endl;
		//std::cout << buf.getChannelCount() << std::endl;
		//std::cout << buf.data() << std::endl;
		//std::cout << amp->getSamplingRatesAvailable() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		// Need to sleep less than 1s otherwise data may be lost
		if (_kbhit()) { //check for key press
			ch = getch();
			if (ch == 'i')
				break;
		}
	}
	cout << "Impedance data stream stopped, starting voltage data stream." << endl;

	delete impStream;

	stream* eegStream = amp->OpenEegStream(1000,1,2.5);
	samplingrate = 250;

	while (true) // Loop forever until 'v' key gets pressed
	{
		buffer buf = eegStream->getData(); // Retrieve data from stream
		for (int c = 0; c < n_channels; c++) {
			sample[c] = buf.getSample(c, 0);
			cout << "Channel " << c << ": " << sample[c] << endl;
		}
		outlet.push_sample(sample);
		std::this_thread::sleep_for(std::chrono::milliseconds(25));
		// Need to sleep less than 1s otherwise data may be lost

		if (_kbhit()) { // check for key press
			ch = getch();
			if (ch == 'v')
				break;
		}
	}

	cout << "Voltage data stream stopped." << endl;

	delete eegStream;
	delete amp; // Make sure to delete the amplifier objects to release resources
	return 0;
	

	
}



