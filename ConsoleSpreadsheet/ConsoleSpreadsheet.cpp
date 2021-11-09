#include <iostream>
#include <fstream> 
#include <string> 
#include <vector>     // Primary tool to store data from spreadsheet
#include "consolespreadsheet.h"

/*
    For this program, a user will enter spreadsheet data into a text file (default spreadsheet.txt) with cells separated by the delimiter \t, and an empty cell
    will be represented by \t. In any cell, a formula can be inserted in the form "=[cell identifier] | [operator][cell identifier]", where cell identifier is in
    the form [string][integer] where [string] corresponds to the column of the spreadsheet (A = 1, B = 2, ... , AB = 28, etc) and [integer] corresponds to the row
    of the spreadsheet. The program will then take that formula and replace it with the desired value (so =A1+B2 will be replaced with the contents of cell A1 + the
    contents of cell B2. If any cell identifier references an invalid cell (either an empty cell or a cell that is out of range of the spreadsheet), it will replace
    that cell with #NAN (not a number). If there is a cell that references itself (either directly or through other cells), it will be replaced with #ERROR.
    Following that, any cell that attempts to reference a cell with #NAN or #ERROR will be replaced with those messages respectively (where #NAN takes precidence).
    The spreadsheet with all integer values and #NAN or #ERROR messages will then be output to the desired text file (set to a default value of output.txt).

*/

int main() {

    string inputFileName = "spreadsheet.txt";
    string outputFileName = "output.txt";


    /* Parses the input file and scans line by line, adding each line to a vector as a string */
    vector<string> rows = getDataFromSpreadsheet(inputFileName);

    /* At this point, each element in the vector rows is just a single string containing the entire line. This line needs to be separated into its cells */

    vector<vector<string>> sheet = separateRows(rows);

    /* Now, the vector of vectors sheet contains the entire spreadsheet properly indexed - sheet[0][0] contains all data within cell 1 (row 1 column 1) as a single string
       Additionally, the way spreadsheet.txt may have been formatted would leave some rows longer than others. separateRows fills in space at the end of a row with tab stops (delimiter)
       so that each row is the same size (makes it much easier to find if a call to sheet is within range or not */

    convertFormulasToIntegers(sheet);

    /* The sheet now contains only integer values in place of formulas, with #NAN and #ERROR properly placed if a reference to an empty or out of range cell was called (#NAN) or
        there was a self reference (#ERROR)*/

    outputToFile(sheet, outputFileName);

    /* The new sheet with all integer values is now in output.txt */

    return 0;
}

vector<string> getDataFromSpreadsheet(const string& fileName) {
    ifstream file;
    file.open(fileName);

    /* Opens the input file stream to be able to read input.txt */

    vector<string> rows;
    string row;
    while (getline(file, row)) {
        rows.push_back(row);

        /* Adds each row into the vector rows */
    }

    file.close();

    return rows;
}

vector<vector<string>> separateRows(const vector<string>& rows) {
    vector<vector<string>> separatedRows;
    int maxSize = 0;

    /*maxSize is needed for the purpose of filling in empty cells at the end of a row so that the spreadsheet is a perfect rectangle - each column has the same amount of rows*/

    for (register int i = 0; i < rows.size(); i++) {
        string row = rows[i];
        vector<string> tokens = separateRowIntoCells(row);

        /* On any given iteration of this for loop, tokens now contains every cell within a row, separated by cell */

        if (tokens.size() > maxSize) maxSize = tokens.size();

        separatedRows.push_back(tokens);
    }

    /*After the loop, separatedRows contains all cells that were present in input.txt */

    for (int i = 0; i < separatedRows.size(); i++) {
        while (separatedRows[i].size() < maxSize) separatedRows[i].push_back("\t");
    }

    /*Properly formatted separatedRows, each column has equal rows */

    return separatedRows;
}

vector<string> separateRowIntoCells(const string& s) {
    vector<string> cells;

    /*Format for the rows can be a little tricky - something like 3\t4\t5 will be {3, 4, 5}, but \t3\t4\t\t5 will be {\t, 3, 4, \t, 5} */

    if (containsDelimiter(s)) {
        if (s[0] == delimiter) {
            /* If the first character is the delimeter, add an empty cell to cells. */
            cells.push_back(string(1, delimiter));
            vector<string> nextTokens = separateRowIntoCells(s.substr(1, s.length() - 1)); // tokenizes the rest of the string
            cells.insert(cells.end(), nextTokens.begin(), nextTokens.end());   // appends the next tokens to vector<string> tokens
        }
        else {
            string cell;
            for (int i = 0; s[i] != delimiter; i++) {
                cell += s[i];
            }
            cells.push_back(cell);
            vector<string> nextTokens = separateRowIntoCells(s.substr(cell.length() + 1, s.length() - cell.length()));
            cells.insert(cells.end(), nextTokens.begin(), nextTokens.end());

            /* If the first character is not a delimiter, will not be an empty cell. Iterate through the row until a delimiter is reached, add that string to a new cell
                and continue separating the row (not including that next delimiter) */
        }

    }

    /* If there are no more delimiters left in the row, can just add the remaining string to the cells. */

    else if (s.length() != 0) {
        cells.push_back(s);
    }

    return cells;
}

void convertFormulasToIntegers(vector<vector<string>>& sheet) {
    for (int i = 0; i < sheet.size(); i++) {
        for (int j = 0; j < sheet[i].size(); j++) {
            if (sheet[i][j][0] == '=') {

                /* Idenifies if the value within the cell was a formula (began with a '=') or just an integer. If it was an integer, it can remain unchanged. However, if it was a formula,
                * convertFormula must evaluate the formula to an integer. */

                string convertedFormula = convertFormula(sheet, sheet[i][j], i, j);
                sheet[i][j] = convertedFormula;
            }
        }
    }
}

string convertFormula(const vector<vector<string>>& sheet, string s, const int& currentRow, const int& currentCol) {

    /* Takes the formula and formats it properly - "A3    + B1" is now "A3+B1" */
    removeSpaces(s);

    vector<string> cellIdentifiers = getIdentifiers(s);
    vector<char> operators = getOperators(s);

    /* Important: going forward, a cell identifier is a value in the form A1 (string integer) that references a specific cell. Operators are simply all of the
        mathematical operators used in calculations. So, cellIdentifiers and operators contain the ordered list of all of the identifiers and operators used in any given formula.

        Example: "=A3+B1*C3
        cellIdentifiers = {A3, B1, C3}
        operators = {'+', '*'}

        */


        /*Check if all of the identifiers in the vector<string> point to valid cells (nonempty, within range). If an identifier points to an invalid cell, that entire formula will be replaced
        with "#NAN" */

    if (!validIdentifiers(cellIdentifiers, sheet)) return "#NAN";

    /* Similarly, check if cells circularly reference themselves. To avoid an infinite loop, replace the formula with "#ERROR" */

    if (!isNumber(s)) { // An integer value cannot be self referencial, do not need to check this case.
        if (circularReference(cellIdentifiers, sheet, currentRow, currentCol)) return "#ERROR";

    }

    /*Base case for recursive function - if cellIdentifiers only contains one value (either an integer or a single reference to another cell. */

    if (cellIdentifiers.size() == 1) {
        int row = getRow(cellIdentifiers[0]);
        int col = getColumn(cellIdentifiers[0]);

        if (sheet[row][col][0] == '=') {
            return convertFormula(sheet, sheet[row][col], row, col);
        }

        /* If the cell contains a simple formula (=A1), exits the function with the value at the referenced cell. Otherwise, simply returns the integer value at the cell. */

        return sheet[row][col];
    }

    /* For any given formula, only two operations can be done at the same time. So, this function must be recursively called, performing one operation each time */

    while (cellIdentifiers.size() > 1) {

        /* If there is a multiplication or division operator in the formula, do those first */

        int i = indexOfMultDiv(operators);
        if (i >= 0) {
            string firstCell = cellIdentifiers[i];
            string secondCell = cellIdentifiers[i + 1];

            /* cellIdentifiers[i] and [i+1] refer to the cell identifiers that must be multiplied or divided. Because i is the index of the '*' or '/' sign in the operators vector,
                must either multiply or divide the correct elements by one another*/

            if (!isNumber(firstCell)) {

                int column = getColumn(firstCell);
                int row = getRow(firstCell);
                firstCell = sheet[row][column];

                if (firstCell[0] == '=') firstCell = convertFormula(sheet, firstCell, row, column);

                /* firstCell references another cell. If that new cell is itself a formula, that cell must be converted into an integer before it can be used. However, if it is an integer,
                firstCell can be replaced by that integer. */

            }

            if (!isNumber(secondCell)) {
                int column = getColumn(secondCell);
                int row = getRow(secondCell);
                secondCell = sheet[row][column];


                if (secondCell[0] == '=') secondCell = convertFormula(sheet, secondCell, row, column);

                /*Same case applies to secondCell - if the new cell references a formula, it must be converted before it can be used in an operation */
            }

            if (firstCell == "#ERROR" || secondCell == "#ERROR") return "#ERROR";

            /* Special case - if a cell references a cell that is self referential on itself, it must stop the operation and replace the formula with "#ERROR" */

            string value = performOperation(firstCell, secondCell, operators[i]);

            /* Performs either multiplication or division on the two cells */

            cellIdentifiers[i] = value;
            cellIdentifiers.erase(cellIdentifiers.begin() + i + 1);
            operators.erase(operators.begin() + i);

            /*cellIdentifiers updates having done exactly one multiplication or division operation between two values. One of the old values is then removed
            * from the vector to shrink its size. Also, the operation just used must be removed from the operators vector */
        }
        else {

            /*This is the case in which there are neither multiplication or division operations to be done on any of the cells. Because it is just addition and subtraction,
            the order does not matter and we can simply go from left to right */

            string firstCell = cellIdentifiers[0];
            string secondCell = cellIdentifiers[1];

            if (!isNumber(firstCell)) {
                int column = getColumn(firstCell);
                int row = getRow(firstCell);
                firstCell = sheet[row][column];

                if (firstCell[0] == '=') firstCell = convertFormula(sheet, firstCell, row, column);
            }

            if (!isNumber(secondCell)) {
                int column = getColumn(secondCell);
                int row = getRow(secondCell);
                secondCell = sheet[row][column];

                if (secondCell[0] == '=') secondCell = convertFormula(sheet, secondCell, row, column);
            }

            if (firstCell == "#ERROR" || secondCell == "#ERROR") return "#ERROR";

            string value = performOperation(firstCell, secondCell, operators[0]);

            cellIdentifiers[0] = value;
            cellIdentifiers.erase(cellIdentifiers.begin() + 1);
            operators.erase(operators.begin());

            /*Because the operations are performed from left to right, operators simply looses its leftmost operator. Additionally, cellIdentifiers[0] gets replaced with
            the new correct value (either cellIdentifiers[0] + cellIdentifiers[1] or cellIdentifiers[0] - cellIdentifiers[1]. So, cellIdentifiers[1] can be removed. */
        }
    }

    /* Once this point is reached, there is only one value remaining in cellIdentifiers - a single integer value that represents the original formula.
        Because there is only one identifier remaining, we can return cellIdentifiers[0]. */

    return cellIdentifiers[0];
}

vector<string> getIdentifiers(const string& s) {
    vector<string> cellIdentifiers;

    /* Makes sure that s does in fact contain identifiers. */
    if (containsIdentifier(s)) {

        /* Checks if the first character in s is wither an operator or an equals sign (signifying it to be a formula). In this case, it will recursively call itself,
            adding the rest of the operators to cellIdentifiers. */

        if (isOperator(s[0]) || s[0] == '=') {
            vector<string> nextIdentifiers = getIdentifiers(s.substr(1, s.length() - 1));
            cellIdentifiers.insert(cellIdentifiers.end(), nextIdentifiers.begin(), nextIdentifiers.end());
        }

        /* If the first character is not an operator or an equals sign, that means it is a valid cell identifier (either an integer value or a reference to another cell) */

        else {
            string identifier;
            for (int i = 0; !isOperator(s[i]) && i < s.length(); i++) {
                identifier += s[i];
            }

            /*Identifier concatinates the next character with itself until it reaches either the end of the string or an operator, in which case the loop terminates.
                By the end, identifier will either be an integer or a valid reference to another cell (string integer) */

            cellIdentifiers.push_back(identifier);

            vector<string> nextIdentifiers = getIdentifiers(s.substr(identifier.length(), s.length() - identifier.length()));
            cellIdentifiers.insert(cellIdentifiers.end(), nextIdentifiers.begin(), nextIdentifiers.end());

            /*Each identifier is added to cellIdentifier, and the recursive call is appendded to the end of cellIdentifiers*/
        }
    }

    return cellIdentifiers;

    /*After this final return statemenet, cellIdentifiers will contain all of the integers or cell references within s (cellIdentifiers = {A1, 3, B3, C13, BA3} )*/
}

vector<char> getOperators(const string& s) {
    /*This function is much more simple than getIdentifier as each operator is simply one character, and a character either is an operator or is not. */
    vector<char> operators;
    for (int i = 0; i < s.length(); i++) {
        if (isOperator(s[i])) operators.push_back(s[i]);

        /*If any character in s is one of the operators (*, +, - or /), that character is added to the vector */
    }
    return operators;

    /*Will contain all of the operators used in s (operators = {'*', '+', '*'})*/
}


string performOperation(const string& firstCell, const string& secondCell, const char& firstOperator) {
    if (firstCell == "#ERROR" || secondCell == "#ERROR") return "#ERROR";

    /*If either of the cells were self referencial, return "#ERROR" */

    /*This function uses a simple switch loop to perform the intended operation on the two cells. */

    string finalVal;
    int val = 0;
    switch (firstOperator) {
    case '+':
        val = stoi(firstCell) + stoi(secondCell);    // Temporarily convert the strings into integers to perform operations
        finalVal = to_string(val);                   // convert the final value back into a string to be returned
        break;
    case '-':
        val = stoi(firstCell) - stoi(secondCell);
        finalVal = to_string(val);
        break;
    case '*':
        val = stoi(firstCell) * stoi(secondCell);
        finalVal = to_string(val);
        break;
    case '/':
        val = stoi(firstCell) / stoi(secondCell);
        finalVal = to_string(val);
        break;
    default:
        break; // Can never get here - by definition the vector<char> operators can only contain valid operators
    }
    return finalVal;
}


int getColumn(const string& s) {
    /* Takes a cellIdentifier as an input and finds the correct column based on the string at the end of the cellIdentifier (i.e. AA corresponds to column 27, index 26) */
    string columnStr;
    int columnVal = 0;

    /* Change all column identifiers to upper case to allow for upper or lower in the input spreadsheet */
    for (int i = 0; !isdigit(s[i]); i++) {
        if (s[i] == '=') continue;
        columnStr += toupper(s[i]);
    }

    /* columnStr contains the string corresponding to the column number - can be thought of as a base-26 number */

    for (int i = columnStr.length() - 1; i >= 0; i--) {
        int charVal = (int)columnStr[i] - 64;
        charVal *= pow(26, i);
        columnVal += charVal;
    }

    return columnVal - 1;
}

int getRow(const string& s) {
    /* Much more simple than getColumn(), getRow simply reads the digits on the end of the cellIdentifier and returns the integer that those digits represent.*/
    int indexOfRow = 0;
    for (int i = 0; !isdigit(s[i]); i++) indexOfRow++;
    return stoi(s.substr(indexOfRow, s.length() - indexOfRow)) - 1;
}
/* Note - in both getColumn() and getRow(), 1 was subtracted from the final return value. This is mainly for indexing purposes, as the cells in the spreadsheet begin at 1.
    So, cell A1 would correspond to the first row and first column, which would be sheet[0][0] */


bool circularReference(const vector<string>& cellIdentifiers, const vector<vector<string>>& sheet, const int& currentRow, const int& currentColumn) {

    /* This function is the reason that many of the other functions required the currentRow and currentColumn to be passed around. In order to know if a cell
        references itself (or eventually references itself through many different formulas), we need to know what cell is currently being looked at. */

    bool circular = false;
    for (int i = 0; i < cellIdentifiers.size(); i++) {

        /* If a cellIdentifier is a number, it is automatically not self referential and can be ignored */

        if (!isNumber(cellIdentifiers[i])) {
            int row = getRow(cellIdentifiers[i]);
            int column = getColumn(cellIdentifiers[i]);

            string newIdentifiers = sheet[row][column];
            vector<string> newCellIdentifiers = getIdentifiers(newIdentifiers);


            if (row == currentRow && column == currentColumn) {

                /*Check the base case - for if the current row and column are being referenced within the cell */

                return true;
            }
            else {

                /* Checks the more general case - think of any given cellIdentifer as the root node of a tree - where its children nodes are the cells it references.
                    Then, those referenced cells can also have children if they themselves are formulas. This "else" clause essentially just checks all of those possible
                    'children' to see if any of them are equal to the current row or column. If they are, it will trigger the "if" clause in a future recursive call
                    and return true. However, if it never loops back to reference itself, then all leaf nodes are integers and circular = false for all calls. */

                circular = circularReference(newCellIdentifiers, sheet, currentRow, currentColumn);
            }
        }

    }
    return circular;
}

void outputToFile(const vector<vector<string>>& sheet, const string& outputFileName) {
    ofstream output;
    output.open(outputFileName);
    /* Opens an output stream to the desired file. */


    for (int i = 0; i < sheet.size(); i++) {
        for (int j = 0; j < sheet[i].size(); j++) {
            if (sheet[i][j] != string(1, delimiter)) {
                if (j == sheet[i].size() - 1) output << sheet[i][j];  // Ensures no extra tabspaces at end of row 
                else output << sheet[i][j] << string(1, delimiter);
            }
            else {
                output << sheet[i][j];
            }
        }
        output << endl;
    }

    output.close();
}


/* All of the following functions are very simple helper functions to either format, give a bool relating to whether or not some value has a certain attribute, or the
    index of a desired value within a vector */

    /* Used to format a formula, so that =  A1   + B1 returns as =A1+B1 */
void removeSpaces(string& s) {
    string finalS;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] != ' ') finalS += s[i];
    }
    s = finalS;
}

/*While anytime this is called, the same effect can be done by isdigit(s[0]), isNumber(s) is more readable, showing that the string as a whole is a valid int rather than just
the first character */
bool isNumber(const string& s) {
    for (int i = 0; i < s.length(); i++) {
        if (!isdigit(s[i])) return false;
    }
    return true;
}


/*Essentially checks that any given cellIdentifier is a valid input, either an integer or references another valid cell */
bool validIdentifiers(const vector<string>& cellIdentifiers, const vector<vector<string>>& sheet) {

    for (int i = 0; i < cellIdentifiers.size(); i++) {

        /*If the cellIdentifier is a number, it is automatically acceptable and can be returned immediately */

        if (isNumber(cellIdentifiers[i])) {
            break;
        }

        int row = getRow(cellIdentifiers[i]);
        int column = getColumn(cellIdentifiers[i]);

        if (row > sheet.size() || column > sheet[0].size() || sheet[row][column][0] == delimiter) return false;

        /* If the column or row being referenced by a cellIdentifier is either out of range of the spreadsheet or contains a '\t', it is not a valid identifier. */
    }
    return true;
}

int indexOfMultDiv(const vector<char>& operators) {
    /* Finds the first index of a '*' or '/' operator because they take precidence in operations. */
    for (int i = 0; i < operators.size(); i++) {
        if (operators[i] == '*' || operators[i] == '/') return i;
    }
    return -1;
    /* If there were no multiplicaion or division operators, return a -1. Note: future calls can treat this function as a bool in an if (indexOfMultDiv(operators) > 0) will return
        true if there were multiplication or division operators and false if there were none. */
}

bool containsIdentifier(const string& s) {
    /*Simply checks to make sure that any given formula contains digits. Then, it will either be a reference to another cell or an integer value. */
    for (int i = 0; i < s.length(); i++) {
        if (isdigit(s[i])) return true;
    }
    return false;
}

bool containsDelimiter(const string& s) {
    /*Simply tells whether or not a string contains a delimiter or not */
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == delimiter) {
            return true;
        }
    }
    return false;
}

bool isOperator(const char& c) {
    /* Determines if any given character is one of the valid operators */
    for (int i = 0; i < sizeof(ops) / sizeof(ops[0]); i++) {
        if (c == ops[i]) return true;
    }
    return false;
}
