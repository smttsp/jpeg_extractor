#pragma once
#include <map>
#include <iostream>
using namespace std;
#include <vector>

#ifndef KNOWN_SIGNATURE
#define KNOWN_SIGNATURE

// Known signature class contains only signatures and has a single function to print a signature.
// other_signature is empty for now because in version 1.0, I haven't included any blacklisting with other filetypes
class KnownSignature
{
public:
	KnownSignature();
	~KnownSignature();

	vector<pair<string, vector<uchar>>> pic_sign;
	pair<string, string> other_sign;
	void printSignature(pair<string, vector<uchar>> sign);
private:

};


// Known signatures, all of them are jpeg files
KnownSignature::KnownSignature() {
	uchar v[] = { 0xff,0xd8,0xff,0xe1 };
	uchar v2[] = { 0xff,0xd8,0xff,0xe0 };
	uchar v3[] = { 0xFF, 0xD8, 0xFF, 0xDB };

	vector<uchar> vv(v, std::end(v));
	vector<uchar> vv2(v2, std::end(v2));
	vector<uchar> vv3(v3, std::end(v3));

	pic_sign.push_back(std::make_pair("jpg", vv));
	pic_sign.push_back(std::make_pair("jpg", vv2));
	pic_sign.push_back(std::make_pair("jpg", vv3));
}

KnownSignature::~KnownSignature(){}

// For printing a signature
void KnownSignature::printSignature(pair<string, vector<uchar>> sign) {
	cout << sign.first << " ";
	for (int i = 0; i < sign.second.size(); i++)
		cout << hex <<sign.second.at(i) << " ";
	cout << endl;
}

#endif // !KNOWN_SIGNATURE