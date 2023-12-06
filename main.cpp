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

using namespace std;

std::mutex Mymutex;
std::mutex coutMutex;
std::condition_variable cv;
bool pilotCommand = false;
bool isSmoke = false;
pthread_mutex_t sensor_values_mutex;
pthread_mutex_t console_mutex;

int fuelValue = 250;
int pressureValue = 25;
int temperatureValue = 500;

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
    void initSmoke1()
    {
        warningMessages[0] = "\033[33mWARNING ...... LEFT ENGINE IS ON FIRE ! ...... WARNING\033[0m";
        isSmoke = true;
    }

    void initSmoke2()
    {
        warningMessages[1] = "\033[33mWARNING ...... RIGHT ENGINE IS ON FIRE ! ...... WARNING\033[0m";
        isSmoke = true;
    }

    void clearSmoke1()
    {
        warningMessages[0] = "none";
        isSmoke = false;
    }
    void clearSmoke2()
    {
        warningMessages[1] = "none";
        isSmoke = false;
    }

    vector<std::string> returnWarningMessages()
    {

        return warningMessages;
    }
};

class Dials
{
private:
    string printMessage;

public:
    void printDials(int pressureValue, int temperatureValue, int fuelValue)
    {
        printMessage = "Fuel level : " + std::to_string(fuelValue) +
                       ",Temperature level : " + std::to_string(temperatureValue) +
                       ", Pressure level : " + std::to_string(pressureValue);
        cout << "\n\n\n";

        cout << "-------------------AIRCRAFT SENSOR VALUES-------------------\n";

        cout << printMessage << endl;
    }
};

class PilotCommand
{
private:
    string inputDemand;
    SmokeDetector smokeDetector;

public:
    static void *threadEntryPoint(void *arg)
    {
        PilotCommand *pilotCommands = static_cast<PilotCommand *>(arg);
        pilotCommands->getCommands();
        return nullptr;
    }
    bool key_pressed()
    {
        return std::cin.rdbuf()->in_avail() != 0;
    }

    // Function to get a key without blocking
    char get_key()
    {
        char ch;
        std::cin.get(ch);
        return ch;
    }

    void getCommands()
    {
        pthread_mutex_lock(&console_mutex);
        std::cout << "\n\n\n";
        std::cout << "COMMAND SYSTEM\n";
        std::cout << "1. Smoke1\n";
        std::cout << "2. Smoke2\n";
        std::cout << "a. clear Smoke 1\n";
        std::cout << "b. clear Smoke 2\n";
        std::cout << "x. Acknowledge All Messages\n";

        if (key_pressed())
        {
            char inputDemand = get_key();
            cout << inputDemand;

            switch (inputDemand)
            {
            case '1':
                smokeDetector.initSmoke1();
                break;
            case '2':
                smokeDetector.initSmoke2();
                break;
            case 'a':
                smokeDetector.clearSmoke1();
                break;
            case 'b':
                smokeDetector.clearSmoke2();
                break;
            default:
                // Handle other keys if needed
                break;
            }
        }
        pthread_mutex_unlock(&console_mutex);
    }
    vector<std::string> getWarningMessages()
    {
        return smokeDetector.returnWarningMessages();
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
    void getTempLamp(int tLamp, int pLamp, int fLamp, int sLamp)
    {

        if (tLamp >= 3)
        {
            tLampMess = "Temperature : RED";
        }
        else
        {
            tLampMess = "Temperature : GREEN";
        }
        if (pLamp >= 3)
        {
            pLampMess = "Pressure : RED";
        }
        else
        {
            pLampMess = "Pressure : GREEN";
        }
        if (fLamp >= 3)
        {
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

        cout << "------AIRCRAFT LAMP SYSTEM------\n";

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
    Lamps lamps;
    SmokeDetector smoke;
    pthread_t readFuel;
    pthread_t readTemperature;
    pthread_t readPressure;

    int rcvid_c;
    struct MyMessage
    {
        int type;
        double data;
    };
    int tLamp, pLamp, fLamp, sLamp;
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
        readDials.printDials(pressureValue, temperatureValue, fuelValue);
        lamps.getTempLamp(tLamp, pLamp, fLamp, sLamp);
        pthread_mutex_unlock(&console_mutex);
    }
};

int main()
{
    AircraftSystem aircraftSystem;
    PilotCommand pilotCommands;

    vector<std::string> smokeWarnings;

    pthread_t monitoringThread;
    pthread_t commandsThread;

    while (true)
    {
        smokeWarnings = pilotCommands.getWarningMessages();
        if (smokeWarnings[0] != "none")
        {
            pilotCommands.displayMessage(smokeWarnings[0]);
        }
        if (smokeWarnings[1] != "none")
        {
            pilotCommands.displayMessage(smokeWarnings[1]);
        }

        pthread_create(&monitoringThread, NULL, &AircraftSystem::threadEntryPoint, &aircraftSystem);
        pthread_create(&commandsThread, NULL, &PilotCommand::threadEntryPoint, &pilotCommands);

        std::this_thread::sleep_for(std::chrono::seconds(1));

        // std::cout << "\033[2J\033[1;1H" << std::flush;

        pthread_join(monitoringThread, NULL);
        pthread_join(commandsThread, NULL);
    }
    return 0;
}
