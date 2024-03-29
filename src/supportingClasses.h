#include <set>
#include <list>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <math.h>

using namespace std;

const string DOC_DIR = "../docs";
const string DOC_COUNT_FILE = DOC_DIR + "/docsCount.txt";
const string WORDS_FILE = "../words/words.txt";
const string STOP_WORDS_FILE = "../stopWords/stopWords.txt";
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
		
		void SetWeight(double inverseDocumentFrequency, double maxDocumentFrequency) {
			m_Weight = ((double)m_DocCount * inverseDocumentFrequency) / maxDocumentFrequency;
		}
		
		friend ostream & operator<< (ostream & os, const LemmaElem & lemmaElem) {
			os << " " << lemmaElem.m_Id << "|" << lemmaElem.m_Weight; //for machine reading
			return os;
		}
};
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
bool compareLemmaElemsByWeight(const LemmaElem & A, const LemmaElem & B) {
	return A.m_Weight < B.m_Weight;
}
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
class Lemma {
	public:
		string m_LemmaName;
		mutable list<LemmaElem> m_Docs;
		mutable unsigned m_MaxDocumentFrequency;
		
		//constructor
		Lemma():m_MaxDocumentFrequency(0) { }
		Lemma(string name):m_LemmaName(name), m_MaxDocumentFrequency(0) { }
		Lemma(string name, int docId):m_LemmaName(name), m_MaxDocumentFrequency(0) { m_Docs.push_back(LemmaElem(docId)); }
		
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
		unsigned GetDocumentsCount() const { return m_Docs.size(); }
		
		void SetMaxDocumentFrequency() const { 
			for(auto x : m_Docs) {
				m_MaxDocumentFrequency = max(m_MaxDocumentFrequency, x.m_DocCount);
			}
		}
		
		unsigned GetMaxDocumentFrequency() const { return m_MaxDocumentFrequency; }
		
		//inverse document frequency - idf
		double GetInverseDocumentFrequency(int docCount) const {
			return log2((double)docCount / (double)GetDocumentsCount());
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
		
		//for constructing container from file
		void PushBackDoc(int docId, double weight) const {
			m_Docs.push_back(LemmaElem(docId, weight));
		}
		
		//sorts list by elements's weight
		void SortByWeight() const {
			m_Docs.sort(compareLemmaElemsByWeight);
		}
		
		//returns 0 if doc is not in list
		double GetWeightByDocId(int docId) const {
			for(auto x : m_Docs) {
				if (x.m_Id == docId)
					return x.m_Weight;
			}
			return 0.0;
		}
		
		//prints document ids to std::out, but only max of resCount
		void PrintDocIds(int resCount) const {
			int index = 0, loopMax = min(resCount, (int)m_Docs.size());
			for(auto x : m_Docs) {
				if (index >= loopMax) break;
				cout << x.m_Id << endl;
				index++;
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
		
		LemmaContainer() { }
		
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
		
		//finds lemma by string in container and returns it; if not present, returns empty Lemma
		Lemma FindLemma(string & lemma) {
			auto it = m_Container.find(Lemma(lemma));
			if (it == m_Container.end()) {
				return Lemma(lemma);
			}
			return *it;
		}
		
		//iterate through container and remove stop words (most frequent)
		void RemoveStopWords() {
			ifstream is;
			is.open(STOP_WORDS_FILE);
			if (is.is_open()) {
				string stopWord;
				while(is) {
					is >> stopWord;
					m_Container.erase(Lemma(stopWord)); //erase stop word from container
				}
				is.close();
			}
		}
		
		//iterate through whole container and compute lemma's weight
		void ComputeWeights(int docCount) {
			for(auto &lemma : m_Container) {
				lemma.SetMaxDocumentFrequency(); //first compute max docs frequency for each lemma
				for(auto &lemmaElem : lemma.m_Docs) {
					lemmaElem.SetWeight(lemma.GetInverseDocumentFrequency(docCount), (double)lemma.GetMaxDocumentFrequency());
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
				string line, lemma;
				int docId;
				double weight;
				char delimiter;
				while(getline(is, line)) { //cat 2|0.56 3|0.333 ...
					istringstream iss(line);
					
					iss >> lemma;
					auto it = m_Container.insert(Lemma(lemma));
					while(iss) {
						iss >> docId;
						iss >> delimiter;
						iss >> weight;
						if (iss) (it.first)->PushBackDoc(docId, weight);
					}
				}
				
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

