#include <fstream>
#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include "arclib.h"	
#include "ethernet.h"

using namespace std;

// Initialize ethernet controller interface object
int DiscoveryNetDevice(CArclib& ar, const char *ip)
{
	ARC_STATUS status;

	EthernetInterface *ei = new EthernetInterface();
	if ( !ei->init(ip) )
	{
		return 0;
	}

	status = ar.ArcInitSession(ei);
	if ( status != ARC_SUCCESS )
	{
		delete ei;
		return 0;
	}

	return 1;
}

// Generate encryption key for volume
ARC_STATUS GenerateKey(CArclib& ar, const char *output_filename)
{
	ARC_STATUS status;
	unsigned char buf[4096];
	DWORD bufsize = sizeof(buf);
	sGUI_VOLUMESET volset = {};


	// For debugging, check to see if volume is set for encryption, and if it already has key loaded
	status = ar.ArcGetVolSetInfo(0, &volset);
	if (status != ARC_SUCCESS)
	{
		cerr << "Failed to ArcGetVolSetInfo(...), Error Code = " << status << endl;
		return status;
	}

	cout << "Key type: ";
	
	switch (volset.gvsAesKeyType) {
		case 0: cout << "AES 256-bit key, password"; break;
		case 1: cout << "AES 256-bit key"; break;
		case 2: cout << "AES 128-bit key, password"; break;
		case 3: cout << "AES 128-bit key"; break;
		default: cout << "unknown value " << (int)volset.gvsAesKeyType; break;
	}
	
	cout << endl << "Key assigned: " << (volset.gvsAesKeyAssigned ? "Yes" : "No") << endl;

	if (volset.gvsVolumeStatus & (0x11)) {
		cerr << "Volume not intialized, aborting." << endl
			<< "In experiments, this state results in a negative-length memcpy that segfaults the API." << endl;
		return 1;
	}

	// generate key for volume 0 (USB enclosures only support one LUN over USB, so multi-volume support
	// is a little bit pointless.  PCI-E controllers have Cli64 so this utility is not needed)
	BYTE volsetArray[1] = {0};
	status = ar.ArcGenerateAESKeyBuffer(1, volsetArray, &bufsize, (unsigned char *)buf);
	//status = ar.ArcGenerateAESKeyBuffer(0, NULL, &bufsize, buf);
	if (status != ARC_SUCCESS) {
		cerr << "Failed to ArcGenerateAESKeyBuffer(...), Error Code = " << status << endl;
		return status;
	}

	ofstream outfile(output_filename, ios::binary | ios::openmode(0666) | ios::trunc);
	if (!outfile) {
		perror((string("Error opening file ") + output_filename).c_str());
		return -1;
	}

	outfile.write((char *)buf, bufsize);
	if (!outfile) {
		perror("Error writing output");
		return -1;
	}
	outfile.close();
	cout << "Success!" << endl << endl
		<< "If you lose " << output_filename << " your data is gone with no possibility of recovery."
	    << endl << endl
		<< "Guard the file " << output_filename << " with your life, make backups, " << endl
		<< "turn it into a QR Code in a safe deposit box, add it to your password manager, etc." << endl;

	cout << "After the array has been powered off, you will need to upload this file through the webUI " << endl
		<< "before accessing your data." << endl;

	return ARC_SUCCESS;
}

CArclib ar = {};

int main(int argc, const char* argv[])
{
	ARC_STATUS status;
	int ctrl = 0;

	po::options_description desc("Allowed options");
	desc.add_options()
		("help", "produce help message")
		("ip", po::value<string>()->required(), "IP address of controller")
		("output", po::value<string>()->default_value("outkey"), "output filename")
		("password", po::value<string>()->default_value("0000"), "controller admin password")
	;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);

	if (vm.count("help")) {
		cout << desc << endl;
		return 1;
	}

	try {
		po::notify(vm);
	} catch (po::required_option e) {
		cerr << e.what() << endl;
		return 1;
	}

	const string& ip = vm["ip"].as<string>();
	ctrl = DiscoveryNetDevice(ar, ip.c_str());
	if (ctrl < 1) {
		cerr << "Failed to find ethernet controller on IP " << ip << endl;
		return 1;
	}

	// login to API
	status = ar.ArcChkPassword((char *)vm["password"].as<string>().c_str());
	if ( status != ARC_SUCCESS )
	{
		cerr << "Failed to ArcChkPassword(...), Error Code = " << status << endl;
		return status;
	}

	status = GenerateKey(ar, vm["output"].as<string>().c_str());
	ar.ArcLogOut();

	return status;
}
