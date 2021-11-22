#include <PPL.h>
#include<algorithm>
#include <chrono>
#include<execution>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

constexpr int MAX_TEST_CASE = 4;
/*
* Infrastructure to replace HackerRank input functions
*/
std::vector<int> convertInputLineToIntVector(std::string query_string)
{
    constexpr int query_size = 3;
    std::vector<int> query;

    std::string::iterator intStart = query_string.begin();
    std::string::iterator intEnd;

    for (int i = 0; i < query_size; i++)
    {
        intEnd = std::find(intStart, query_string.end(), ' ');
        int pos = intEnd - query_string.begin();
        std::string tempInt(intStart, intEnd);
        query.push_back(stoi(tempInt));
        if (intEnd < query_string.end())
        {
            intStart = query_string.begin() + pos + 1;
        }
    }

    return query;
}

std::vector<std::vector<int>> getIntVectors(std::ifstream* inFile)
{
    std::vector<std::vector<int>> inputVector;
    std::string string_vector_count;

    getline(*inFile, string_vector_count);

    int strings_count = stoi(string_vector_count);

    for (int i = 0; i < strings_count; i++) {
        std::string string_item;
        getline(*inFile, string_item);

        inputVector.push_back(convertInputLineToIntVector(string_item));
    }

    return inputVector;
}

int getInputLines(std::string inputFileName, int &vectorSize, std::vector<std::vector<int>>& queries)
{
    std::string string_count_size;
    std::ifstream inFile(inputFileName);

    if (!inFile.is_open())
    {
        std::cerr << "Can't open " << inputFileName << " for input.\n";
        std::cout << "Can't open " << inputFileName << " for input.\n";
        return EXIT_FAILURE;
    }

    getline(inFile, string_count_size);
    vectorSize = stoi(string_count_size);

    queries = getIntVectors(&inFile);

    return EXIT_SUCCESS;
}

void getTestCountAndFirstTestCase(int& testCount, int& firstTestCase)
{
    do {
        std::cout << "How many test cases do you want to run?";
        std::cin >> testCount;
        if (testCount < 0 || testCount > MAX_TEST_CASE)
        {
            std::cerr << "The number of test cases must be greater > 0 and less than " << " " << MAX_TEST_CASE << "\n";
        }
    } while (testCount < 0 || testCount > MAX_TEST_CASE);

    if (testCount < MAX_TEST_CASE)
    {
        bool hasErrors = true;
        do {
            std::cout << "What test case file do you want to start with?";
            std::cin >> firstTestCase;
            if (firstTestCase < 0 || firstTestCase > MAX_TEST_CASE)
            {
                std::cerr << "The first test cases must be greater > 0 and less than " << " " << MAX_TEST_CASE << "\n";
                hasErrors = true;
            }
            else
            {
                hasErrors = false;
            }
            if (!hasErrors && testCount + firstTestCase > MAX_TEST_CASE)
            {
                std::cerr << "The first test cases and the test count must be less than or equal to " << MAX_TEST_CASE << "\n";
                hasErrors = true;
            }
        } while (hasErrors);
    }
    else
    {
        firstTestCase = 1;
    }
}

/*
 * Begin HackerRank Solution
*/
constexpr int IDX_FIRST_LOCATION = 0;
constexpr int IDX_LAST_LOCATION = 1;
constexpr int IDX_VALUE_TO_ADD = 2;
constexpr int MAX_THREADS = 16;

unsigned long mergeAndFindMax(std::vector<unsigned long> maxValues, std::vector<std::vector<unsigned long>> calculatedValues, const size_t executionCount)
{
    unsigned long maximumValue = 0;

    for (size_t i = 0; i < MAX_THREADS; i++)
    {
        std::vector<unsigned long>::iterator cvi = calculatedValues[i].begin();
        std::vector<unsigned long>::iterator cvEnd = calculatedValues[i].end();
        std::vector<unsigned long>::iterator mvi = maxValues.begin();
        for ( ; mvi < maxValues.end() && cvi < cvEnd; mvi++, cvi++)
        {
            *mvi += *cvi;
            if (*mvi > maximumValue)
            {
                maximumValue = *mvi;
            }
        }
        if (i > executionCount)
        {
            break;
        }
    }

    return maximumValue;
}

unsigned long arrayManipulation(const int n, const std::vector<std::vector<int>> queries)
{
    std::vector<unsigned long> maximumValues(n, 0);
    std::vector<std::vector<unsigned long>> calculatedValues(MAX_THREADS);
    std::mutex m;
    for_each(calculatedValues.begin(), calculatedValues.end(), [maximumValues](std::vector<unsigned long>& cvi) {cvi = maximumValues; });
    int executionCount = 0;

//    TODO add parallel execution.
    // for_each(std::execution::parallel, queries.begin(), queries.end(),
    Concurrency::parallel_for_each(queries.begin(), queries.end(),
        [&m, &calculatedValues, &executionCount](std::vector<int> query)
        {
            std::lock_guard<std::mutex> guard(m);
            size_t startLoc = query[IDX_FIRST_LOCATION];
            size_t endLoc = query[IDX_LAST_LOCATION];
            const unsigned long valueToAdd = query[IDX_VALUE_TO_ADD];
            for_each(calculatedValues[executionCount % MAX_THREADS].begin() + (startLoc - 1),
                calculatedValues[executionCount % MAX_THREADS].begin() + endLoc,
                [valueToAdd](unsigned long& n) {n += valueToAdd; });
            executionCount++;
        });

    return mergeAndFindMax(maximumValues, calculatedValues, executionCount);
}

int executeAndTimeTestCases(int testCaseCount, int firstTestCase)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
 
    for (int i = 0; i < testCaseCount; i++)
    {
        std::string testFileName = "TestCase" + std::to_string(firstTestCase) + ".txt";

        int n = 0;
        std::vector<std::vector<int>> queries;

        std::cout << "Test Case " << firstTestCase << "\n";

        auto fileReadStartTime = high_resolution_clock::now();
        int exitStatus = getInputLines(testFileName, n, queries);
        if (exitStatus != EXIT_SUCCESS)
        {
            return exitStatus;
        }
        auto fileReadEndTime = high_resolution_clock::now();
        duration<double, std::milli> msReadTime = fileReadEndTime - fileReadStartTime;
        std::cout << "Test Case " << firstTestCase << " File read time " << msReadTime.count() << " milliseconds\n";

        auto executionStartTime = high_resolution_clock::now();
        unsigned long result = arrayManipulation(n, queries);
        auto executionEndTime = high_resolution_clock::now();
        duration<double, std::milli> msExecution = executionEndTime - executionStartTime;

        if (msExecution.count() > 1000.0)
        {
            std::cout << "Test Case " << firstTestCase << " result is " << result << " Execution time " << msExecution.count() / 1000.0 << " Seconds\n\n";
        }
        else
        {
            std::cout << "Test Case " << firstTestCase << " result is " << result << " Execution time " << msExecution.count() << " milliseconds\n\n";
        }
        firstTestCase++;
    }

    return EXIT_SUCCESS;
}

int main()
{
    int testCaseCount = 0;
    int firstTestCase = 0;
    getTestCountAndFirstTestCase(testCaseCount, firstTestCase);

    return executeAndTimeTestCases(testCaseCount, firstTestCase);
}
