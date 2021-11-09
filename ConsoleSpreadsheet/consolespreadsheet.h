using namespace std;

char delimiter = '\t';
char ops[] = { '+', '-', '*', '/' };

/* Takes the input file fileName and separates it by rows into a vector<string> */
vector<string> getDataFromSpreadsheet(const string& fileName);

/* Takes each individual row from the spreadsheet and separates them into cells */
vector<vector<string>> separateRows(const vector<string>& rows);

/* Used to aid separateRows method, takes an individual row and separates it into its cells */
vector<string> separateRowIntoCells(const string& s);

/* Checks that any given string contains the delimiter '\t', used to tokenize a row into its cells */
bool containsDelimiter(const string& s);

/* Converts all formulas in the spreadsheet into the integers they represent*/
void convertFormulasToIntegers(vector<vector<string>>& sheet);

/* Takes a single formla string and converts it into its expected value (helps convertFormulaToIntegers) */
string convertFormula(const vector<vector<string>>& sheet, string s, const int& currentRow, const int& currentCol);

/*Used for formatting the formula, removes spaces so that "A1   + B3" can still be read */
void removeSpaces(string& s);

/* Takes two values and performs the correct operation on them */
string performOperation(const string& firstCell, const string& secondCell, const char& firstOperator);

/* Extract the operators out of a formula and return the vector<char> of those operators */
vector<char> getOperators(const string& s);

/* Extract the cell identifiers (A1, B4, etc) out of the formlua and returns the vector<string> of those identifiers */
vector<string> getIdentifiers(const string& s);

/* Checks that all cell identifiers within a formula reference non-zero, existing cells */
bool validIdentifiers(const vector<string>& cellIdentifiers, const vector<vector<string>>& sheet);

/* Ensures that a cell does not contain an identifier to itself */
bool circularReference(const vector<string>& cellIdentifiers, const vector<vector<string>>& sheet, const int&, const int& currentColumn);

/* In any given vector<char> of operators from a formula, finds the location of the first multiplication or division sign (allows for precidence in operations) */
int indexOfMultDiv(const vector<char>& operators);

/* Checks that a formula contains an identifier to distinguish it from an integer or a simple data copy (=1, =A4) */
bool containsIdentifier(const string& s);

/* Takes a cell identifier and outputs the index of the row it references (A4 references row 4, outputs an index of 3) */
int getRow(const string& s);

/* Takes a cell identifier and outputs the index of the column it references (B5 references column 2, outputs an index of 1) */
int getColumn(const string& s);

/* Checks if a string is a number or not - used to distinguish when a formula references a cell or uses a constant integer */
bool isNumber(const string& s);

/* Checks if a character is one of the 4 operators */
bool isOperator(const char& c);

/* Outputs the formatted spreadsheet to output.txt */
void outputToFile(const vector<vector<string>>& sheet, const string& outputFileName);