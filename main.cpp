/*
This software performs Gauss elimination using parallel programming paradigm.

The input matrix is given as .csv file. Output vector is also .csv.

April 2020
*/

#include <iostream>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <string>
#include <fstream>
#include <bits/stdc++.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

const char endOfLine = '\n';
static string dataLogger = "\n***New Data Logger***";

struct parallelParam{
    omp_sched_t scheduleType;
    int chunkSize;
    int wantedThreads;
};

static parallelParam parameters ={ omp_sched_auto, 100, 8 };//default parallel parameters

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

class cMatrix
{
public:
    int width;
    int height;
    float** data;
    double timeSeq, timePar;
    bool errorFlag;

    cMatrix(int w, int h);//default constructor, sets all elements to 0
    cMatrix(string sourceName);//reading constructor
    cMatrix(const cMatrix& source);//copy constructor
    ~cMatrix();
    void mPrint(string name);//print the elements to the file
    void screenPrint();//print the elements to the screen
};

cMatrix::cMatrix(int w, int h)//default constructor
{
    width = w;
    height = h;
    timePar = 0;
    timeSeq = 0;
    errorFlag = false;
    data = new float* [height];
    for (int i = 0; i < height; i++)
    {
        data[i] = new float[width];
        for (int j = 0; j < width; j++)
        {
            data[i][j] = 0;
        }
    }
}

cMatrix::cMatrix(const cMatrix& source)//copy constructor
{
    srand( time( NULL ) );
    width = source.width;
    height = source.height;
    timePar = source.timePar;
    timeSeq = source.timeSeq;
    data = new float* [height];
    for (int i = 0; i < height; i++)
    {
        data[i] = new float[width];
        for (int j = 0; j < width; j++)
        {
            data[i][j] = source.data[i][j];
        }
    }
}

cMatrix::cMatrix(string sourceName)//file copy constructor
{
    errorFlag = false;
    string line;
    string s;

    dataLogger += endOfLine;
    dataLogger += "Reading file: ";
    dataLogger += sourceName;
    dataLogger += "...";
    dataLogger += "time: ";
    dataLogger += currentDateTime();
    dataLogger += "...";


    ifstream sourceFile;
    sourceFile.open(sourceName);

    if (!sourceFile.is_open()){
        cout<<"Cannot open files."<<endl;
        dataLogger += "Cannot open files.";
        errorFlag = true;
        width = 1;
        height = 1;
        data = new float*[1];
        data[0] = new float;
        data[0][0] = 0;
        return;
    }

    sourceFile>>height;
    if(sourceFile.fail()){
        cout<<"Cannot read files."<<endl;
        dataLogger += "Cannot read files.";
        errorFlag = true;
        width = 1;
        height = 1;
        data = new float*[1];
        data[0] = new float;
        data[0][0] = 0;
        return;
    }

    width = height + 1;
    /*sourceFile>>width;
    if(sourceFile.fail()){
        cout<<"Cannot read files."<<endl;
        dataLogger += "Cannot read files.";
        errorFlag = true;
        width = 1;
        height = 1;
        data = new float*[1];
        data[0] = new float;
        data[0][0] = 0;
        return;
    }*/

    getline(sourceFile, line); //before reading rows we need to change line

    data = new float* [height];
    for (int i = 0; i < height; i++)
    {
        data[i] = new float[width];
        getline(sourceFile, line);

        for (int j = 0; j < width; j++)
        {
            s = line.substr(0,line.find(";"));
            line.erase(0,s.length()+1);
            data[i][j] = atof(s.c_str());

            if(sourceFile.fail()){
                cout<<"Cannot read files."<<endl;
                dataLogger += "Cannot read files.";
                errorFlag = true;
                for (int i = 0; i < height; i++){//to prevent memory leaks
                    delete data[i];
                }
                delete data;
                width = 1;
                height = 1;
                data = new float*[1];
                data[0] = new float;
                data[0][0] = 0;
                return;
            }
        }
    }
    dataLogger += " OK";
    cout<<"File reading: OK"<<endl;
}

cMatrix::~cMatrix()//destructor deallocates memory
{
    for (int i = 0; i < height; i++)
    {
        delete data[i];
    }
    delete data;
}

void cMatrix::screenPrint()//printing to screen
{
    using namespace std;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            cout<<data[i][j]<<"\t";
        }
        cout<<endl;
    }
    cout<<endl;
}

void cMatrix::mPrint(string name)//printing to file
{
    using namespace std;

    ofstream resultFile;
    errorFlag = false;

    string outputName = name;
    outputName += "_";
    outputName += to_string(timeSeq);
    outputName += "_";
    outputName += to_string(timePar);
    outputName += ".csv";

    dataLogger += endOfLine;
    dataLogger += "File writing... ";
    dataLogger += outputName;

    dataLogger += "...time: ";
    dataLogger += currentDateTime();
    dataLogger += "...";

    resultFile.open (outputName);

    //resultFile<<height<<endl;
    resultFile<<width<<endl;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            resultFile<<fixed << setprecision(6)<<data[i][j];
            if(j<(width-1)){
                resultFile<<";";
            }
        }
        resultFile<<endl;
    }
    resultFile<<endl;
    resultFile.close();
    dataLogger += " OK";
}

void updateDataLog()//printing data log to file and clearing it
{
    using namespace std;
    ofstream resultFile;
    string outputName = "DataLog.txt";
    resultFile.open (outputName,std::ofstream::app);
    if (resultFile.is_open())
    {
        resultFile<<endl;
        resultFile<<dataLogger;
        resultFile.close();
    }
    else
        cout << "Unable to open file";
    dataLogger = "";
}

void readDataLog()//reading data log file and printing it to the screen
{
    using namespace std;
    string line = "";
    string name = "DataLog.txt";
    ifstream dataFile;

    dataFile.open(name);

    if (!dataFile.is_open()){
        cout<<"Cannot open data files."<<endl;
        return;
    }
    if(dataFile.fail()){
        cout<<"Cannot read files."<<endl;
    }
    do
    {
        cout<<line;
        getline(dataFile, line);//gets line from a file
        cout<<endl;//prints it to the screen
    }while(!dataFile.fail());
}

//changes options of parallel tasks
void parallelOptionChange()
{
    int optionChosen;//chosen option

    dataLogger += endOfLine;
    dataLogger += "Changing parallel options: ";
    dataLogger += currentDateTime();
    dataLogger += ", ";

    do{
        cout<<"*******************************"<<endl;
        cout<<"Choose 1 to set a static scheduling:"<<endl;
        cout<<"Choose 2 to set a dynamic scheduling:"<<endl;
        cout<<"Choose 3 to set a guided scheduling:"<<endl;
        cout<<"Choose 4 to set an auto scheduling:"<<endl;

        cin.clear();
        cin.ignore(10000,'\n');
        cin>>optionChosen;

        if(cin.fail()){
            cout<<"Choose a correct value."<<endl;
            continue;
        }

        if (optionChosen==1)
        {
            parameters.scheduleType = omp_sched_static;
            break;
        }
        else if (optionChosen==2)
        {
            parameters.scheduleType = omp_sched_dynamic;
            break;
        }
        else if (optionChosen==3)
        {
            parameters.scheduleType = omp_sched_guided;
            break;
        }
        else if (optionChosen==4)
        {
            parameters.scheduleType = omp_sched_auto;
            break;
        }
        else
        {
            cout<<"Choose a correct value."<<endl;
        }
    }while(1);

    do{
        cout<<"Choose size of chunks:"<<endl;
        cin.clear();
        cin.ignore(10000,'\n');

        cin>>optionChosen;

        if(cin.fail()){
            cout<<"Choose a correct value."<<endl;
            continue;
        }

        if(sizeof(optionChosen)==sizeof(int))
        {
            parameters.chunkSize = optionChosen;
            break;
        }

        else
        {
            cout<<"Choose a correct value."<<endl;
        }

    }while(1);

    do{
        cout<<"Choose a desired number of threads:"<<endl;
        cin.clear();
        cin.ignore(10000,'\n');

        cin>>optionChosen;

        if(cin.fail()){
            cout<<"Choose a correct value."<<endl;
            continue;
        }

        if(sizeof(optionChosen)==sizeof(int))
        {
            parameters.wantedThreads = optionChosen;
            break;
        }

        else
        {
            cout<<"Choose a correct value."<<endl;
        }

    }while(1);
}

cMatrix matrixGaussianElimination(cMatrix* matrixArg, bool* errors)
//gives the Gaussian elimination solution vector (matrix type) of a given matrix
{
    /*taken from an omp enum sched type declaration
    omp_sched_static = 1,
    omp_sched_dynamic = 2,
    omp_sched_guided = 3,
    omp_sched_auto = 4
    */
    omp_set_schedule(parameters.scheduleType,parameters.chunkSize);
    omp_set_num_threads(parameters.wantedThreads);
    int index = 0;
    double time;

    dataLogger += endOfLine;
    dataLogger += "Gaussian elimination time: ";
    dataLogger += currentDateTime();
    dataLogger += ", amount of equations: ";
    dataLogger += to_string(matrixArg->height);
    dataLogger += ", ";
    dataLogger += "parallel schedule type: ";
    switch (parameters.scheduleType)
    {
        case 1:
            dataLogger += "static";
            break;
        case 2:
            dataLogger += "dynamic";
            break;
        case 3:
            dataLogger += "guided";
            break;
        case 4:
            dataLogger += "auto";
            break;
        default:
            dataLogger += "unknown";
    }
    dataLogger += ", ";
    dataLogger += "parallel size of chunk: ";
    dataLogger += to_string(parameters.chunkSize);
    dataLogger += ", ";
    dataLogger += "parallel wanted number of threads: ";
    dataLogger += to_string(parameters.wantedThreads);
    dataLogger += ", ";

    if (matrixArg->errorFlag){
        std::cout<<"Input error."<<std::endl;
        dataLogger += "Input error.";
        *errors = true;
        cMatrix result = cMatrix(1, 1);
        return result;
    }

    if (matrixArg->width!=(matrixArg->height)+1){
        std::cout<<"Dimension mismatch. Elimination."<<std::endl;
        dataLogger += "Dimension mismatch. Elimination.";
        *errors = true;
        cMatrix result = cMatrix(1, 1);
        return result;
    }

    cMatrix result = cMatrix(matrixArg->height, 1);//result declaration
    cMatrix tmp = cMatrix(*matrixArg);//to keep original input values a matrix copy is created

    //*************sequence part*******************************
    time = omp_get_wtime();

    //Stage 1 - elimination
    for (int i = 0; i < tmp.height; i++)
    {
        int maxIndex = i;
        for (int j = i; j < tmp.height; j++)//searching for a maximum element
        {
            if(abs(tmp.data[j][i])>abs(tmp.data[maxIndex][i]))
            {
                maxIndex = j;
            }
        }

        if(maxIndex!=(i))//changing rows if needed
        {
            float tmpFloat = 0;//holds initial value for the calculations
            for(int k = 0; k < tmp.width; k++)
            {
                tmpFloat = tmp.data[i][k];
                tmp.data[i][k] = tmp.data[maxIndex][k];
                tmp.data[maxIndex][k] = tmpFloat;
            }
        }

        if(tmp.data[i][i]==0){//rows with maximum element equal to 0 are omitted
            index++;
            *errors = true;
            continue;
        }

        for (int j = i + 1; j < tmp.height; j++)//reduction
        {
            float tmpFloat = tmp.data[j][i];//holds initial value for the calculations - it would normally change in process
            for(int k = 0; k < tmp.width; k++)
            {
                tmp.data[j][k] = tmp.data[j][k] - tmpFloat*tmp.data[i][k]/tmp.data[i][i];
            }
        }
    }

    //tmp.screenPrint();//reordered input matrix can be printed to the screen

    //Stage 2 - solution
    for(int i = result.width-1; i >= 0; i--)
    {
        float tmpSum = 0;
        for(int j = i; j < result.width; j++)
        {
            tmpSum += tmp.data[i][j] * result.data[0][j];
        }
        result.data[0][i] = (tmp.data[i][tmp.width-1] - tmpSum)/tmp.data[i][i];
    }

    time = omp_get_wtime() - time;
    result.timeSeq = time;

    std::cout<<"Sequence time: "<<result.timeSeq<<std::endl;
    dataLogger += "sequence time: ";
    dataLogger += to_string(result.timeSeq);
    dataLogger += ", ";
    dataLogger += to_string(index);
    dataLogger += " rows omitted in sequence part, ";
    index = 0;

    //*************parallel part*******************************
    cMatrix tmp2 = cMatrix(*matrixArg);//to keep original input values a matrix copy is created
    //second copy of the data for parallel algorithm to start from the beginning
    cMatrix result2 = cMatrix(matrixArg->height, 1);//result declaration
    time = omp_get_wtime();

    //Stage 1 - elimination

    for (int i = 0; i < tmp2.height; i++)
    {
        int maxIndex = i;

        ///*
        #pragma omp parallel shared(maxIndex)
        {
            int localMax = i;
            #pragma omp for schedule(runtime)
            for (int j = i; j < tmp2.height; j++)//searching for a maximum element
            {
                if(abs(tmp2.data[j][i])>abs(tmp2.data[localMax][i]))
                {
                    localMax = j;
                }
            }
            #pragma omp critical
            {
                if(abs(tmp2.data[localMax][i])>abs(tmp2.data[maxIndex][i]))
                {
                    maxIndex = localMax;
                }
            }
        }
        //*/
        /*
        for (int j = i; j < tmp2.height; j++)//searching for a maximum element
            {
                if(abs(tmp2.data[j][i])>abs(tmp2.data[maxIndex][i]))
                {
                    maxIndex = j;
                }
            }
        */

        if(maxIndex!=(i))//changing rows if needed
        {
            #pragma omp parallel shared(tmp2)
            {
                float tmpFloat = 0;//holds initial value for the calculations
                #pragma omp for schedule(runtime) nowait
                for(int k = 0; k < tmp2.width; k++)
                {
                    tmpFloat = tmp2.data[i][k];
                    tmp2.data[i][k] = tmp2.data[maxIndex][k];
                    tmp2.data[maxIndex][k] = tmpFloat;
                }
            }
        }

        if(tmp2.data[i][i]==0){//rows with maximum element equal to 0 are omitted
            index++;
            *errors = true;
            continue;
        }

        for (int j = i + 1; j < tmp2.height; j++)//reduction
        {
            float tmpFloat = tmp2.data[j][i];//holds initial value for the calculations - it would normally change in process
            //#pragma omp parallel lastprivate(tmp2)
            {
                //#pragma omp for schedule(runtime)
                for(int k = 0; k < tmp2.width; k++)
                {
                    tmp2.data[j][k] = tmp2.data[j][k] - tmpFloat*tmp2.data[i][k]/tmp2.data[i][i];
                }
            }
        }
    }

    //cout<<index<<" rows omitted."<<endl;//rows omitted can be printed to the screen
    //tmp2.screenPrint();//reordered input matrix can be printed to the screen

    //Stage 2 - solution
    for(int i = result2.width-1; i >= 0; i--)
    {
        float tmpSum = 0;
        #pragma omp parallel shared(tmpSum)
        {
            float tmpSumLocal = 0;
            #pragma omp for schedule(runtime)
            for(int j = i; j < result2.width; j++)
            {
                tmpSumLocal += tmp2.data[i][j] * result2.data[0][j];
            }
            #pragma omp critical
            {
                tmpSum += tmpSumLocal;
            }
        }
        result2.data[0][i] = (tmp2.data[i][tmp2.width-1] - tmpSum)/tmp2.data[i][i];
    }

    time = omp_get_wtime() - time;
    result.timePar = time;

    std::cout<<"Parallel time: "<<result.timePar<<std::endl;
    dataLogger += "parallel time: ";
    dataLogger += to_string(result.timePar);
    dataLogger += ", ";
    dataLogger += to_string(index);
    dataLogger += " rows omitted in parallel part, ";

    result2.timePar = result.timePar;
    result2.timeSeq = result.timeSeq;//to prevent both sections to interfere with each other they operate on separate
    //copies of input data and solution vector; tmp and result for sequence while tmp2 and result2 for parallel

    if(index == 0)
    {
        std::cout<<"Gaussian elimination: OK"<<std::endl;
        dataLogger += "...OK";
    }
    else
    {
        std::cout<<"Gaussian elimination: OK - some rows were omitted, so the result is incorrect."<<std::endl;
        dataLogger += "...OK - some rows were omitted, so the result is incorrect.";
    }
    *errors = false;
    return result2;
}

int main()
{
    bool errors = false;//general error flag
    string nameInput = "C.csv";//input file name
    string nameOutput = "X";//output file name

    int option = 0;//chosen option
    bool dataFlag = false;//flag for the menu choice validation



    do{
            if(option!=1)
            {
                updateDataLog();
            }
            cout<<"*******************************"<<endl;
            cout<<"Choose 1 to see the dataLogger:"<<endl;
            cout<<"Choose 2 to exit:"<<endl;
            cout<<"Choose 3 to read a file:"<<endl;
            cout<<"Choose 4 to perform task:"<<endl;
            cout<<"Choose 5 to change parallel execution options:"<<endl;

            cin.clear();

            if(dataFlag){
                cin.ignore(10000,'\n');
            }

            cin>>option;

            dataFlag = true;

            if(cin.fail()){
                cout<<"Choose a correct value."<<endl;
                continue;
            }

            if (option==1){
                readDataLog();
            }

            else if (option==2){
                break;
            }

            else if (option==3){
                cMatrix matrixA = cMatrix(nameInput);
                if (matrixA.errorFlag)
                {
                    cout<<"Error."<<endl;
                }
            }

            else if (option ==4){
                cMatrix matrixA = cMatrix(nameInput);

                cMatrix matrixX = cMatrix(matrixGaussianElimination(&matrixA, &errors));
                if(!errors){
                    matrixX.mPrint(nameOutput);
                }
                else{
                    cout<<"Error."<<endl;
                }
            }

            else if (option ==5){
                parallelOptionChange();
            }

            else{
                cout<<"Choose a correct value."<<endl;
                continue;
            }

    }while(1);

    cout << "End of program." << endl;
    return 0;
}
