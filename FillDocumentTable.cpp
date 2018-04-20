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

const string DOC_DIR = "data";
const string DOC_COUNT_FILE = DOC_DIR + "/docsCount.txt";
const string WORDS_FILE = "words/words.txt";

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
int docCount() {
	int docCount = 0;
	ifstream is;	
	is.open(DOC_COUNT_FILE);
	if(is.is_open()) {
		is >> docCount; //believing that there is integer and nothing else
		is.close();
	}	
	return docCount;
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WordPair { //for example: were -> be
	public:
		string m_Word;
		string m_Lemma;
		
		WordPair(string word):m_Word(word), m_Lemma("") { } //dummy constructor just for finding by word part
		WordPair(string word, string lemma):m_Word(word), m_Lemma(lemma) { }
		
		bool operator== (const WordPair & wordPair) const { 
			return m_Word == wordPair.m_Word;
		}
		bool operator!= (const WordPair & wordPair) const {
			return !(*this == wordPair);
		}
		bool operator< (const WordPair & wordPair) const {
			return m_Word < wordPair.m_Word;
		}
		
		friend ostream & operator<< (ostream & os, const WordPair & wordPair) {
			return os << wordPair.m_Word << " " << wordPair.m_Lemma;
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class WordPairContainer {
	public:
		set<WordPair> m_Container;
		
		WordPairContainer() { FillContainer(); }
		
		void FillContainer() {
			ifstream is;
			is.open(WORDS_FILE);
			string line, word, lemma;
			char delimiter = ' ';
			if (is.is_open()) {
				while(getline(is, line)) { //loop through every pair in file
					istringstream iss(line);
					getline(iss, word, delimiter); //get word
					getline(iss, lemma, delimiter); //get lemma
					m_Container.insert(WordPair(word, lemma)); //insert into container
				}
				is.close();
			}
			else {
				throw "Unable to open '" + WORDS_FILE + "'."; //error
			}
		}
		
		string FindLemma(const string & word) const {
			//cout <<"test" <<endl;
			auto it = m_Container.find(WordPair(word));
			return (it != m_Container.end()) ? it->m_Lemma : "";
		}
		
		void PrintContainer() const { //just for debugging
			for(auto x : m_Container) {
				cout << x << endl;
			}
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class LemmaElem {
	public:
		int m_Id;
		double m_Weight;
		
		LemmaElem(int id):m_Id(id), m_Weight(0) { }
		LemmaElem(int id, double weight):m_Id(id), m_Weight(weight) { }
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Lemma {
	public:
		string m_LemmaName;
		mutable list<LemmaElem> m_Docs;
		
		Lemma(string name, int docId):m_LemmaName(name) { m_Docs.push_back(LemmaElem(docId)); }
		
		bool operator== (const Lemma & lemma) const { 
			return m_LemmaName == lemma.m_LemmaName;
		}
		bool operator!= (const Lemma & lemma) const {
			return !(*this == lemma);
		}
		bool operator< (const Lemma & lemma) const {
			return m_LemmaName < lemma.m_LemmaName;
		}
		
		//has to be const - in order to store this in set, this method must be const, as well as list must be mutable
		void PushBackDoc(int docId) const {
			if ((m_Docs.back()).m_Id != docId) { //only if it is not present in list already (if it is there, it has to be at the end)
				m_Docs.push_back(LemmaElem(docId));
			}			
		}
		
		friend ostream & operator<< (ostream & os, const Lemma & lemma) {
			os << lemma.m_LemmaName;
			for(auto x : lemma.m_Docs) {
				//os << " -> [" << x.m_Id << "|" << x.m_Weight << "]"; //for human reading
				os << " " << x.m_Id << "|" << x.m_Weight; //for machine reading
			}
			return os;
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class LemmaContainer {
	public:
		set<Lemma> m_Container;
		
		LemmaContainer() { }
		
		void Insert(string lemma, int docId) {
			//if it is already in container, then add docId
			//if it is not, then insert it into container with one docId
			pair<set<Lemma>::iterator, bool> ret = m_Container.insert(Lemma(lemma, docId));
			if (ret.second == false) { //insert returns pair; second is false, when item was not inserted, because it is already in
				//only if is not in container
				(ret.first)->PushBackDoc(docId); //add docId to the list
			}
		}
		
		void PrintContainer() const { //just for debugging
			for(auto x : m_Container) {
				cout << x << endl;
			}
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool isInterestingChar(char c) {
	//a-z, A-Z, 0-9
	return ((c >= 97 && c <= 122) || (c >= 65 && c <= 90) || (c >= 48 && c <= 57)) ? true : false;
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
void eraseStrangeCharsFromString(string & s) {
	int i = 0;
	while(i < s.size() != 0 && !isInterestingChar(s[i])) { //erase strange chars from start
		i++;
	}
	s = s.erase(0, i);
	i = s.size() - 1;
	while(i >= 0 && !isInterestingChar(s[i])) { //erase strange chars from end
		i--;
	}
	s = s.erase(i + 1, s.size());
}

void normalizeString(string & s) {
	eraseStrangeCharsFromString(s); //erase beginning and ending strange chars
	transform(s.begin(), s.end(), s.begin(), ::tolower); //set every char to lower version
}
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
				lemma = wordPairContainer.FindLemma(word); //get lemma, returns nullptr, if cannot find
				if (lemma.empty() == false) lemmaContainer.Insert(lemma, i); //and add it to container
			}
			is.close(); //in the end of file, close it properly
		}
		else {
			throw "Unable to open file with document: 'doc" + to_string(i) + ".txt'."; //error
		}
	}
	
	lemmaContainer.PrintContainer();
	
	//system("pause");
}


