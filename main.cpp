#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <conio.h>
#include <mutex>
#include <condition_variable>

using namespace std;

std::mutex Mymutex;
std::condition_variable cv;
bool pilotCommand = false;
bool isSmoke = false;

class FuelSensor
{
private:
    mutable int fuelValue = 75;
    mutable int warningCount = 0;

public:
    double readFuel()
    {
        fuelValue -= std::rand() % 5 + 1;
        if (fuelValue <= 0)
        {
            fuelValue = 75;
        }
        fuelWithinRange();
        return fuelValue;
    }

    bool fuelWithinRange()
    {
        if (fuelValue < 25)
        {
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
    mutable int pressureValue = 25;
    mutable int temperatureValue = 30;

    mutable int warningCountPressure = 0;
    mutable int warningCountTemperature = 0;

public:
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
    void getCommands()
    {

        cout << "\n\n\n";
        cout << "COMMAND SYSTEM\n";
        cout << "1. Smoke1\n";
        cout << "2. Smoke2\n";

        cout << "a. clear Smoke 1\n";
        cout << "b. clear Smoke 2\n";
        cout << "x. Acknowledge All Messages\n";

        if (_kbhit())
        {
            inputDemand = _getch();

            if (inputDemand == "1")
            {
                smokeDetector.initSmoke1();
            }
            else if (inputDemand == "2")
            {
                smokeDetector.initSmoke2();
            }
            else if (inputDemand == "a")
            {
                smokeDetector.clearSmoke1();
            }
            else if (inputDemand == "b")
            {
                smokeDetector.clearSmoke2();
            }
        }
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
            tLampMess = "\033[31mTemperature\033[0m";
        }
        else
        {
            tLampMess = "\033[32mTemperature\033[0m";
        }
        if (pLamp >= 3)
        {
            pLampMess = "\033[31mPressure\033[0m";
        }
        else
        {
            pLampMess = "\033[32mPressure\033[0m";
        }
        if (fLamp >= 3)
        {
            fLampMess = "\033[31mFuel\033[0m";
        }
        else
        {
            fLampMess = "\033[32mFuel\033[0m";
        }
        if (sLamp >= 1)
        {
            sLampMess = "\033[31mSmoke\033[0m";
        }
        else
        {
            sLampMess = "\033[32mSmoke\033[0m";
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

public:
    void performMonitoring()
    {

        int tLamp, pLamp, fLamp, sLamp;

        double enginePressure = engineSensor.readPressure();
        double engineTemperature = engineSensor.readTemperature();
        double aircraftFuel = fuelSensor.readFuel();

        if (!enginePressure)
        {
            engineSensor.incrementWarningsPressure();
        }
        else if (!engineTemperature)
        {
            engineSensor.incrementWarningsTemperature();
        }
        else if (!aircraftFuel)
        {
            fuelSensor.incrementWarnings();
        }

        tLamp = engineSensor.getWarningCountsTemperature();
        pLamp = engineSensor.getWarningCountsPressure();
        fLamp = fuelSensor.getWarningCounts();

        sLamp = 0;

        if (isSmoke == true)
        {
            sLamp = 1;
        }

        readDials.printDials(enginePressure, engineTemperature, aircraftFuel);

        lamps.getTempLamp(tLamp, pLamp, fLamp, sLamp);
    }
};

int main()
{
    AircraftSystem aircraftSystem;
    PilotCommand pilotCommands;

    vector<std::string> smokeWarnings;

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

        {
            std::unique_lock<std::mutex> lock(Mymutex);
            std::thread monitoringThread(&AircraftSystem::performMonitoring, &aircraftSystem);
            monitoringThread.join();
        }

        {
            std::unique_lock<std::mutex> lock(Mymutex);
            std::thread commandsThread(&PilotCommand::getCommands, &pilotCommands);
            commandsThread.join();
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "\033[2J\033[1;1H" << flush;
    }
    return 0;
}