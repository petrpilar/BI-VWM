#include <set>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

#include "FillDocumentTable.h"

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//cl FilLDocumentTable.cpp
//FilLDocumentTable.exe
int main() {
	LemmaContainer lemmaContainer;
	WordPairContainer wordPairContainer;
	
	string word, lemma;
	int fileCount = docCount();
	for(int i = 1; i <= fileCount; i++) { //first doc is "doc1.txt"
		ifstream is;	
		is.open(DOC_DIR + "/doc" + to_string(i) + ".txt");
		if(is.is_open()) {
			while(is >> word) { //read file word by word, operator >> skips white spaces
				//find word in vector of lemms, if it is not there, then add it, else just add Elem to its list
				//add to its list this id of file, and weight 0
				normalizeString(word);
				lemma = wordPairContainer.FindLemma(word); //get lemma, returns empty string, if cannot find
				if (lemma.empty() == false) lemmaContainer.Insert(lemma, i); //and add it to container
			}
			is.close(); //in the end of file, close it properly
		}
		else {
			throw "Unable to open file with document: 'doc" + to_string(i) + ".txt'."; //error
		}
	}
	
	lemmaContainer.PrintContainer();
	
	//to the file
	
	//system("pause");
}


