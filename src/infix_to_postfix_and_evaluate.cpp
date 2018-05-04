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

#include "FillDocumentTable.h"

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

double opNOT(double weight) {
    return 1 - weight;
}

/*static Lemma opNOT(Lemma lemma) {
        Lemma res(lemma.m_LemmaName);
	
        //lemma.m_Docs ordered by docId by default, so we can use little trick
        for(int i = 1; i <= documentCount; i++) {
                if (i == (lemma.m_Docs.front()).m_Id) {
                        lemma.m_Docs.pop_front(); //push nothing and pop from reference lemma 
                }
                else {
                        res.m_Docs.push_back(LemmaElem(i, computeWeightForNot())); //else add to our res
                }
        }
	
        return res;
}*/

double opAND(double weightA, double weightB) {
    double termCount = 2.0,
            res = 1 - sqrt(((1 - weightA)*(1 - weightA) + (1 - weightB)*(1 - weightB)) / termCount);
    return res;
}

/*static Lemma opAND(Lemma & lemma1, Lemma & lemma2) {
        Lemma res("res");
        auto it1 = lemma1.m_Docs.begin();
        auto it2 = lemma2.m_Docs.begin();
        //lists are ordered by docId
        while(it1 != lemma1.m_Docs.end() && it2 != lemma2.m_Docs.end()) {
                if (it1->m_Id == it2->m_Id) {
                        res.PushBackDoc(it1->m_Id, computeWeightForAnd(it1->m_Weight, it2->m_Weight));
                        it1++;
                        it2++;
                }
                else if (it1->m_Id < it2->m_Id) {
                        it1++;
                }
                else {
                        it2++;
                }
        }
        return res;
}*/

double opOR(double weightA, double weightB) {
    double termCount = 2.0,
            res = sqrt((weightA * weightA + weightB * weightB) / termCount);
    return res;
}

/*static Lemma opOR(Lemma & lemma1, Lemma & lemma2) {
    Lemma res("res");
        auto it1 = lemma1.m_Docs.begin();
        auto it2 = lemma2.m_Docs.begin();
        //lists are ordered by docId
        while(it1 != lemma1.m_Docs.end() || it2 != lemma2.m_Docs.end()) {
                if (it1->m_Id == it2->m_Id) {
                        res.PushBackDoc(it1->m_Id, computeWeightForAnd(it1->m_Weight, it2->m_Weight));
                        it1++;
                        it2++;
                }
                else if (it1->m_Id < it2->m_Id) {
                        res.PushBackDoc(it1->m_Id, computeWeightForAnd(it1->m_Weight, it2->m_Weight));
                        it1++;
                }
                else {
                        res.PushBackDoc(it2->m_Id, computeWeightForAnd(it1->m_Weight, it2->m_Weight));
                        it2++;
                }
        }
        return res;
}*/

/*static Lemma evaluate(string in_postfix, LemmaContainer & lemmaContainer, WordPairContainer & wordPairContainer) {
    stack<Lemma> zasobnik;

    stringstream ss(in_postfix);
    string term;
	
    ss >> term;
    if (isOperator(term)) {
        cerr << "ERROR: prvni term v postfixu je operator!!" << endl;
        exit(1);
    }
        string lemmaString = wordPairContainer.FindLemma(term);
        Lemma lemma = lemmaContainer.FindLemma(term);
    zasobnik.push(lemma);
    while (ss >> term) {
        if (isOperator(term)) {
            //jedná se o operator
            if (term == "NOT") {
                Lemma tmp = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opNOT(tmp));
            } else if (term == "AND") {
                Lemma op1 = zasobnik.top();
                zasobnik.pop();
                Lemma op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opAND(op1, op2));
            } else if (term == "OR") {
                Lemma op1 = zasobnik.top();
                zasobnik.pop();
                Lemma op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opOR(op1, op2));
            }
        } else {
            //jedná se o operand --> pouze pridame do zasobniku
                        string lemmaString = wordPairContainer.FindLemma(term); //returns "" if not successful
                        Lemma lemma = lemmaContainer.FindLemma(lemmaString); //returns empty Lemma (with empty list of ids) if not succesful
            zasobnik.push(lemma);
        }

    }
    return zasobnik.top();
}*/

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

bool orderPairIntDoubleByDouble(const pair<int, double> & A, const pair<int, double> & B) {
    return A.second > B.second;
}

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
    //cout << "**************" << endl;

    //order documents in result by relevancy
    resList.sort(orderPairIntDoubleByDouble);

    //return on std out list of document Ids
    //only docIds plus newline: 2\r\n4\r\n7...
    int index = 0;
    for (auto x : resList) {
        if (index >= resCount) break; //maximum of resCount
        cout << x.first << endl; //for app run
        //cout << x.first << " " << x.second << endl; //for debugging
        index++;
    }

    return 0;
}





