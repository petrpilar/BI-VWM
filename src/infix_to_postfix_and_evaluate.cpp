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

static string opNOT(string term) {
    return "(" + term + "NOT" + ")";
}

static string opAND(string term1, string term2) {
    return "(" + term1 + "AND" + term2 + ")";
}

static string opOR(string term1, string term2) {
    return "(" + term1 + "OR" + term2 + ")";
}

static string evaluate(string in_postfix) {
    stack<string> zasobnik;

    stringstream ss(in_postfix);
    string term;

    ss >> term;
    if (isOperator(term)) {
        cerr << "ERROR: prvni term v postfixu je operator!!" << endl;
        exit(1);
    }
    zasobnik.push(term);
    while (ss >> term) {
        if (isOperator(term)) {
            //jedná se o operator
            if (term == "NOT") {
                string tmp = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opNOT(tmp));
            } else if (term == "AND") {
                string op1 = zasobnik.top();
                zasobnik.pop();
                string op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opAND(op1, op2));
            } else if (term == "OR") {
                string op1 = zasobnik.top();
                zasobnik.pop();
                string op2 = zasobnik.top();
                zasobnik.pop();
                zasobnik.push(opOR(op1, op2));
            }
        } else {
            //jedná se o operand --> pouze pridame do zasobniku
            zasobnik.push(term);
        }

    }
    return zasobnik.top();
}

int main(int argc, char** argv) {
	
	if (argc != 2) {
		cerr << "Argument count != 2." << endl;
		return -1;
	}
	
    //string infix = "kocka AND pes AND ( NOT opice OR krava )";
	string infix(argv[1]);
	
	//init inverted index structure
	LemmaContainer lemmaContainer; //container of inverted indexes
	lemmaContainer.InitContainer(); //get structure from file
	lemmaContainer.PrintContainer(); //for debugging
	
    cout << "INFIX:   " << infix << endl;
    string postfix = InfixToPostfix(infix); //kocka pes AND opice NOT krava OR AND
    cout << "POSTFIX: " << postfix << endl;
    string result = evaluate(postfix); //
    cout << "RESULT:  " << result << endl;

    return 0;
}

