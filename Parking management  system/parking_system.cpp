/*
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <limits>

using namespace std;

// ===================== VEHICLE HIERARCHY =====================
class Vehicle {
protected:
    string plateNumber;
    string ownerName;
    string phoneNumber;
    string model;
    string type;
    time_t entryTime;

public:
    Vehicle(string plate, string owner, string phone, string mod, string t)
        : plateNumber(move(plate)), ownerName(move(owner)), 
          phoneNumber(move(phone)), model(move(mod)), type(move(t)) {
        entryTime = time(nullptr);
    }
    virtual ~Vehicle() = default;

    string getPlate() const { return plateNumber; }
    string getOwnerName() const { return ownerName; }
    string getPhoneNumber() const { return phoneNumber; }
    string getModel() const { return model; }
    string getType() const { return type; }
    time_t getEntryTime() const { return entryTime; }

    // Tiered Parking Fee Calculator
    virtual double calculateMMUFee(double hours) const {
        if (hours <= 0.5) return 0.0;          // First 30 mins free
        else if (hours <= 2.0) return 50.0;    // Up to 2 hours: Kshs 50
        else if (hours <= 4.0) return 100.0;   // Up to 4 hours: Kshs 100
        else if (hours <= 6.0) return 300.0;   // Up to 6 hours: Kshs 300
        else return 500.0;                     // Over 6 hours: Kshs 500
    }

    virtual void displayInfo() const {
        cout << "Type: " << type << " | Plate: " << plateNumber 
             << " | Driver: " << ownerName << " (" << phoneNumber << ") | Model: " << model;
    }
};

class Bike : public Vehicle {
public:
    Bike(string plate, string owner, string phone, string mod) 
        : Vehicle(move(plate), move(owner), move(phone), move(mod), "Bike") {}
};

class Car : public Vehicle {
public:
    Car(string plate, string owner, string phone, string mod) 
        : Vehicle(move(plate), move(owner), move(phone), move(mod), "Car") {}
};

class Truck : public Vehicle {
public:
    Truck(string plate, string owner, string phone, string mod) 
        : Vehicle(move(plate), move(owner), move(phone), move(mod), "Truck") {}
};

// ===================== TICKET MODEL =====================
class Ticket {
private:
    string ticketId;
    string vehiclePlate;
    string ownerName;
    string phoneNumber;
    int slotId;
    time_t entryTime;

public:
    Ticket(string id, string plate, string owner, string phone, int slot, time_t t)
        : ticketId(move(id)), vehiclePlate(move(plate)), ownerName(move(owner)),
          phoneNumber(move(phone)), slotId(slot), entryTime(t) {}

    void print() const {
        cout << "\n================ MMU PARKING TICKET ================\n";
        cout << " Ticket ID   : " << ticketId << "\n";
        cout << " Driver Name : " << ownerName << "\n";
        cout << " Phone No    : " << phoneNumber << "\n";
        cout << " Vehicle     : " << vehiclePlate << "\n";
        cout << " Slot Number : Slot " << slotId << "\n";
        char buf[64];
        struct tm* timeinfo = localtime(&entryTime);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
        cout << " Entry Time  : " << buf << "\n";
        cout << "====================================================\n\n";
    }
};

// ===================== PARKING SLOT MODEL =====================
class ParkingSlot {
private:
    int slotId;
    bool occupied;
    unique_ptr<Vehicle> vehicle;

public:
    explicit ParkingSlot(int id) : slotId(id), occupied(false), vehicle(nullptr) {}

    int getId() const { return slotId; }
    bool isOccupied() const { return occupied; }

    bool parkVehicle(unique_ptr<Vehicle> v) {
        if (occupied) return false;
        vehicle = move(v);
        occupied = true;
        return true;
    }

    unique_ptr<Vehicle> removeVehicle() {
        if (!occupied) return nullptr;
        unique_ptr<Vehicle> v = move(vehicle);
        occupied = false;
        return v;
    }

    const Vehicle* getVehicle() const { return vehicle.get(); }
};

// ===================== SYSTEM MANAGER (SINGLETON) =====================
class ParkingLotSystem {
private:
    vector<unique_ptr<ParkingSlot>> slots;
    unordered_map<string, int> plateToSlotMap;
    int nextTicketNumber;

    ParkingLotSystem(int totalSlots) : nextTicketNumber(1) {
        for (int i = 1; i <= totalSlots; ++i) {
            slots.push_back(make_unique<ParkingSlot>(i));
        }
    }

public:
    static ParkingLotSystem& getInstance(int totalSlots = 10) {
        static ParkingLotSystem instance(totalSlots);
        return instance;
    }

    void displayAvailableSlotsVisual() const {
        int available = 0;
        cout << "\n====================================================\n";
        cout << "          DRIVER VISUAL DISPLAY BOARD               \n";
        cout << "====================================================\n";
        for (const auto& slot : slots) {
            cout << "[ Slot " << setw(2) << setfill('0') << slot->getId() << ": ";
            if (slot->isOccupied()) {
                cout << "OCCUPIED ] ";
            } else {
                cout << "FREE     ] ";
                available++;
            }
            if (slot->getId() % 5 == 0) cout << "\n";
        }
        cout << "----------------------------------------------------\n";
        cout << " TOTAL AVAILABLE SLOTS: " << available << " / " << slots.size() << "\n";
        cout << "====================================================\n\n";
    }

    Ticket parkVehicle(unique_ptr<Vehicle> v) {
        string plate = v->getPlate();

        if (plateToSlotMap.find(plate) != plateToSlotMap.end()) {
            throw runtime_error("Vehicle with plate " + plate + " is already inside!");
        }

        int freeSlotId = -1;
        for (const auto& s : slots) {
            if (!s->isOccupied()) {
                freeSlotId = s->getId();
                break;
            }
        }

        if (freeSlotId == -1) throw runtime_error("Parking lot is currently full!");

        string owner = v->getOwnerName();
        string phone = v->getPhoneNumber();

        ParkingSlot* slot = slots[freeSlotId - 1].get();
        slot->parkVehicle(move(v));
        plateToSlotMap[plate] = freeSlotId;

        stringstream ss;
        ss << "MMU-T" << setfill('0') << setw(4) << nextTicketNumber++;
        return Ticket(ss.str(), plate, owner, phone, freeSlotId, slot->getVehicle()->getEntryTime());
    }

    double processCheckoutAndOpenBarrier(const string& plateNumber, double manualHoursOverride = -1.0) {
        if (plateToSlotMap.find(plateNumber) == plateToSlotMap.end()) {
            throw runtime_error("Vehicle plate not found in active parking records!");
        }

        int slotId = plateToSlotMap[plateNumber];
        ParkingSlot* slot = slots[slotId - 1].get();
        
        time_t entryT = slot->getVehicle()->getEntryTime();
        time_t exitT = time(nullptr);

        double hoursSpent;
        if (manualHoursOverride >= 0.0) {
            hoursSpent = manualHoursOverride;
        } else {
            hoursSpent = difftime(exitT, entryT) / 3600.0;
        }

        unique_ptr<Vehicle> v = slot->removeVehicle();
        plateToSlotMap.erase(plateNumber);

        double fee = v->calculateMMUFee(hoursSpent);

        cout << "\n=================== EXIT SUMMARY ===================\n";
        cout << " Driver Name     : " << v->getOwnerName() << "\n";
        cout << " Plate Number    : " << v->getPlate() << "\n";
        cout << " Time Parked     : " << fixed << setprecision(2) << hoursSpent << " hours\n";
        cout << " Total Fee Due   : Kshs. " << fixed << setprecision(2) << fee << "\n";
        cout << "----------------------------------------------------\n";
        cout << " Payment verified! Exit Barrier OPENING automatically...\n";
        cout << "====================================================\n\n";

        return fee;
    }
};

// Helper function to safely read integer input
int getIntInput() {
    int value;
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Please enter a valid number: ";
    }
    return value;
}

// ===================== MAIN DRIVER INTERFACE =====================
int main() {
    ParkingLotSystem& system = ParkingLotSystem::getInstance(10);
    int choice;

    while (true) {
        system.displayAvailableSlotsVisual();
        cout << "1. Vehicle Arrival (Check-In)\n";
        cout << "2. Vehicle Exit & Calculate Fee (Check-Out)\n";
        cout << "0. Exit System\n";
        cout << "Select option: ";
        choice = getIntInput();

        if (choice == 0) break;

        if (choice == 1) {
            cout << "\nEnter Vehicle Type (1-Bike, 2-Car, 3-Truck): ";
            int t = getIntInput();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Enter Driver Full Name: ";
            string owner; getline(cin, owner);

            cout << "Enter Driver Phone Number: ";
            string phone; getline(cin, phone);

            cout << "Enter Vehicle License Plate: ";
            string plate; getline(cin, plate);

            cout << "Enter Vehicle Model: ";
            string model; getline(cin, model);

            unique_ptr<Vehicle> v;
            if (t == 1) v = make_unique<Bike>(plate, owner, phone, model);
            else if (t == 3) v = make_unique<Truck>(plate, owner, phone, model);
            else v = make_unique<Car>(plate, owner, phone, model);

            try {
                Ticket ticket = system.parkVehicle(move(v));
                ticket.print();
            } catch (const exception& e) {
                cout << "\nError: " << e.what() << "\n";
            }
        } else if (choice == 2) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\nEnter Vehicle Plate for Checkout: ";
            string plate; getline(cin, plate);

            cout << "Simulate specific parked hours? (Enter -1 for real system time, or enter hours e.g. 2.5): ";
            double hours;
            while (!(cin >> hours)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid entry. Enter numeric hours or -1: ";
            }

            try {
                system.processCheckoutAndOpenBarrier(plate, hours);
            } catch (const exception& e) {
                cout << "\nError: " << e.what() << "\n";
            }
        }
    }
    return 0;
}*/

#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include <cmath>

using namespace std;

// ===================== VEHICLE HIERARCHY =====================
class Vehicle {
protected:
    string plateNumber;
    string ownerName;
    string phoneNumber;
    string model;
    string type;
    time_t entryTime;

public:
    Vehicle(string plate, string owner, string phone, string mod, string t)
        : plateNumber(move(plate)), ownerName(move(owner)), 
          phoneNumber(move(phone)), model(move(mod)), type(move(t)) {
        entryTime = time(nullptr);
    }
    virtual ~Vehicle() = default;

    string getPlate() const { return plateNumber; }
    string getOwnerName() const { return ownerName; }
    string getPhoneNumber() const { return phoneNumber; }
    string getModel() const { return model; }
    string getType() const { return type; }
    time_t getEntryTime() const { return entryTime; }

    // Dynamic Fee Calculation Engine (Based on tiered duration)
    virtual double calculateParkingFee(double hours) const {
        if (hours <= 0.5) return 0.0;           // Up to 30 mins: Free
        else if (hours <= 2.0) return 50.0;     // Up to 2 hours: Kshs. 50
        else if (hours <= 4.0) return 100.0;    // Up to 4 hours: Kshs. 100
        else if (hours <= 6.0) return 300.0;    // Up to 6 hours: Kshs. 300
        else return 500.0;                      // Over 6 hours: Kshs. 500
    }

    virtual void displayInfo() const {
        cout << "Type: " << type << " | Plate: " << plateNumber 
             << " | Driver: " << ownerName << " (" << phoneNumber << ") | Model: " << model;
    }
};

class Bike : public Vehicle {
public:
    Bike(string plate, string owner, string phone, string mod) 
        : Vehicle(move(plate), move(owner), move(phone), move(mod), "Bike") {}
};

class Car : public Vehicle {
public:
    Car(string plate, string owner, string phone, string mod) 
        : Vehicle(move(plate), move(owner), move(phone), move(mod), "Car") {}
};

class Truck : public Vehicle {
public:
    Truck(string plate, string owner, string phone, string mod) 
        : Vehicle(move(plate), move(owner), move(phone), move(mod), "Truck") {}
};

// ===================== TICKET & SLOT MODELS =====================
class Ticket {
private:
    string ticketId;
    string vehiclePlate;
    string ownerName;
    string phoneNumber;
    int slotId;
    time_t entryTime;

public:
    Ticket(string id, string plate, string owner, string phone, int slot, time_t t)
        : ticketId(move(id)), vehiclePlate(move(plate)), ownerName(move(owner)),
          phoneNumber(move(phone)), slotId(slot), entryTime(t) {}

    void print() const {
        cout << "\n================ AUTOMATED PARKING TICKET ================\n";
        cout << " Ticket ID   : " << ticketId << "\n";
        cout << " Driver Name : " << ownerName << "\n";
        cout << " Phone No    : " << phoneNumber << "\n";
        cout << " Vehicle     : " << vehiclePlate << "\n";
        cout << " Slot Number : Slot " << slotId << "\n";
        char buf[64];
        struct tm* timeinfo = localtime(&entryTime);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", timeinfo);
        cout << " Entry Time  : " << buf << "\n";
        cout << "=========================================================\n\n";
    }
};

class ParkingSlot {
private:
    int slotId;
    bool occupied;
    unique_ptr<Vehicle> vehicle;

public:
    explicit ParkingSlot(int id) : slotId(id), occupied(false), vehicle(nullptr) {}

    int getId() const { return slotId; }
    bool isOccupied() const { return occupied; }

    bool parkVehicle(unique_ptr<Vehicle> v) {
        if (occupied) return false;
        vehicle = move(v);
        occupied = true;
        return true;
    }

    unique_ptr<Vehicle> removeVehicle() {
        if (!occupied) return nullptr;
        unique_ptr<Vehicle> v = move(vehicle);
        occupied = false;
        return v;
    }

    const Vehicle* getVehicle() const { return vehicle.get(); }
};

// ===================== SYSTEM MANAGER (SINGLETON) =====================
class ParkingLotSystem {
private:
    vector<unique_ptr<ParkingSlot>> slots;
    unordered_map<string, int> plateToSlotMap;
    int nextTicketNumber;

    ParkingLotSystem(int totalSlots) : nextTicketNumber(1) {
        for (int i = 1; i <= totalSlots; ++i) {
            slots.push_back(make_unique<ParkingSlot>(i));
        }
    }

public:
    static ParkingLotSystem& getInstance(int totalSlots = 10) {
        static ParkingLotSystem instance(totalSlots);
        return instance;
    }

    void displayAvailableSlotsVisual() const {
        int available = 0;
        cout << "\n====================================================\n";
        cout << "          DRIVER VISUAL DISPLAY BOARD               \n";
        cout << "====================================================\n";
        for (const auto& slot : slots) {
            cout << "[ Slot " << setw(2) << setfill('0') << slot->getId() << ": ";
            if (slot->isOccupied()) {
                cout << "OCCUPIED ] ";
            } else {
                cout << "FREE     ] ";
                available++;
            }
            if (slot->getId() % 5 == 0) cout << "\n";
        }
        cout << "\n----------------------------------------------------\n";
        cout << " TOTAL AVAILABLE SLOTS: " << available << " / " << slots.size() << "\n";
        cout << "====================================================\n\n";
    }

    Ticket parkVehicle(unique_ptr<Vehicle> v) {
        string plate = v->getPlate();

        // Duplicate entry guard
        if (plateToSlotMap.find(plate) != plateToSlotMap.end()) {
            throw runtime_error("Vehicle with license plate " + plate + " is already inside!");
        }

        int freeSlotId = -1;
        for (const auto& s : slots) {
            if (!s->isOccupied()) {
                freeSlotId = s->getId();
                break;
            }
        }

        if (freeSlotId == -1) throw runtime_error("Parking lot is currently full!");

        string owner = v->getOwnerName();
        string phone = v->getPhoneNumber();

        ParkingSlot* slot = slots[freeSlotId - 1].get();
        slot->parkVehicle(move(v));

        plateToSlotMap[plate] = freeSlotId;

        stringstream ss;
        ss << "TKT-" << setfill('0') << setw(5) << nextTicketNumber++;
        return Ticket(ss.str(), plate, owner, phone, freeSlotId, slot->getVehicle()->getEntryTime());
    }

    double processCheckoutAndOpenBarrier(const string& plateNumber, double simulatedHoursOverride = -1.0) {
        if (plateToSlotMap.find(plateNumber) == plateToSlotMap.end()) {
            throw runtime_error("Vehicle plate not found in active parking records!");
        }

        int slotId = plateToSlotMap[plateNumber];
        ParkingSlot* slot = slots[slotId - 1].get();
        unique_ptr<Vehicle> v = slot->removeVehicle();
        plateToSlotMap.erase(plateNumber);

        // Calculate hours automatically via system clock or dynamic simulation override
        double hoursSpent = 0.0;
        if (simulatedHoursOverride >= 0.0) {
            hoursSpent = simulatedHoursOverride;
        } else {
            time_t now = time(nullptr);
            double seconds = difftime(now, v->getEntryTime());
            hoursSpent = seconds / 3600.0;
        }

        double fee = v->calculateParkingFee(hoursSpent);

        cout << "\n=================== EXIT SUMMARY ===================\n";
        cout << " Driver Name     : " << v->getOwnerName() << "\n";
        cout << " Plate Number    : " << v->getPlate() << "\n";
        cout << " Time Parked     : " << fixed << setprecision(2) << hoursSpent << " hours\n";
        cout << " Total Fee Due   : Kshs. " << fixed << setprecision(2) << fee << "\n";
        cout << "----------------------------------------------------\n";
        cout << " Payment verified! Exit Barrier OPENING automatically...\n";
        cout << "====================================================\n\n";

        return fee;
    }
};

// Clear input buffer helper
void clearInputStream() {
    cin.clear();
    cin.ignore(10000, '\n');
}

// ===================== MAIN INTERACTIVE DRIVER =====================
int main() {
    ParkingLotSystem& system = ParkingLotSystem::getInstance(10);
    int choice;

    while (true) {
        system.displayAvailableSlotsVisual();
        cout << "1. Vehicle Arrival (Check-In)\n";
        cout << "2. Vehicle Exit & Calculate Fee (Check-Out)\n";
        cout << "0. Exit System\n";
        cout << "Select option: ";

        if (!(cin >> choice)) {
            cout << "\nInvalid input! Please enter a valid numerical choice.\n";
            clearInputStream();
            continue;
        }

        if (choice == 0) break;

        if (choice == 1) {
            cout << "\nEnter Vehicle Type (1-Bike, 2-Car, 3-Truck): ";
            int t; 
            if (!(cin >> t) || t < 1 || t > 3) {
                cout << "Invalid vehicle type chosen!\n";
                clearInputStream();
                continue;
            }
            clearInputStream();

            cout << "Enter Driver Full Name: ";
            string owner; getline(cin, owner);

            cout << "Enter Driver Phone Number: ";
            string phone; getline(cin, phone);

            cout << "Enter Vehicle License Plate: ";
            string plate; getline(cin, plate);

            cout << "Enter Vehicle Model (e.g. Toyota Vitz): ";
            string model; getline(cin, model);

            unique_ptr<Vehicle> v;
            if (t == 1) v = make_unique<Bike>(plate, owner, phone, model);
            else if (t == 2) v = make_unique<Car>(plate, owner, phone, model);
            else v = make_unique<Truck>(plate, owner, phone, model);

            try {
                Ticket ticket = system.parkVehicle(move(v));
                ticket.print();
            } catch (const exception& e) {
                cout << "\nError: " << e.what() << "\n";
            }
        } else if (choice == 2) {
            clearInputStream();
            cout << "\nEnter Vehicle Plate for Checkout: ";
            string plate; getline(cin, plate);

            cout << "Use manual simulated time for testing? (y/n): ";
            char simChoice; cin >> simChoice;
            
            double simHours = -1.0;
            if (simChoice == 'y' || simChoice == 'Y') {
                cout << "Enter simulated hours spent (e.g., 0.4, 1.5, 5.0): ";
                cin >> simHours;
            }

            try {
                system.processCheckoutAndOpenBarrier(plate, simHours);
            } catch (const exception& e) {
                cout << "\nError: " << e.what() << "\n";
            }
        }
    }
    return 0;
}


/*
Module 1: Visual Display Module

ALGORITHM DisplayAvailableSlots(SlotsVector)
    INPUT: Array/Vector of ParkingSlot objects
    OUTPUT: Formatted visual board on LED/Monitor

    Initialize availableCount = 0
    PRINT "=== DRIVER VISUAL DISPLAY BOARD ==="
    
    FOR EACH slot IN SlotsVector DO
        IF slot.isOccupied IS FALSE THEN
            PRINT "[ Slot ID: FREE ]"
            availableCount = availableCount + 1
        ELSE
            PRINT "[ Slot ID: OCCUPIED ]"
        ENDIF
        
        IF (slot.id MOD 5 EQUALS 0) THEN
            PRINT Newline
        ENDIF
    ENDFOR
    
    PRINT "TOTAL AVAILABLE SLOTS: " + availableCount
END ALGORITHM
*/

/*
Module 2: Vehicle Check-In & Ticket Generation Module
ALGORITHM VehicleCheckIn(VehicleData)
    INPUT: Vehicle details (Plate, Owner, Phone, Model, Type)
    OUTPUT: Printed Ticket OR Error Message

    IF LookupTable Contains Key(VehicleData.Plate) THEN
        RAISE ERROR "Vehicle already registered inside lot"
        RETURN
    ENDIF

    targetSlotId = -1
    FOR EACH slot IN ParkingSlots DO
        IF slot.isOccupied IS FALSE THEN
            targetSlotId = slot.getId()
            BREAK
        ENDIF
    ENDFOR

    IF targetSlotId EQUALS -1 THEN
        RAISE ERROR "Parking Lot Full"
        RETURN
    ENDIF

    NewVehicle = Instantiate Vehicle(Type, Plate, Owner, Phone, Model, CurrentTime())
    ParkingSlots[targetSlotId].Park(NewVehicle)
    LookupTable.Insert(Key = VehicleData.Plate, Value = targetSlotId)

    TicketCode = GenerateUniqueTicketID()
    Ticket = CreateTicket(TicketCode, VehicleData, targetSlotId, CurrentTime())
    
    CALL PrintTicket(Ticket)
END ALGORITHM
*/

/*
Module 3: Vehicle Check-Out & Barrier Automation Module
ALGORITHM VehicleCheckOut(PlateNumber, SimulatedHours)
    INPUT: License Plate Number, Optional Simulated Hours
    OUTPUT: Financial Breakdown & Barrier Trigger Signals

    IF LookupTable Does NOT Contain Key(PlateNumber) THEN
        RAISE ERROR "Plate Number Not Found"
        RETURN
    ENDIF

    slotId = LookupTable.Get(PlateNumber)
    Vehicle = ParkingSlots[slotId].RemoveVehicle()
    LookupTable.Remove(PlateNumber)

    IF SimulatedHours IS PROVIDED THEN
        hoursSpent = SimulatedHours
    ELSE
        currentTime = CurrentTime()
        secondsElapsed = DifferenceInSeconds(currentTime, Vehicle.EntryTime)
        hoursSpent = secondsElapsed / 3600.0
    ENDIF

    // Fee Computation Logic
    IF hoursSpent <= 0.5 THEN
        fee = 0.0
    ELSE IF hoursSpent <= 2.0 THEN
        fee = 50.0
    ELSE IF hoursSpent <= 4.0 THEN
        fee = 100.0
    ELSE IF hoursSpent <= 6.0 THEN
        fee = 300.0
    ELSE
        fee = 500.0
    ENDIF

    PRINT "Exit Receipt: " + Vehicle.Plate + " | Amount: Kshs " + fee
    
    // Trigger Hardware Relay
    CALL TriggerBarrierReleaseSignal()
    PRINT "Exit Barrier Opened Successfully."
END ALGORITHM
*/