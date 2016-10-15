#ifndef TABLEHEADER_H
#define TABLEHEADER_H

#include <vector>
#include <utility>
#include <string>
#include <iostream>

using namespace std;

typedef pair<string,int> PAIR;
typedef vector< PAIR > PAIRVECTOR;

class TableHeader{
	PAIRVECTOR thVector;
public:
	TableHeader(PAIRVECTOR rawVector){
		PAIR ridPair("#rid",4);
		thVector.push_back(ridPair);
		PAIRVECTOR::iterator pvi;
		for(pvi = rawVector.begin();pvi!=rawVector.end();pvi++)
			thVector.push_back(*pvi);
	}
	PAIR operator[](const int index){
		return thVector[index];
	}
	int getSize(){
		return thVector.size();
	}
};

#endif
// vim: ts=4
