//Image_Extractor_v1.0
//This code is implemented by Samet Taspinar
// 10 Dec 2016
// arg[1]: path/to/file from which images will be extracted
// arg[2]: output folder. This one is optional, if the user doesn't input it, a folder, output_folder, 
// will be created and images that are found will be pushed to that folder. 
// A sub-folder, frag will be created under output folder to which image fragments will be saved.

// Note that there are Windows dependencies so the code works in Windows only.
// In the future versions, 
//		- Windows dependencies will be eliminated and the code will be platform independent
//		- Multi-threading will be added 
//		- Various different type of data can be extracted
//			- This will help decrease the false positive rate for the fragments
//		- More vulnerability check, memory and CPU usage analysis (Performance Profiling)
//		- Further experiments with different types of disk images will be done
//		- Different JPEG images might have different type of information, include various types of jpeg files

#include <iostream>
#include <string>
using namespace std;

#include "DiskImage.h"
#include <direct.h>
int main(int argc, char** argv ) {
	clock_t start = clock();

	string disk_img = "";
	string out_folder = "";

	// read the input arguments
	if (argc >= 3) {
		disk_img = argv[1];
		out_folder = argv[2];
		out_folder = sanitize_dir(out_folder);
		_mkdir(out_folder.c_str());
		_mkdir((out_folder + "\\frag\\").c_str());
	}
	else if (argc >= 2) {
		disk_img = argv[1];
		out_folder = ".\\out_folder\\";
		_mkdir(out_folder.c_str());
		_mkdir((out_folder+"\\frag\\").c_str());
	}
	
	//sanitize output folder
	//out_folder = sanitize_dir(out_folder);

	//check if file and output folder exist 
	if (!file_exists(disk_img) || !folder_exists(out_folder)) {
		cout << "Usage is: \"path/to/disk_image\" output_folder  OR \n"
			<< "Usage is: \"path/to/disk_image\"";
		cout << "\nPress any key to end the program\n";
		getchar();
		return 0;

	}

	// runner function of diskImage
	DiskImage di;
	di.run(disk_img, out_folder);
	
	cout << "\nTotal time: "<< clock() - start << "msec "<< endl;
	cout << "Press any key to end the program\n";
	getchar();

}
