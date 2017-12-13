#pragma once
#include "DiskImage.h"
#include "KnownSignature.h"
#define sz  4096

DiskImage::DiskImage(){

}

DiskImage::~DiskImage(){

}

KnownSignature ks;
int image_count = 0;
int numNZ = 100;

int img_cnt = 0;
int frag_cnt = 0;

// runner file
void DiskImage::run(const string filename, const string out_folder) {
	folder = out_folder;
	vector<uint64_t> tot_img_sign;
	vector<pair<string, uint64_t>>  tot_oth_sign;	//we need to update this one to ;

	//find the length of the disk image
	length = find_length(filename);
	uint64_t num_clus = length / (uint64_t)sz+1;
	cout  << "File size " << (length/1024) << "KB\nThere are " << num_clus << " blocks of 4KB" << endl;
	vector<bool> clusters(num_clus, 0);

	// find possible files based on their signatures
	// for now, we  only jpeg files 
	find_possible_files(filename, clusters, tot_img_sign, tot_oth_sign);

	extract_files(filename, tot_img_sign, tot_oth_sign, clusters);
	search_jpeg_frag(filename, clusters);
	cout << endl;
}


// This iterates tru all blocks and find possible image pieces
void DiskImage::find_possible_files(const string filename, vector<bool> &clusters, vector<uint64_t> &tot_img_sign, vector<pair<string, uint64_t>> &other_pos) {

	uint64_t num_clus = clusters.size();
	uint64_t perc = num_clus / 20;

	for (uint64_t i = 0; i < num_clus; i++) {
		if (i % perc == 0)				
			cout << (i / perc * 5) << "% is completed\r" << std::flush;

		vector<uchar> block_hex;
		uint64_t cnt = sz;
		if (i*sz + sz > length)
			cnt = length - i*(uint64_t)sz;
		read_Xbytes(filename, i*sz, cnt, block_hex);

		if (is_empty_cluster(block_hex)) {
			//cout << i << "_th cluster is empty: " << i*sz << endl;
			clusters[i] = 1;
		}
		else {
			vector<int> imgs = search_signature(block_hex);
			for (int s = 0; s < imgs.size(); s++)
				tot_img_sign.push_back((uint64_t)i*sz + imgs.at(s));
		}

	}
	cout << endl;

}


//This file aims to extract known file types in the disk
//For v1.0 it only extracts the images
void DiskImage::extract_files(const string filename, const vector<uint64_t> img_signs, const vector<pair<string, uint64_t>> oth_signs, vector<bool> &clusters) {

	for (int i = 0; i < img_signs.size(); i++) {
		extract_1image(filename, img_signs.at(i), clusters);
	}

	cout << endl;

	//other signatures
	for (int i = 0; i < oth_signs.size(); i++) {

	}
}


//This function extracts 1 image file at a time if the file is really a jpeg file
// It checks if the file has different image attributes such as header is correct or the length of markers are matching with the next marker.
// For example if the Huffman table is X bytes, the next marker should start after X bytes. 
// If that position is not a marker or start of encoded data, then the byte stream is not an image 
// and I don't extract that byte stream as image
void DiskImage::extract_1image(const string filename, uint64_t beg, vector<bool> &clusters) {
	bool finished = false;
	bool real_img = true;
	vector<uchar> file;
	read_Xbytes(filename, beg, 4, file);
	uint64_t cnt = beg+4;
	while (!finished) {

		//read 2 bytes length
		vector<uchar> s;
		read_Xbytes(filename, cnt, 2, s);
		int mark_len = s[0]; mark_len = (mark_len << 8) | s[1];

		//read length+2 bytes
		vector<uchar> bytes;
		read_Xbytes(filename, cnt, mark_len, bytes);
		//file.reserve(file.size() + bytes.size());
		file.insert(file.end(), bytes.begin(), bytes.end());
		bytes.clear();
		cnt += mark_len;

		//check the next two bytes are finish marker or sth
		read_Xbytes(filename, cnt, 2, s);
		cnt += 2;
		//file.reserve(file.size() + s.size());
		file.insert(file.end(), s.begin(), s.end());
		s.clear();

		//printHex(file);
		
		if (s[0] == 0xff) {
			//stop when Start of Scan
			if (s[1] == 0xda) { //start of scan
		
				read_Xbytes(filename, cnt, -1, bytes);
				cnt += bytes.size();
				//file.reserve(file.size() + bytes.size());
				file.insert(file.end(), bytes.begin(), bytes.end());
				bytes.clear();
		
				finished = true;
			}
			//check if it is a marker
			else if (s[1] < 0xff && s[1] >= 0xc0) {

			}
			else {
				finished = true;
				real_img = false;
				break;
			}
		}
	}
	if (file.size() > 0 && real_img) {
		writeFile(folder + "img"+ to_string(beg)+".jpg", file);
		//cout << "img" << to_string(beg) << ".jpg is extracted" << endl;
		img_cnt++;
		cout << img_cnt << " images are extracted\r" << std::flush;
		if (beg % sz < 100)
			clusters.at((uint64_t)(beg / sz)) = 1;
		for (uint64_t f = (uint64_t)(beg / sz)+1; f < (uint64_t)((beg + file.size()) / sz); f++) {
			clusters.at(f) = 1;
		}
	}
	if (file.size() < 0) {
		// fill whichever clusters are not image
		// because they are unknown type maybe?
	}
}

#define xbyte 1024
void DiskImage::read_Xbytes(const string filename, uint64_t pos, int Xbytes, vector<uchar>& bytes) {

	//cout << "beg: " << dec << Xbytes << endl;
	uchar last_byte = 0;
	ifstream is;
	is.open(filename, ios::binary);
	is.seekg(pos, ios::beg);

	if (Xbytes > 0) {
		bytes.resize(Xbytes);
		is.read((char*)&bytes[0], Xbytes);
	}
	else {

		uint64_t ccc = pos;
		bool stop = false;
		while (true) {

			vector<uchar> buffer(xbyte);
			is.read((char*)&buffer[0], xbyte);

			buffer.insert(buffer.begin(), last_byte);
			int inc = -1;
			bool prev = false;

			for (int i = 0; i < xbyte - 1; i++) {
				if (buffer[i] == 0xff && buffer[i + 1] == 0xd9) {
					inc = i + 1;
					stop = true;
					break;
				}
			}

			if (inc == -1) {
				ccc += xbyte;
				last_byte = buffer.at(xbyte - 1);
				//bytes.reserve(bytes.size() + buffer.size() - 1);
				bytes.insert(bytes.end(), buffer.begin() + 1, buffer.end());
			}
			else {
				ccc += inc;
				//bytes.reserve(bytes.size() + buffer.size() - 1);
				bytes.insert(bytes.end(), buffer.begin() + 1, buffer.begin() + inc + 1);
			}
			if (stop)
				break;
			buffer.clear();
		}
	}
	is.close();

}

vector<int> DiskImage::search_signature(const vector<uchar> block_hex){

	//1) Search for image signatures
	vector<int> img_sign;
	for (int i = 0; i < ks.pic_sign.size(); i++) {

		vector<uchar> cur_sign = ks.pic_sign.at(i).second;
		for (int j = 0; j < block_hex.size() - cur_sign.size() + 1; j++) {

			if (cur_sign.at(0) == block_hex.at(j)){
				//here implement string search
				//make sure I don't exceed the limits
				bool matched = true;
				for (int k = 1; k < cur_sign.size(); k++) {

					if (cur_sign.at(k) == 0x00)
						continue;
					if ((j + k) < block_hex.size() && cur_sign.at(k) != block_hex.at(j + k)) {
						matched = false;
						break;
					}

				}

				if(matched){
					//ks.printSignature(cur_sign);
					img_sign.push_back(j);

				}
			}
		}
	}

	return img_sign;
}


#define min_frag 64*1024 //64 KB
void DiskImage::jpeg_byte_analysis(const vector<uchar> bytes, uint64_t pos) {

	vector<uchar> pos_frag;
	clock_t t2 = clock();
	int beg = 0;

	for (auto i = 0; i < bytes.size(); i++) {

		if (bytes.at(i) == 0x00)
			beg++;
		else 
			break;

	}

	for (auto i = beg; i < bytes.size()-1; i++) {

		if (bytes.at(i) == 0xFF) {

			// if FF is not followed by 00 or D1-7 in encoded data, this is not a jpeg file
			if (bytes.at(i + 1) != 0x00 && !(bytes.at(i + 1) > 0xD0 && bytes.at(i + 1) < 0xD8)) {
				//&& bytes.at(i + 1) != 0xFE --> this is comment
				if (i - beg > min_frag) {
					vector<uchar> newVec(bytes.begin() + beg, bytes.begin()+i);
					writeFile(folder + "frag\\frag" + to_string(beg) + ".frag", newVec);
					//cout << "frag" << to_string(beg) << ".frag with size:" << (newVec.size() / 1024) << "KB is created\n";
					frag_cnt++;
					cout << frag_cnt << " possible fragments are extracted\r" << std::flush;

				}
				beg = i+1;
			}
		}
	}
	auto fin = 0;
	for (auto i = bytes.size() - 1; i > 1; i--) {
		if (bytes.at(i) == 0x00)
			fin++;
		else
			break;
	}

	if (bytes.size() - beg -fin > min_frag) {
		clock_t t1 = clock();
		vector<uchar> newVec(bytes.begin()+beg, bytes.end()-fin);
		writeFile(folder + "frag\\frag" + to_string(pos+beg) + ".frag", newVec);
		frag_cnt++;
		cout << frag_cnt << " possible fragments are extracted\r" << std::flush;
	}
}

//I will write this later on as last step
bool DiskImage::search_jpeg_frag(const string filename, vector<bool> &clusters) {
	for (auto i = 0; i < clusters.size(); i++) {
		if (clusters.at(i) == 0) {
			//check if it is
			auto j = i + 1;
			while ( j < clusters.size() && clusters.at(j) == 0)
				j++;
			vector<uchar> bytes;
			read_Xbytes(filename, i*sz, (j-i)*sz, bytes);
			if(bytes.size()>min_frag)
				jpeg_byte_analysis(bytes, i*sz);
			i = j;
		}
	}
	return false;
}

bool DiskImage::is_empty_cluster(const vector<uchar> block_hex){

	int cnt = 0;
	for (int i = 0; i < block_hex.size(); i++){
		if (block_hex.at(i) != 0x00)
			cnt++;
		if (cnt > numNZ) {
			return false;
		}
	}
	return true;
}

uint64_t DiskImage::find_length(const string filename) {
	std::streamoff length = 0;
	std::ifstream is(filename, std::ios::binary);

	length = is.tellg();
	is.seekg(0, std::ios::end);
	length = is.tellg() - length;
	is.close();

	return (uint64_t)length;
}

void DiskImage::printHex(vector<uchar> binaries){
	for (int i = 0; i < binaries.size(); i++)
		cout << hex(binaries[i]) << " ";
	cout << endl;
}

void DiskImage::writeFile(const string img_name, vector<uchar> file){
	ofstream fout(img_name, ios::out | ios::binary);
	fout.write((char*) &file[0], file.size() * sizeof(file[0]));
	fout.close();
}

