#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <condition_variable>
#include <sys/neutrino.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <atomic>
#include <ncurses.h>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <devctl.h>
#include <sys/ioctl.h>
#include <vector>
#include <numeric>

using namespace std;

std::mutex Mymutex;
std::mutex coutMutex;
std::condition_variable cv;
bool pilotCommand = false;
bool isSmoke = false;
pthread_mutex_t sensor_values_mutex;
pthread_mutex_t console_mutex;
int tLamp, pLamp, fLamp, sLamp;
int fuelValue = 250;
int pressureValue = 25;
int temperatureValue = 500;
double elapsed_time;
bool boolPrintRates = false;

bool warningFuel = false;
bool fuelIgn = false;
bool warningTemp = false;
bool tempIgn = false;
bool warningPres = false;
bool presIgn = false;

std::vector<int> allFuel;
std::vector<int> allTemp;
std::vector<int> allPres;

volatile char userInputValue;

class FuelSensor
{
private:
    mutable int warningCount = 0;
    struct MyMessage
    {
        int type;
        double data;
    };

public:
    static void *ThreadEntryRead(void *arg)
    {
        FuelSensor *fuelSensor = static_cast<FuelSensor *>(arg);
        fuelSensor->readFuel();
        return nullptr;
    }
    double readFuel()
    {
        fuelValue -= std::rand() % 5 + 1;
        if (fuelValue <= 0)
        {
            fuelValue = 250;
        }
        fuelWithinRange();
        allFuel.push_back(fuelValue);
        return fuelValue;
    }

    bool fuelWithinRange()
    {
        if (fuelValue < 25)
        {
            incrementWarnings();
            return false;
        }
        else
        {
            warningCount = 0;
            return true;
        }
    }
    void incrementWarnings()
    {
        warningCount++;
    }

    int getWarningCounts()
    {
        return warningCount;
    }
};

class EngineSensor
{
private:
    mutable int warningCountPressure = 0;
    mutable int warningCountTemperature = 0;

public:
    static void *ThreadEntryReadP(void *arg)
    {
        EngineSensor *engineSensor = static_cast<EngineSensor *>(arg);
        engineSensor->readPressure();
        return nullptr;
    }
    double readPressure()
    {
        pressureValue += std::rand() % 21 - 10;
        pressureWithinRange();
        allPres.push_back(pressureValue);
        return pressureValue;
    }

    bool pressureWithinRange()
    {
        if (pressureValue < 20 || pressureValue > 40)
        {
            incrementWarningsPressure();
            return false;
        }
        else
        {
            warningCountPressure = 0;
            return true;
        }
    }
    void incrementWarningsPressure()
    {
        warningCountPressure++;
    }

    int getWarningCountsPressure()
    {
        return warningCountPressure;
    }
    static void *ThreadEntryReadT(void *arg)
    {
        EngineSensor *engineSensore = static_cast<EngineSensor *>(arg);
        engineSensore->readTemperature();
        return nullptr;
    }

    double readTemperature()
    {
        temperatureValue += std::rand() % 21 - 10;
        temperatureWithinRange();
        allTemp.push_back(temperatureValue);
        return temperatureValue;
    }

    bool temperatureWithinRange()
    {
        if (temperatureValue > 50 || temperatureValue < -10)
        {
            incrementWarningsTemperature();
            return false;
        }
        else
        {
            warningCountTemperature = 0;
            return true;
        }
    }
    void incrementWarningsTemperature()
    {

        warningCountTemperature++;
    }

    int getWarningCountsTemperature()
    {
        return warningCountTemperature;
    }
};
class SmokeDetector
{
private:
    vector<std::string> warningMessages = {"none", "none"};

public:
    static void *threadEntrySmoke1(void *arg)
    {

        SmokeDetector *smokeDetector = static_cast<SmokeDetector *>(arg);
        smokeDetector->initSmoke1();
        return nullptr;
    }
    void initSmoke1()
    {

        warningMessages[0] = "WARNING ...... LEFT ENGINE IS ON FIRE ! ...... WARNING";

        isSmoke = true;
    }
    static void *threadEntrySmoke2(void *arg)
    {

        SmokeDetector *smokeDetector = static_cast<SmokeDetector *>(arg);
        smokeDetector->initSmoke2();
        return nullptr;
    }

    void initSmoke2()
    {
        warningMessages[1] = "WARNING ...... RIGHT ENGINE IS ON FIRE ! ...... WARNING";

        isSmoke = true;
    }
    static void *threadEntryClear1(void *arg)
    {

        SmokeDetector *smokeDetector = static_cast<SmokeDetector *>(arg);
        smokeDetector->clearSmoke1();
        return nullptr;
    }

    void clearSmoke1()
    {
        warningMessages[0] = "none";
        if (warningMessages[1] == "none")
        {
            isSmoke = false;
        }
    }
    static void *threadEntryClear2(void *arg)
    {

        SmokeDetector *smokeDetector = static_cast<SmokeDetector *>(arg);
        smokeDetector->clearSmoke2();
        return nullptr;
    }

    void clearSmoke2()
    {
        warningMessages[1] = "none";
        if (warningMessages[0] == "none")
        {
            isSmoke = false;
        }
    }

    vector<std::string> returnWarningMessages()
    {

        return warningMessages;
    }
};

class Timer
{
public:
    Timer() : start_time(std::chrono::high_resolution_clock::now()) {}

    void reset()
    {
        start_time = std::chrono::high_resolution_clock::now();
    }

    double elapsed() const
    {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return duration.count() * 1e-6;
    }

private:
    std::chrono::high_resolution_clock::time_point start_time;
};

class Dials
{
private:
    string printMessage;
    double fuelRate;
    double tempRate;
    double presRate;
    Timer timer;

public:
    static void *threadGetDials(void *arg)
    {
        Dials *dials = static_cast<Dials *>(arg);
        dials->printDials(pressureValue, temperatureValue, fuelValue);
        return nullptr;
    }
    void printDials(int pressureValue, int temperatureValue, int fuelValue)
    {
        printMessage = "Fuel level : " + std::to_string(fuelValue) +
                       "L,Temperature level : " + std::to_string(temperatureValue) +
                       "°C, Pressure level : " + std::to_string(pressureValue) + "Pa";
        cout << "\n\n\n";

        cout << "--------------------AIRCRAFT SENSOR VALUES--------------------\n";

        cout << printMessage << endl;

        if (boolPrintRates == true)
        {
            printRates();

            cout << "\n\n\n-----------------------AIRCRAFT RATE VALUES-----------------------\n";
            cout << "Fuel Rate : " << fuelRate << "L/s, "
                 << "Temperature Rate : " << tempRate << "°C/s, "
                 << "Pressure Rate : " << presRate << "Pa/s" << endl;
        }
    }

    void printRates()
    {
        elapsed_time = timer.elapsed();

        fuelRate = (allFuel.back() - allFuel.front()) / elapsed_time; // current value - initial value / time in s
        presRate = (allPres.back() - allPres.front()) / elapsed_time;
        tempRate = (allTemp.back() - allTemp.front()) / elapsed_time;
    }
};

class PilotCommand
{
private:
    string inputDemand;
    SmokeDetector smokeDetector;
    Dials dials;
    int flag = 0;
    string input;
    pthread_t initSmoke1;
    pthread_t initSmoke2;
    pthread_t clearSmoke1;
    pthread_t clearSmoke2;
    vector<std::string> smokeWarnings;
    pthread_t pilotRequest;
    Timer timer;
    pthread_t rates;
    double elapsed_time;

public:
    static void *threadEntryPoint(void *arg)
    {
        PilotCommand *pilotCommands = static_cast<PilotCommand *>(arg);
        pilotCommands->getCommands();
        return nullptr;
    }

    void getCommands()
    {
        elapsed_time = timer.elapsed();
        cout << "---------------------AIRCRAFT WARNINGS---------------------\n";
        pthread_mutex_lock(&console_mutex);
        smokeWarnings = smokeDetector.returnWarningMessages();

        if (smokeWarnings[0] != "none")
        {
            displayMessage(smokeWarnings[0]);
        }
        if (smokeWarnings[1] != "none")
        {
            displayMessage(smokeWarnings[1]);
        }
        if (warningFuel == true && !fuelIgn)
        {
            displayMessage("FUEL IN THE RED ... press x ignore");
        }
        if (warningPres == true && !presIgn)
        {
            displayMessage("PRESSURE IN THE RED ... press x ignore");
        }
        if (warningTemp == true && !tempIgn)
        {
            displayMessage("TEMPERATURE IN THE RED ... press x ignore");
        }

        std::cout << "\n\n\n";
        std::cout << "COMMAND SYSTEM\n";
        cout << "v. Get Rates\n";
        cout << "c. Clear Rates\n";
        std::cout << "1. Simulate Smoke1\n";
        std::cout << "2. Simulate Smoke2\n";
        std::cout << "a. clear Smoke 1\n";
        std::cout << "b. clear Smoke 2\n";

        // pthread_create(&pilotRequest, NULL, &UserInput::userInputThread, &userInput);
        // pthread_detach(pilotRequest);

        int fd = STDIN_FILENO;
        int bytesAvailable;
        int ret = devctl(fd, FIONREAD, &bytesAvailable, sizeof(bytesAvailable), NULL);

        if (bytesAvailable > 0)
        {
            std::cin >> input;
        }

        if (input == "v")
        {
            boolPrintRates = true;
        }
        else if (input == "1")
        {
            pthread_create(&initSmoke1, NULL, &SmokeDetector::threadEntrySmoke1, &smokeDetector);
            pthread_detach(initSmoke1);
        }
        else if (input == "2")
        {
            pthread_create(&initSmoke2, NULL, &SmokeDetector::threadEntrySmoke2, &smokeDetector);
            pthread_detach(initSmoke2);
        }
        else if (input == "a")
        {
            pthread_create(&clearSmoke1, NULL, &SmokeDetector::threadEntryClear1, &smokeDetector);
            pthread_detach(clearSmoke1);
        }
        else if (input == "b")
        {
            pthread_create(&clearSmoke2, NULL, &SmokeDetector::threadEntryClear2, &smokeDetector);
            pthread_detach(clearSmoke2);
        }
        else if (input == "x")
        {
            if (warningFuel == true)
            {
                warningFuel = false;
                fuelIgn = true;
            }
            if (warningTemp == true)
            {
                warningTemp = false;
                tempIgn = true;
            }
            if (warningPres == true)
            {
                warningPres = false;
                presIgn = true;
            }
        }
        else if (input == "c")
        {
            boolPrintRates = false;
        }

        input = "";

        pthread_mutex_unlock(&console_mutex);
    }

    void displayMessage(string message)
    {
        cout << message << endl;
    }
};

class Lamps
{
private:
    EngineSensor engineSensor;
    FuelSensor fuelSensor;
    string tLampMess, pLampMess, fLampMess, sLampMess = "";

public:
    static void *threadGetLamps(void *arg)
    {
        Lamps *lamps = static_cast<Lamps *>(arg);
        lamps->getTempLamp(tLamp, pLamp, fLamp, sLamp);
        return nullptr;
    }

    void getTempLamp(int tLamp, int pLamp, int fLamp, int sLamp)
    {

        if (tLamp >= 3)
        {

            tLampMess = "Temperature : RED";
            warningTemp = true;
        }
        else
        {
            tLampMess = "Temperature : GREEN";
        }
        if (pLamp >= 3)
        {
            warningPres = true;
            pLampMess = "Pressure : RED";
        }
        else
        {
            pLampMess = "Pressure : GREEN";
        }
        if (fLamp >= 3)
        {
            warningFuel = true;
            fLampMess = "Fuel : RED";
        }
        else
        {
            fLampMess = "Fuel : GREEN";
        }
        if (sLamp >= 1)
        {
            sLampMess = "Smoke : RED";
        }
        else
        {
            sLampMess = "Smoke : GREEN";
        }
        cout << "\n\n\n";

        cout << "---------------------AIRCRAFT LAMP SYSTEM---------------------\n";

        cout << fLampMess << ", " << tLampMess << ", " << pLampMess
             << ", " << sLampMess << endl;
    }
};

class AircraftSystem
{
private:
    EngineSensor engineSensor;
    FuelSensor fuelSensor;
    Dials readDials;
    Lamps printLamps;
    SmokeDetector smoke;
    pthread_t readFuel;
    pthread_t readTemperature;
    pthread_t readPressure;

    pthread_t lamps;

    pthread_t dials;

    int rcvid_c;
    struct MyMessage
    {
        int type;
        double data;
    };

    double receivedFuelValue;

public:
    static void *threadEntryPoint(void *arg)
    {

        AircraftSystem *aircraftSystem = static_cast<AircraftSystem *>(arg);
        aircraftSystem->performMonitoring();
        return nullptr;
    }

    void performMonitoring()
    {

        pthread_mutex_lock(&sensor_values_mutex);

        pthread_create(&readFuel, NULL, &FuelSensor::ThreadEntryRead, &fuelSensor);
        pthread_join(readFuel, NULL);

        pthread_create(&readPressure, NULL, &EngineSensor::ThreadEntryReadP, &engineSensor);
        pthread_join(readPressure, NULL);

        pthread_create(&readTemperature, NULL, &EngineSensor::ThreadEntryReadT, &engineSensor);
        pthread_join(readTemperature, NULL);

        pthread_mutex_unlock(&sensor_values_mutex);

        tLamp = engineSensor.getWarningCountsTemperature();
        pLamp = engineSensor.getWarningCountsPressure();
        fLamp = fuelSensor.getWarningCounts();

        sLamp = 0;

        if (isSmoke == true)
        {
            sLamp = 1;
        }

        pthread_mutex_lock(&console_mutex);
        pthread_create(&dials, NULL, &Dials::threadGetDials, &readDials);
        pthread_join(dials, NULL);

        pthread_create(&lamps, NULL, &Lamps::threadGetLamps, &printLamps);
        pthread_join(lamps, NULL);

        pthread_mutex_unlock(&console_mutex);
    }
};

int main()
{
    AircraftSystem aircraftSystem;
    PilotCommand pilotCommands;

    pthread_t monitoringThread;
    pthread_t commandsThread;

    while (true)
    {

        pthread_create(&monitoringThread, NULL, &AircraftSystem::threadEntryPoint, &aircraftSystem);
        pthread_create(&commandsThread, NULL, &PilotCommand::threadEntryPoint, &pilotCommands);

        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (int i = 0; i < 50; ++i)
        {
            std::cout << std::endl;
        }

        pthread_join(monitoringThread, NULL);
        pthread_join(commandsThread, NULL);
    }
    return 0;
}
