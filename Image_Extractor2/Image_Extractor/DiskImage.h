#pragma once
#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include <vector>
#include <bitset>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

//typedef bitset<8> byte;
typedef unsigned char uchar;

#ifndef DISK_IMAGE_H
#define DISK_IMAGE_H


//This code is for printing the byte as 0x00 (byte format)
struct HexCharStruct{
	uchar c;
	HexCharStruct(uchar _c) : c(_c) { }
};

inline std::ostream& operator<<(std::ostream& o, const HexCharStruct& hs){
	return (o << std::hex << setfill('0') << setw(2) << (int)hs.c);
}

inline HexCharStruct hex(uchar _c){
	return HexCharStruct(_c);
}

//inline function to check if the given file exists
inline bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

//inline function to check if the given folder exists
inline bool folder_exists(const std::string& name) {
	struct stat info;
	if (stat(name.c_str(), &info) != 0) 
		return 0;
	else if (info.st_mode & S_IFDIR) // S_ISDIR() doesn't exist on my windows 
		return 1;
	else 
		return 0;
}

//This file is used to replace a token with another in a string
// used only to sanitize the directory
inline void replaceStringInPlace(std::string& subject, const std::string& search,
	const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}

// This function sanitized the input directory, it makes sure the directory end with '\\'
inline string sanitize_dir(string in_dir) {
	replaceStringInPlace(in_dir, "/", "\\");
	if (in_dir.at(in_dir.length() - 1) != '\\')
		in_dir += "\\";
	return in_dir;
}


// This class is the most important class for the project, all the important operations are done in this class.
// Pseudo code for the project is:
// 1) find the length of the disk image divide into 4KB blocks
// 2) iterate through the blocks
// 3)     find empty block (blocks that has less than 100 not 0x00 bytes)
// 4)     find possible image signatures
// 5) for each image signature found in the disk
// 6)     read it and make sure it is real image
// 7)     if real image
// 8)         write it as img_disk_position.jpg
// 9) for each block which is not empty and no images found in it.
// 10     find possible image fragment that are more than 64KB
// 11)    if a continious byte series doesn't have non-image characteristics
// 12)    write it to the out_folder/frag/ as frag_disk_position.frag

class DiskImage
{
public:
	string infile = "";
	string folder = "";
	uint64_t length = 0;

	DiskImage();
	~DiskImage();

	// main DiskImage folder that runs from beginning to the end
	void run(const string filename, const string out_folder);

	//find the length of the diskimage
	uint64_t  find_length(const string filename);
	
	// This function checks if a block has less than 100  non-zero bytes
	//If yes, the block is marked as 1 in vector<boolean> clusters
	bool is_empty_cluster(const vector<uchar> block_hex);

	//find possible image files 
	// This is going to be looking for all file types whose signature is added in other_sign in KnownSignature class.
	// For now, it is searching for only jpeg
	void find_possible_files(const string filename, vector<bool> &clusters, vector<uint64_t> &tot_img_sign, vector<pair<string, uint64_t>> &other_pos);

	// find_possible_files calls this function and this search for image signatures
	vector<int>  search_signature(const vector<uchar> block_hex);

	// Reads X bytes
	//If the Xbytes is greater that 0, that many bytes is written, otherwise, it reads until one of the condition happens based on where the code is called. 
	// For example, for End of Image, it iterates till 0xFF 0xD9 bytes etc.
	void read_Xbytes(const string filename, uint64_t pos, int Xbytes, vector<uchar>& bytes);

	//This function extracts files
	//For v1.0 it only extracts images
	void extract_files(const string filename, const vector<uint64_t> img_signs, const vector<pair<string, uint64_t>> oth_signs, vector<bool> &clusters);
	
	//This file is called from extract_files function. it extract one image only
	void extract_1image(const string filename, uint64_t beg, vector<bool> &clusters);

	// This is the last step function that searches for all possible jpeg fragments.
	// if byte stream contains only JPEG standard structure, it is written to ./frag/ folder as frag_pos.frag
	bool search_jpeg_frag(const string filename, vector<bool> &clusters);

	//this file analyzes if vector<uchar> bytes can be a jpeg fragment
	void jpeg_byte_analysis(vector<uchar> bytes, uint64_t pos);

	//Prints hex version of bytes vector
	void printHex(const vector<uchar> hex);

	//writes a vector into disk (image or fragment)
	void writeFile(const string img_name, vector<uchar> file);

};


#endif // !IMAGE_H
