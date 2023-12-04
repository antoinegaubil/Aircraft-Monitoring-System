#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <conio.h>

using namespace std;

class FuelSensor
{
private:
    mutable int fuelValue = 250;
    mutable int warningCount = 0;

public:
    double readFuel()
    {
        fuelValue -= std::rand() % 26 + 10;
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

class SmokeDetector1
{
public:
    bool detectSmoke() const
    {
        return true;
    }
};

class SmokeDetector2
{
public:
    bool detectSmoke() const
    {
        return false;
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

        cout << "----------AIRCRAFT SENSOR VALUES----------\n";

        cout << printMessage << endl;
    }
};

class PilotCommand
{
private:
    string inputDemand;

public:
    void getCommands()
    {
        while (true)
        {
            cout << "\n\n\n";
            cout << "COMMAND SYSTEM\n";
            cout << "1. Smoke1\n";
            cout << "2. Smoke2\n";

            if (_kbhit())
            {
                inputDemand = _getch();

                if (inputDemand == "1")
                {
                    std::cout << "WARNING Smoke1\n";
                }
                else if (inputDemand == "2")
                {
                    std::cout << "WARNING Smoke2\n";
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
};

class Lamps
{
private:
    EngineSensor engineSensor;
    FuelSensor fuelSensor;
    string tLampMess, pLampMess, fLampMess = "";

public:
    void getTempLamp(int tLamp, int pLamp, int fLamp)
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
        cout << "\n\n\n";

        cout << "----------AIRCRAFT LAMP SYSTEM----------\n";

        cout << fLampMess << ", " << tLampMess << ", " << pLampMess << endl;
    }
};

class AircraftSystem
{
private:
    EngineSensor engineSensor;
    FuelSensor fuelSensor;
    SmokeDetector1 smokeDetector1;
    SmokeDetector2 smokeDetector2;
    Dials readDials;
    Lamps lamps;

public:
    void performMonitoring()
    {

        int tLamp, pLamp, fLamp;

        while (true)
        {

            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            double enginePressure = engineSensor.readPressure();
            double engineTemperature = engineSensor.readTemperature();
            double aircraftFuel = fuelSensor.readFuel();

            bool isSmokeDetected1 = smokeDetector1.detectSmoke();
            bool isSmokeDetected2 = smokeDetector2.detectSmoke();

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

            readDials.printDials(enginePressure, engineTemperature, aircraftFuel);

            lamps.getTempLamp(tLamp, pLamp, fLamp);

            // displayWarnings();
        }
    }
};

int main()
{
    srand(static_cast<unsigned>(time(nullptr)));

    AircraftSystem aircraftSystem;
    PilotCommand pilotCommands;

    thread monitoringThread(&AircraftSystem::performMonitoring, &aircraftSystem);

    thread commandsThread(&PilotCommand::getCommands, &pilotCommands);

    // commandsThread.join();
    monitoringThread.join();

    return 0;
}
