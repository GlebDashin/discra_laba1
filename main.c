#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

// Вводить ~~ толькой через ~(~)
/*
6   ~       инверсия
5   * |     конъюнкция, штрих Шеффера
4   ^       XOR
3   + #     дизъюнкция, стрелка Пирса
2   > <     импликация, коимпликация
1   =       эквивалентность 
*/

struct stackNode{
    int data;
    struct stackNode *nextPtr;
};

typedef struct stackNode StackNode;
typedef StackNode *StackNodePtr;

struct stringStackNode{
    char *data;
    struct stringStackNode *nextPtr;
};

typedef struct stringStackNode StringStackNode;
typedef StringStackNode *StringStackNodePtr;

void push(StackNodePtr *topPtr, int info);
int pop(StackNodePtr *topPtr);
void stringPush(StringStackNodePtr *topPtr, char *info);
char *stringPop(StringStackNodePtr *topPtr);

void plusOne(int arr[], int varCount);
void printTable(int rowCount, int columnCount, int table[rowCount][columnCount]);
bool SDNF(int rowCount, int columnCount, int table[rowCount][columnCount], char variable[], int varCount, char *string);
bool SKNF(int rowCount, int columnCount, int table[rowCount][columnCount], char variable[], int varCount, char *string);
void printfFictitiousVariable(int rowCount, int columnCount, int table[rowCount][columnCount], char variable[], int varCount);
bool checkFictitiousRow(int row1[], int row2[], int charIndex, int columnCount);
void printPeirceBasis(char postfix[], char variable[], int varCount);
void printShefferBasis(char postfix[], char variable[], int varCount);
void printZhegalkinBasis(int rowCount, int columnCount, int table[rowCount][columnCount], int varCount, char variable[]);

void infixToPostfix(char infix[], char postfix[]);
int findVariable(char infix[], char variable[]);

int calculate(char postfix[], char variable[], int rawn[], int varCount);
int littleCalculate(int first, int two, char op);

int priority(char op);

int notOp(int first);    // ~  отрицание
int orOp(int first, int two);    // +  дизъюнкция
int andOp(int first, int two);    // *  конъюнкция
int xorOp(int first, int two);    // ^  исключающее ИЛИ
int eqOp(int first, int two);     // =  эквиваленция
int implOp(int first, int two);   // >  импликация
int rImplOp(int first, int two); // <  обратная импликация
int norOp(int first, int two);   // #  стрелка Пирса
int nandOp(int first, int two);   // |  штрих Шеффера



int main( int argc, char *argv[] ){
    SetConsoleOutputCP(CP_UTF8);
    // Читаем файл в char infix[]
    FILE *file = fopen( argv[1], "r" );
    if ( file == NULL ){
        printf( "Не правильно указан путь");
        return 0;
    }

    char infix[256];
    fgets ( infix, sizeof(infix), file );

    //Считаем количество переменных, записываем их и переводим инфикс в постфикс.
    char postfix[256];
    char variable[256];

    int varCount = findVariable(infix, variable);

    infixToPostfix(infix, postfix);

    //Выводим заголовки столбцов
    char row1[varCount+1];

    for (int i = 0; i<varCount ; i++) {
        row1[i]=variable[i];
    }
    row1[varCount]='\0';

    // Создаем таблицу истинности и заполняем всеми возможными значениями переменных и соотвественными значениями функции
    int table[1 << varCount][varCount+1];

    for (int i = 0; i<varCount; i++) {
        table[0][i] = 0;
    }
    table[0][varCount] = calculate(postfix, variable, table[0], varCount);

    for (int i = 1; i<(1 << varCount); i++) {
        for (int y = 0; y < varCount; y++) {
            table[i][y]=table[i-1][y];
        }
        plusOne(table[i], varCount);
        table[i][varCount] = calculate(postfix, variable, table[i], varCount);
    }
    
    // Выводит таблицу
    printf("%s %s\n", row1, infix);
    printTable(1 << varCount, varCount+1, table);

    //Строит и выводит СКНФ и СДНФ
    char stringSKNF[4096];
    char stringSDNF[4096];
    bool stringSKNFBool=SKNF(1 << varCount, varCount+1, table, variable, varCount, stringSKNF);
    bool stringSDNFBool=SDNF(1 << varCount, varCount+1, table, variable, varCount, stringSDNF);
    if (stringSKNFBool){
        printf("СКНФ: %s\n", stringSKNF);
    }
    else{
        printf("СКНФ нельзя построить, выражение тоджественно истинно\n");
    }
    if (stringSDNFBool){
        printf("СДНФ: %s\n", stringSDNF);
    }
    else{
        printf("СДНФ нельзя построить, выражение тождественно ложно\n");
    }

    //Выводит фиктивные переменные
    printfFictitiousVariable(1 << varCount, varCount+1, table, variable, varCount);

    //Вывод 7.1
    if (stringSDNFBool){
        printf("{*,+,~}: %s\n", stringSDNF);
    }
    else{
        printf("{*,+,~}: A*~A\n");
    }

    //Вывод7.2-7.3
    char postfixSDNF[6767];
    if (stringSDNFBool==1){
        infixToPostfix(stringSDNF, postfixSDNF);
    }
    else{
        char fakeInfix[10] = "A*~A";
        infixToPostfix(fakeInfix, postfixSDNF);
    }
    printPeirceBasis(postfixSDNF, variable, varCount);
    printShefferBasis(postfixSDNF, variable, varCount);

    //Вывод 7.4
    printZhegalkinBasis(1 << varCount, varCount+1, table, varCount, variable);

    return 0;
}

void push( StackNodePtr *topPtr, int info ){

    StackNodePtr newPtr;
    
    newPtr = malloc( sizeof( StackNode ) );
    if ( newPtr != NULL ){
        newPtr->data = info;
        newPtr->nextPtr = *topPtr;
        *topPtr = newPtr;
    }
    else{
        printf( "Не хватает памяти на %d\n", info);
        exit(EXIT_FAILURE);
    }
}

int pop( StackNodePtr *topPtr ){
    
    StackNodePtr tempPtr;
    int popValue = ( *topPtr )->data;

    tempPtr = *topPtr;
    *topPtr = ( *topPtr )->nextPtr;
    free( tempPtr );
    return popValue;
}

void stringPush( StringStackNodePtr *topPtr, char *info ){

    StringStackNodePtr newPtr;
    
    newPtr = malloc( sizeof( StringStackNode ) );
    if ( newPtr != NULL ){
        newPtr->data = info;
        newPtr->nextPtr = *topPtr;
        *topPtr = newPtr;
    }
    else{
        printf( "Не хватает памяти на %s\n", info);
        exit(EXIT_FAILURE);
    }
}

char *stringPop(StringStackNodePtr *topPtr ){
    
    StringStackNodePtr tempPtr;
    char *info = ( *topPtr )->data;

    tempPtr = *topPtr;
    *topPtr = ( *topPtr )->nextPtr;
    free( tempPtr );
    return info;
}

void plusOne(int arr[], int varCount) {
    bool point = false;
    for (int i = (varCount-1); i >= 0 ; i--) {
        if (arr[i] == 0) {
            arr[i] = 1;
            if (point == true) {
                for (int y = (i+1); y<varCount; y++) {
                    arr[y] = 0;
                }
                break;
            }
            else {
                break;
            }
        }
        else {
            point = true;
        }
    }
}

void printTable(int rowCount, int columnCount, int table[rowCount][columnCount]) {
    for (int i = 0; i < rowCount; i++) {
        for (int y = 0; y < (columnCount-1); y++){
            printf("%d", table[i][y]);
        }
        printf(" %d", table[i][columnCount-1]);
        printf("\n");
    }
}

int priority ( char op ){
    switch (op){
        case '=':
            return 1;
        case '>':
        case '<':
            return 2;
        case '+':
        case '#':
            return 3;
        case '^':
            return 4;
        case '*':
        case '|':
            return 5;
        case '~':
            return 6;
        default:
            return 0;
    }
}

int calculate(char postfix[], char variable[], int rawn[], int varCount){
    StackNodePtr stackPrtCalc = NULL;
    for (int i = 0; ;i++){
        if (postfix[i]=='\0'){
            return (stackPrtCalc->data);
        }
        if ( strchr("+*^=><#|", postfix[i] ) != NULL) {
            int two = pop(&stackPrtCalc);
            int first = pop(&stackPrtCalc);
            push(&stackPrtCalc, littleCalculate(first,two, postfix[i]));
            
        }
        else if(postfix[i]=='~') {
            push(&stackPrtCalc, notOp(pop(&stackPrtCalc)));
        }
        else {
            if (postfix[i] == '0' || postfix[i] == '1') {
                push(&stackPrtCalc, postfix[i]-'0');
            }
            else {
                for (int y = 0; y<varCount; y++) {
                    if (postfix[i] == variable[y]){
                        push(&stackPrtCalc, rawn[y]);
                        break;
                    }
                }
            }
        }
    }
}

int littleCalculate(int first, int two, char op){
    switch(op){
        case '+':
            return orOp(first, two);
        case '*':
            return andOp(first, two);
        case '^':
            return xorOp(first, two);
        case '=':
            return eqOp(first, two);
        case '>':
            return implOp(first, two);
        case '<':
            return rImplOp(first, two);
        case '#':
            return norOp(first, two);
        case '|':
            return nandOp(first, two);
    }
}

int notOp(int first) {
    if (first == 1){
        return 0;
    }
    return 1;
}    // ~  отрицание
int orOp(int first, int two) {
    return (first||two);
}    // +  дизъюнкция
int andOp(int first, int two) {
    return (first*two);
}    // *  конъюнкция
int xorOp(int first, int two) {
    return (first!=two);
}    // ^  исключающее ИЛИ
int eqOp(int first, int two) {
    return (first==two);
}     // =  эквиваленция
int implOp(int first, int two) {
    return (!first || two);
}  // >  импликация
int rImplOp(int first, int two) {
    return (!two || first);
} // <  обратная импликация
int norOp(int first, int two) {
    return (!(first+two));
}   // #  стрелка Пирса
int nandOp(int first, int two) {
    return (!(first*two));
} // |  штрих Шеффера

bool SDNF(int rowCount, int columnCount, int table[rowCount][columnCount], char variable[], int varCount, char *SDNF) {
    int oneCount = 0;
    int string_length = 0;
    for (int i = 0; i < rowCount; i++) {
        if (table[i][columnCount-1] == 1){
            if (oneCount != 0){
                SDNF[string_length] = '+';
                string_length++;
            }
            oneCount++;
            SDNF[string_length]='(';
            string_length++;
            for (int y = 0; y < (columnCount-1); y++){
                if (table[i][y] == 0){
                    SDNF[string_length] = '~';
                    SDNF[string_length+1]=variable[y];
                    string_length+=2;
                }
                else{
                    SDNF[string_length]=variable[y];
                    string_length++;
                }
                if (y < (varCount-1)){
                    SDNF[string_length] = '*';
                    string_length++;
                }
            }
            SDNF[string_length]=')';
            string_length++;
        }
    }
    SDNF[string_length]='\0';

    if (oneCount == 0) {
        return false;
    }
    else{
        return true;
    }
}

bool SKNF(int rowCount, int columnCount, int table[rowCount][columnCount], char variable[], int varCount, char *SKNF) {
    int nullCount = 0;
    int string_length = 0;
    for (int i = 0; i < rowCount; i++) {
        if (table[i][columnCount-1] == 0){
            if (nullCount != 0){
                SKNF[string_length] = '*';
                string_length++;
            }
            nullCount++;
            SKNF[string_length]='(';
            string_length++;
            for (int y = 0; y < (columnCount-1); y++){
                if (table[i][y] == 1){
                    SKNF[string_length] = '~';
                    SKNF[string_length+1]=variable[y];
                    string_length+=2;
                }
                else{
                    SKNF[string_length]=variable[y];
                    string_length++;
                }
                if (y < (varCount-1)){
                    SKNF[string_length] = '+';
                    string_length++;
                }
            }
            SKNF[string_length]=')';
            string_length++;
        }
    }
    SKNF[string_length]='\0';

    if (nullCount == 0) {
        return false;
    }
    else{
        return true;
    }
}

void printfFictitiousVariable(int rowCount, int columnCount, int table[rowCount][columnCount], char variable[], int varCount){
    char fictitiousVariable[256];
    int string_length = 0;
    for (int i = 0; i < varCount; i++){
        bool charIsFictive = true;
        int step = (1 << varCount-1-i);
        int z = 0;
        for (int y = 0; y < (rowCount/step/2); y++) {
            int zStep=z+step;
            for (; z<zStep; z++){
                if (checkFictitiousRow(table[z], table[z+step], i, columnCount) == false){
                    charIsFictive = false;
                    goto end;
                }
            }
            z +=step;
        }
        end:
        if (charIsFictive){
            fictitiousVariable[string_length] = variable[i];
            string_length++;
            if (i != (varCount-1)){
                fictitiousVariable[string_length]= ' ';
                string_length++;
            }
        }
    }
    fictitiousVariable[string_length] = '\0';
    printf("Фиктивные переменные: %s\n", fictitiousVariable);
}

bool checkFictitiousRow(int row1[], int row2[], int charIndex, int columnCount){
    for (int i = 0; i<columnCount; i++) {
        if (i != charIndex) {
            if (row1[i] != row2[i]){
                return false;
            }
        }
    }
    return true;
}

void printPeirceBasis(char postfix[], char variable[], int varCount){
    StringStackNodePtr stackPrtPeirce = NULL;

    for (int i = 0; ;i++){
        if (postfix[i]=='\0'){
            break;
        }
        if ( strchr("+*", postfix[i] ) != NULL) {
            char *textTwo = stringPop(&stackPrtPeirce);
            char *textOne = stringPop(&stackPrtPeirce);
            int textOneLen = strlen(textOne);
            int textTwoLen = strlen(textTwo);
            switch (postfix[i])
            {
            case '+':{
                char *text = malloc((textOneLen+textTwoLen)*2+6+3+1);
                if (text != NULL){
                    text[0]='(';
                    text[1]='(';
                    for (int y = 0; y < textOneLen; y++){
                        text[2+y]=textOne[y];
                    }
                    text[2+textOneLen]='#';
                    for (int y = 0; y<textTwoLen; y++){
                        text[3+textOneLen+y]=textTwo[y];
                    }
                    text[3+textOneLen+textTwoLen]=')';
                    text[4+textOneLen+textTwoLen]='#';
                    text[5+textOneLen+textTwoLen]='(';
                    for (int y = 0; y < textOneLen; y++){
                        text[6+textOneLen+textTwoLen+y]=textOne[y];
                    }
                    text[6+textOneLen*2+textTwoLen]='#';
                    for (int y = 0; y<textTwoLen; y++){
                        text[7+textOneLen*2+textTwoLen+y]=textTwo[y];
                    }
                    text[(textOneLen+textTwoLen)*2+6+3-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3-1-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3]='\0';
                    stringPush(&stackPrtPeirce, text);
                    free(textOne);
                    free(textTwo);
                }
                else{
                    printf("Не хватает памяти\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case '*':{
                char *text = malloc((textOneLen+textTwoLen)*2+6+3+1);
                if (text != NULL){
                    text[0]='(';
                    text[1]='(';
                    for (int y = 0; y < textOneLen; y++){
                        text[2+y]=textOne[y];
                    }
                    text[2+textOneLen]='#';
                    for (int y = 0; y<textOneLen; y++){
                        text[3+textOneLen+y]=textOne[y];
                    }
                    text[3+textOneLen*2]=')';
                    text[4+textOneLen*2]='#';
                    text[5+textOneLen*2]='(';
                    for (int y = 0; y < textTwoLen; y++){
                        text[6+textOneLen*2+y]=textTwo[y];
                    }
                    text[6+textOneLen*2+textTwoLen]='#';
                    for (int y = 0; y<textTwoLen; y++){
                        text[7+textOneLen*2+textTwoLen+y]=textTwo[y];
                    }
                    text[(textOneLen+textTwoLen)*2+6+3-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3-1-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3]='\0';
                    stringPush(&stackPrtPeirce, text);
                    free(textOne);
                    free(textTwo);
                }
                else{
                    printf("Не хватает памяти\n");
                    exit(EXIT_FAILURE);
                }
                break;  
            }
            default:
                break;
            }
        }
        else if(postfix[i]=='~') {
            char *textOne = stringPop(&stackPrtPeirce);
            int textOneLen = strlen(textOne);
            char *text = malloc(textOneLen*2+3+1);

            if (text != NULL){
                text[0]='(';
                for (int y = 0; y<textOneLen; y++){
                    text[1+y]=textOne[y];
                }
                text[1+textOneLen]='#';
                for (int y = 0; y<textOneLen; y++){
                    text[2+textOneLen+y]=textOne[y];
                }
                text[textOneLen*2+3-1]=')';
                text[textOneLen*2+3]='\0';
                stringPush(&stackPrtPeirce, text);
                free(textOne);
            }
            else{
                printf("Не хватает памяти\n");
                exit(EXIT_FAILURE);
            }
        }
        else {
            char *text = malloc(2);
            if (text != NULL){
                text[0]=postfix[i];
                text[1]='\0';
                stringPush(&stackPrtPeirce, text);
            }
            else{
                printf("Не хватает памяти\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    printf("{#}: %s\n", stackPrtPeirce->data);
}

void printShefferBasis(char postfix[], char variable[], int varCount){
    StringStackNodePtr stackPrtSheffer = NULL;

    for (int i = 0; ;i++){
        if (postfix[i]=='\0'){
            break;
        }
        if ( strchr("+*", postfix[i] ) != NULL) {
            char *textTwo = stringPop(&stackPrtSheffer);
            char *textOne = stringPop(&stackPrtSheffer);
            int textOneLen = strlen(textOne);
            int textTwoLen = strlen(textTwo);
            switch (postfix[i])
            {
            case '*':{
                char *text = malloc((textOneLen+textTwoLen)*2+6+3+1);
                if (text != NULL){
                    text[0]='(';
                    text[1]='(';
                    for (int y = 0; y < textOneLen; y++){
                        text[2+y]=textOne[y];
                    }
                    text[2+textOneLen]='|';
                    for (int y = 0; y<textTwoLen; y++){
                        text[3+textOneLen+y]=textTwo[y];
                    }
                    text[3+textOneLen+textTwoLen]=')';
                    text[4+textOneLen+textTwoLen]='|';
                    text[5+textOneLen+textTwoLen]='(';
                    for (int y = 0; y < textOneLen; y++){
                        text[6+textOneLen+textTwoLen+y]=textOne[y];
                    }
                    text[6+textOneLen*2+textTwoLen]='|';
                    for (int y = 0; y<textTwoLen; y++){
                        text[7+textOneLen*2+textTwoLen+y]=textTwo[y];
                    }
                    text[(textOneLen+textTwoLen)*2+6+3-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3-1-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3]='\0';
                    stringPush(&stackPrtSheffer, text);
                    free(textOne);
                    free(textTwo);
                }
                else{
                    printf("Не хватает памяти\n");
                    exit(EXIT_FAILURE);
                }
                break;
            }
            case '+':{
                char *text = malloc((textOneLen+textTwoLen)*2+6+3+1);
                if (text != NULL){
                    text[0]='(';
                    text[1]='(';
                    for (int y = 0; y < textOneLen; y++){
                        text[2+y]=textOne[y];
                    }
                    text[2+textOneLen]='|';
                    for (int y = 0; y<textOneLen; y++){
                        text[3+textOneLen+y]=textOne[y];
                    }
                    text[3+textOneLen*2]=')';
                    text[4+textOneLen*2]='|';
                    text[5+textOneLen*2]='(';
                    for (int y = 0; y < textTwoLen; y++){
                        text[6+textOneLen*2+y]=textTwo[y];
                    }
                    text[6+textOneLen*2+textTwoLen]='|';
                    for (int y = 0; y<textTwoLen; y++){
                        text[7+textOneLen*2+textTwoLen+y]=textTwo[y];
                    }
                    text[(textOneLen+textTwoLen)*2+6+3-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3-1-1]=')';
                    text[(textOneLen+textTwoLen)*2+6+3]='\0';
                    stringPush(&stackPrtSheffer, text);
                    free(textOne);
                    free(textTwo);
                }
                else{
                    printf("Не хватает памяти\n");
                    exit(EXIT_FAILURE);
                }
                break;  
            }
            default:
                break;
            }
        }
        else if(postfix[i]=='~') {
            char *textOne = stringPop(&stackPrtSheffer);
            int textOneLen = strlen(textOne);
            char *text = malloc(textOneLen*2+3+1);

            if (text != NULL){
                text[0]='(';
                for (int y = 0; y<textOneLen; y++){
                    text[1+y]=textOne[y];
                }
                text[1+textOneLen]='|';
                for (int y = 0; y<textOneLen; y++){
                    text[2+textOneLen+y]=textOne[y];
                }
                text[textOneLen*2+3-1]=')';
                text[textOneLen*2+3]='\0';
                stringPush(&stackPrtSheffer, text);
                free(textOne);
            }
            else{
                printf("Не хватает памяти");
                exit(EXIT_FAILURE);
            }
        }
        else {
            char *text = malloc(2);
            if (text != NULL){
                text[0]=postfix[i];
                text[1]='\0';
                stringPush(&stackPrtSheffer, text);
            }
            else{
                printf("Не хватает памяти\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    printf("{|}: %s\n", stackPrtSheffer->data);
}


void infixToPostfix(char infix[], char postfix[]){
    int infixLen = strlen(infix);
    infix[infixLen]=')';
    infix[infixLen+1]='\0';

    //Создаем массив символов для postifx      
    //Переводим инфикс в постфикс
    StackNodePtr stackPrt = NULL;

    push( &stackPrt, '(' );

    int j = 0;

    for ( int i = 0; stackPrt!=NULL; i++ ){
        if ( strchr("~+*^=><#|", infix[i] ) != NULL){
            while( priority( stackPrt->data )>=priority( infix[i] )){
                postfix[j]=pop( &stackPrt);
                j++;
            }
            push( &stackPrt, infix[i]);
        }
        else if( infix[i] == '(' ){
            push( &stackPrt, infix[i] );
        }
        else if( infix[i] == ')'){
            while( stackPrt->data != '('){
                postfix[j]=pop( &stackPrt );
                j++;
            }
            pop(&stackPrt);
        }
        else{
            if (infix[i] == '1' || infix[i] == '0'){
                postfix[j]=infix[i];
            }
            else {
                //Переводим все переменные в верхний регистр
                char bigChar = toupper(infix[i]);
                postfix[j]=bigChar;
            }
            j++;
        }
    }
    postfix[j]='\0';
    infix[infixLen]='\0';
}

int findVariable(char infix[], char variable[]){
    int varCount = 0;
    int infixLen = strlen(infix);
    for (int i = 0; i<infixLen; i++) {
        if (strchr("()~+*^=><#|01.", infix[i] ) == NULL) {
            char bigChar = toupper(infix[i]);
            for ( int y = 0; ; y++ ){
                if (y < varCount && bigChar == variable[y]){
                    break;
                }
                else if (varCount==y){
                    variable[y]=bigChar;
                    varCount++;
                    break;
                }
            } 
        }
    }
    variable[varCount]='\0';
    return varCount;
}

void printZhegalkinBasis(int rowCount, int columnCount, int table[rowCount][columnCount], int varCount, char variable[]){
    char res[6767];
    int stringLength = 0;
    int pascalTable[rowCount][rowCount];
    for(int i = 0; i < rowCount; i++){
        pascalTable[0][i]=table[i][columnCount-1];
    }
    int j = rowCount;
    for(int i = 1; i <rowCount; i++){
        j-=1;
        for(int y = 0; y < j; y++){
            pascalTable[i][y]=pascalTable[i-1][y]^pascalTable[i-1][y+1];
        }
    }
    for (int i = 0; i<rowCount; i++){
        if (pascalTable[i][0]==0){
            continue;
        }
        int z = 0;
        for(int y = 0; y<varCount; y++){
            if (table[i][y]){
                if (z>0){
                res[stringLength]='*';
                stringLength++;
                }
                res[stringLength]=variable[y];
                stringLength++;
                z++;
            }
        }
        if (z != 0){
            res[stringLength]='^';
            stringLength++;
        } 
        else if (z == 0){
            if (stringLength != 0){
                res[stringLength]='^';
                stringLength++;
            }
            res[stringLength]='1';
            stringLength++;
            res[stringLength]='^';
            stringLength++;
        } 
    }
    if (stringLength == 0){
        printf("{^,*,1}: 1^1\n");
        return;
    }
    else if (res[stringLength-1] == '^'){
        res[stringLength-1] = '\0';
    }
    else{
        res[stringLength]='\0';
    }
    printf("{^*1}: %s\n", res);
}
