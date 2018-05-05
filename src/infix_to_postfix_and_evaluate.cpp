#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <cstdio>
#include <cstring>
#include <climits>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <stack>
using namespace std;

#include "supportingClasses.h"

static int getPriority(string term) {
    if (term == "AND") return 0;
    if (term == "OR") return 0;
    if (term == "NOT") return 1;
    return -1;
}

static bool isOperator(string term) {
    if (term == "AND") return true;
    if (term == "OR") return true;
    if (term == "NOT") return true;
    if (term == "(") return true;
    if (term == ")") return true;
    return false;
}

static string InfixToPostfix(string in_infix) {
    string postfix = "";
    stack<string> zasobnik;

    //dáme in_infix do streamu, ktery nam zajisti cteni po mezerách
    stringstream ss(in_infix);
    string term;
    while (ss >> term) {
        //cout << term << endl; //vypis termu zasebou
        if (!isOperator(term)) {
            //pokud se nejedna o operator
            postfix = postfix + term + " ";
        } else if (term == "(") {
            zasobnik.push("(");
        } else if (term == ")") {
            while (!zasobnik.empty() && zasobnik.top() != "(") {
                postfix = postfix + zasobnik.top() + " ";
                zasobnik.pop();
            }
            if (!zasobnik.empty() && zasobnik.top() != "(") {
                cerr << "ERROR Invalid Expression" << endl;
                exit(1);
            } else {
                zasobnik.pop();
            }
        } else {
            while (!zasobnik.empty() && getPriority(term) <= getPriority(zasobnik.top())) {
                postfix = postfix + zasobnik.top() + " ";
                zasobnik.pop();
            }
            zasobnik.push(term);
        }
    }
    while (!zasobnik.empty()) {
        postfix = postfix + zasobnik.top() + " ";
        zasobnik.pop();
    }
    return postfix;
}

//na zacatku main spocitat
int documentCount = 0;

//**********************************************************************************************INVERTED INDEX**********************************************************************************************
double opNOT(double weight) {
    return 1 - weight;
}

double opAND(double weightA, double weightB) {
    double termCount = 2.0,
            res = 1 - sqrt(((1 - weightA)*(1 - weightA) + (1 - weightB)*(1 - weightB)) / termCount);
    return res;
}

double opOR(double weightA, double weightB) {
    double termCount = 2.0,
            res = sqrt((weightA * weightA + weightB * weightB) / termCount);
    return res;
}

static double evaluate(string in_postfix, int docId, LemmaContainer & lemmaContainer, WordPairContainer & wordPairContainer) {
    stack<double> zasobnik;

    stringstream ss(in_postfix);
    string term;

    ss >> term;
    if (isOperator(term)) {
        cerr << "ERROR: prvni term v postfixu je operator!!" << endl;
        exit(1);
    }
    string lemmaString = wordPairContainer.FindLemma(term);
    Lemma lemma = lemmaContainer.FindLemma(lemmaString);
    zasobnik.push(lemma.GetWeightByDocId(docId));
    while (ss >> term) {
        if (isOperator(term)) {
            //jedná se o operator
            if (term == "NOT") {
                double tmp = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opNOT(tmp));
            } else if (term == "AND") {
                double op1 = zasobnik.top();
                zasobnik.pop();
                double op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opAND(op1, op2));
            } else if (term == "OR") {
                double op1 = zasobnik.top();
                zasobnik.pop();
                double op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opOR(op1, op2));
            }
        } else {
            //jedná se o operand --> pouze pridame do zasobniku
            string lemmaString = wordPairContainer.FindLemma(term); //returns "" if not successful
            Lemma lemma = lemmaContainer.FindLemma(lemmaString); //returns empty Lemma (with empty list of ids) if not succesful
            zasobnik.push(lemma.GetWeightByDocId(docId));
        }
    }
    return zasobnik.top();
}

//**********************************************************************************************SEQUENTIAL**********************************************************************************************
static vector<int> opNOTSequential(vector<int> & ids) {
	vector<int> res;

	//ids ordered by docId by default, so we can use little trick
	unsigned idsIndex = 0;
	for(int i = 1; i <= documentCount; i++) {
		if (idsIndex < ids.size() && i == ids[idsIndex]) {
			idsIndex++; //go to next id
		}
		else {
			res.push_back(i); //else add to our res
		}
	}

	return res;
}

static vector<int> opANDSequential(vector<int> & ids1, vector<int> & ids2) {
	vector<int> res;
	auto it1 = ids1.begin();
	auto it2 = ids2.begin();
	//lists are ordered by docId
	while(it1 != ids1.end() && it2 != ids2.end()) {
		if (*it1 == *it2) {
			res.push_back(*it1);
			it1++;
			it2++;
		}
		else if (*it1 < *it2) {
			it1++;
		}
		else {
			it2++;
		}
	}
	return res;
}

static vector<int> opORSequential(vector<int> & ids1, vector<int> & ids2) {
    vector<int> res;
	auto it1 = ids1.begin();
	auto it2 = ids2.begin();
	//lists are ordered by docId
	while(it1 != ids1.end() && it2 != ids2.end()) {
		if (*it1 == *it2) {
			res.push_back(*it1);
			it1++;
			it2++;
		}
		else if (*it1 < *it2) {
			res.push_back(*it1);
			it1++;
		}
		else {
			res.push_back(*it2);
			it2++;
		}
	}
	return res;
}

//projde celou kolekci dokumentu a vrati vector cisel dokumentu, kde se dane lemmaString nachazi
vector<int> findLemmaInDocuments(string & lemmaString, WordPairContainer & wordPairContainer) {
	vector<int> res;
	string word, lemma;
	for(int i = 1; i <= documentCount; i++) {
		ifstream is;	
		is.open(DOC_DIR + "/doc" + to_string(i) + ".txt");
		if(is.is_open()) {
			while(is >> word) { //read file word by word, operator >> skips white spaces
				normalizeString(word);
				lemma = wordPairContainer.FindLemma(word); //get lemma, returns empty string, if cannot find
				if (lemma == lemmaString) { res.push_back(i); break; } //push back docId and we can finish this document
			}
			is.close(); //in the end of file, close it properly
		}
		else {
			throw "Unable to open file with document: 'doc" + to_string(i) + ".txt'."; //error
		}
	}
	return res;
}

static vector<int> evaluateSequential(string in_postfix, WordPairContainer & wordPairContainer) {
    stack<vector<int> > zasobnik; //vector of ids

    stringstream ss(in_postfix);
    string term;
	
    ss >> term;
    if (isOperator(term)) {
        cerr << "ERROR: prvni term v postfixu je operator!!" << endl;
        exit(1);
    }
	string lemmaString = wordPairContainer.FindLemma(term);
    zasobnik.push(findLemmaInDocuments(lemmaString, wordPairContainer));
    while (ss >> term) {
        if (isOperator(term)) {
            //jedná se o operator
            if (term == "NOT") {
                vector<int> tmp = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opNOTSequential(tmp));
            } else if (term == "AND") {
                vector<int> op1 = zasobnik.top();
                zasobnik.pop();
                vector<int> op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opANDSequential(op1, op2));
            } else if (term == "OR") {
                vector<int> op1 = zasobnik.top();
                zasobnik.pop();
                vector<int> op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opORSequential(op1, op2));
            }
        } else {
            //jedná se o operand --> pouze pridame do zasobniku
			string lemmaString = wordPairContainer.FindLemma(term);
			zasobnik.push(findLemmaInDocuments(lemmaString, wordPairContainer));
        }
    }
    return zasobnik.top();
}

bool orderPairIntDoubleByDouble(const pair<int, double> & A, const pair<int, double> & B) {
    return A.second > B.second;
}

//**********************************************************************************************MAIN**********************************************************************************************
int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Argument count != 3. Usage: exe [resultCount] query." << endl;
        return -1;
    }

    documentCount = docCount();

    //string infix = "kocka AND pes AND ( NOT opice OR krava )";
    int resCount = atoi(argv[1]);
    string infix(argv[2]);

    //init pairs word/term (cats/cat, sang/sing...)
    WordPairContainer wordPairContainer;

    //init inverted index structure
    LemmaContainer lemmaContainer; //container of inverted indexes
    lemmaContainer.InitContainer(); //get structure from file
    //lemmaContainer.PrintContainer(); //for debugging

    //cout << "INFIX:   " << infix << endl;
    string postfix = InfixToPostfix(infix); //kocka pes AND opice NOT krava OR AND
    //cout << "POSTFIX: " << postfix << endl;

    list<pair<int, double> > resList; //pair of docId and relevancy
    double relevancy = 0.0;
    for (int i = 1; i <= documentCount; i++) {
        relevancy = evaluate(postfix, i, lemmaContainer, wordPairContainer); //returns relevancy for each document
        resList.push_back(make_pair(i, relevancy));
        //cout << i << " " << relevancy << endl;
    }

    //order documents in result by relevancy
    resList.sort(orderPairIntDoubleByDouble);

    //return on std out list of document Ids
    //only docIds plus newline: 2\r\n4\r\n7...
	//cout << "INVERTED INDEX" << endl;
    int index = 0;
    for (auto x : resList) {
        if (index >= resCount) break; //maximum of resCount
        cout << x.first << endl; //for app run
        //cout << x.first << " " << x.second << endl; //for debugging
        index++;
    }
	
	//cout << endl << "SEQUENTIAL" << endl;
	//vector<int> resSequential = evaluateSequential(postfix, wordPairContainer);
	//for (unsigned i = 0; i < resSequential.size(); i++) {
    //    if (i >= resCount) break; //maximum of resCount
    //    cout << resSequential.at(i) << endl; //for app run
    //}
	

    return 0;
}





