# jpeg_extractor

This project extracts jpeg images and fragments (without header) from a given disk. 

Image_Extractor_v1.0
This code is implemented by Samet Taspinar
10 Dec 2016

arg[1]: path/to/file from which images will be extracted
arg[2]: output folder. This one is optional, if the user doesn't input it, a folder, output_folder, 
will be created and images that are found will be pushed to that folder. 
A sub-folder, frag will be created under output folder to which image fragments will be saved.

Note that there are Windows dependencies so the code works in Windows only.
In the future versions, 
		- Windows dependencies will be eliminated and the code will be platform independent
		- Multi-threading will be added 
		- Various different type of data can be extracted
		- This will help decrease the false positive rate for the fragments
		- More vulnerability check, memory and CPU usage analysis (Performance Profiling)
		- Further experiments with different types of disk images will be done
		- Different JPEG images might have different type of information, include various types of jpeg files
