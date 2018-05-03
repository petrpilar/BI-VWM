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
#include <math.h>

using namespace std;

const string DOC_DIR = "../data";
const string DOC_COUNT_FILE = DOC_DIR + "/docsCount.txt";
const string WORDS_FILE = "../words/words.txt";
const string INVERTED_INDEX_FILE = "../inverted_index/inverted_index.txt";

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//opens docsCount.txt and extract total docs count
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
class WordPair { //for example: were -> be, cats -> cat
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
//contains set of WordPairs from words.txt -> word/lemma (cats/cat)
class WordPairContainer {
	public:
		set<WordPair> m_Container;
		
		WordPairContainer() { FillContainer(); }
		
		//fills container from words.txt
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
		
		//finds string word in words.txt (aka our set) and returns its corresponding lemma (cats -> cat)
		string FindLemma(const string & word) const {
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
//element of the lemma list, docId and weight
class LemmaElem {
	public:
		int m_Id;
		double m_Weight;
		unsigned m_DocCount; //frequency of term in one doc
		
		LemmaElem(int id):m_Id(id), m_Weight(0), m_DocCount(1) { }
		LemmaElem(int id, double weight):m_Id(id), m_Weight(weight), m_DocCount(1) { }
		
		void IncrementDocCount() { m_DocCount++; }
		
		void SetWeight(double inverseDocumentFrequency, double maxInverseDocumentFrequency) {
			m_Weight = ((double)m_DocCount * inverseDocumentFrequency) / maxInverseDocumentFrequency;
		}
		
		friend ostream & operator<< (ostream & os, const LemmaElem & lemmaElem) {
			os << " " << lemmaElem.m_Id << "|" << lemmaElem.m_Weight; //for machine reading
			return os;
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Lemma {
	public:
		string m_LemmaName;
		mutable list<LemmaElem> m_Docs;
		
		//constructor
                Lemma(){}
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
		
		//number of documents, where lemma has been found - df, document frequency
		unsigned GetDocumentFrequency() const { return m_Docs.size(); }
		
		//inverse document frequency - idf
		double GetInverseDocumentFrequency(int docCount) const {
			return log2((double)docCount / (double)GetDocumentFrequency());
		}
		
		//adds document id to lemma, only if it does not contain it already
		//has to be const - in order to store this in set, this method must be const, as well as list must be mutable
		void PushBackDoc(int docId) const {
			if ((m_Docs.back()).m_Id != docId) { //only if it is not present in list already (if it is there, it has to be at the end)
				m_Docs.push_back(LemmaElem(docId));
			}
			else {
				(m_Docs.back()).IncrementDocCount(); //else just inc doc count - number of terms in single document
			}
		}
		
		friend ostream & operator<< (ostream & os, const Lemma & lemma) {
			os << lemma.m_LemmaName;
			for(auto x : lemma.m_Docs) {
				//os << " -> [" << x << "]"; //for human reading
				os << x; //for machine reading
			}
			return os;
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//set of Lemmas
class LemmaContainer {
	public:
		set<Lemma> m_Container;
		
		double m_MaxInverseDocumentFrequency; //max idf
		
		LemmaContainer():m_MaxInverseDocumentFrequency(0) { }
		
		//inserts string lemma (cat) and document Id to set
		void Insert(string lemma, int docId) {
			//if it is already in container, then add docId
			//if it is not, then insert it into container with one docId
			pair<set<Lemma>::iterator, bool> ret = m_Container.insert(Lemma(lemma, docId));
			if (ret.second == false) { //insert returns pair; second is false, when item was not inserted, because it is already in
				//only if is not in container
				(ret.first)->PushBackDoc(docId); //add docId to the list
			}
		}
		
		//iterate through whole container and get maximum of inverse document frequencies
		void SetMaxInverseDocumentFrequency(int docCount) {
			for(auto x : m_Container) {
				m_MaxInverseDocumentFrequency = max(m_MaxInverseDocumentFrequency, x.GetInverseDocumentFrequency(docCount));
			}
		}
		
		//iterate through whole container and compute lemma's weight
		void ComputeWeights(int docCount) {
			SetMaxInverseDocumentFrequency(docCount);
			for(auto &lemma : m_Container) {
				double inverseDocumentFrequency = lemma.GetInverseDocumentFrequency(docCount);
				for(auto &lemmaElem : lemma.m_Docs) {
					lemmaElem.SetWeight(inverseDocumentFrequency, m_MaxInverseDocumentFrequency);
				}
			}
		}
		
		void WriteToFile() const {
			ofstream os;	
			os.open(INVERTED_INDEX_FILE);
			if(os.is_open()) {
				for(auto x : m_Container) {
					os << x << endl;
				}
				os.close();
			}
		}
			
		//extract container from file
		void InitContainer() {
			m_Container.clear(); //clear everything
			
			ifstream is;
			is.open(INVERTED_INDEX_FILE);
			if (is.is_open()) {
				
				is.close();
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
//returns true, if it is a-z or A-Z or 0-9, else false
bool isInterestingChar(char c) {
	return ((c >= 97 && c <= 122) || (c >= 65 && c <= 90) || (c >= 48 && c <= 57)) ? true : false;
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//erases not interesting chars from start as well as end of string (from "/[cat." -> "cat") 
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
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//calls function above and transforms string to lower only
void normalizeString(string & s) {
	eraseStrangeCharsFromString(s); //erase beginning and ending strange chars
	transform(s.begin(), s.end(), s.begin(), ::tolower); //set every char to lower version
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

