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
}*/

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>

using namespace std;

// --- DYNAMIC RATE SYSTEM ---
struct TariffRate {
    double minHours;
    double maxHours;
    double rate;
};

class RateManager {
private:
    vector<TariffRate> rates;
    double vatPercentage;

public:
    RateManager() : vatPercentage(16.0) {
        // Default Rate Configuration (Can be changed dynamically by Admin)
        rates = {
            {0.0, 0.5, 0.0},     // First 30 mins free
            {0.5, 2.0, 50.0},    // Up to 2 hours: Ksh 50
            {2.0, 4.0, 100.0},   // Up to 4 hours: Ksh 100
            {4.0, 6.0, 300.0},   // Up to 6 hours: Ksh 300
            {6.0, 999.0, 500.0}  // Above 6 hours: Ksh 500
        };
    }

    void updateRates(const vector<TariffRate>& newRates) {
        rates = newRates;
        cout << "[ADMIN] Tariff rates updated successfully!\n";
    }

    void setVAT(double vat) {
        vatPercentage = vat;
        cout << "[ADMIN] VAT rate updated to " << vatPercentage << "%\n";
    }

    double getVAT() const { return vatPercentage; }

    double calculateFee(double hoursSpent) const {
        for (const auto& tier : rates) {
            if (hoursSpent >= tier.minHours && hoursSpent <= tier.maxHours) {
                return tier.rate;
            }
        }
        return 500.0; // Fallback
    }
};

// --- VEHICLE & PARKING BAY ENTITIES ---
class Vehicle {
public:
    string plateNumber;
    string ownerName;
    string phoneNumber;
    string model;
    string type;
    time_t entryTime;

    Vehicle(string plate, string owner, string phone, string model, string type)
        : plateNumber(plate), ownerName(owner), phoneNumber(phone), model(model), type(type) {
        entryTime = time(0);
    }
};

class ParkingSlot {
private:
    int id;
    bool occupied;
    shared_ptr<Vehicle> currentVehicle;

public:
    ParkingSlot(int slotId) : id(slotId), occupied(false), currentVehicle(nullptr) {}

    int getId() const { return id; }
    bool isOccupied() const { return occupied; }

    void park(shared_ptr<Vehicle> v) {
        currentVehicle = v;
        occupied = true;
    }

    shared_ptr<Vehicle> removeVehicle() {
        shared_ptr<Vehicle> v = currentVehicle;
        currentVehicle = nullptr;
        occupied = false;
        return v;
    }

    shared_ptr<Vehicle> getVehicle() const { return currentVehicle; }
};

// --- AUDITABLE FINANCIAL RECORD ---
struct AuditRecord {
    string ticketCode;
    string plateNumber;
    double durationHours;
    double totalAmount;
    double vatAmount;
    double netAmount;
    string paymentMethod; // M-Pesa, Card, Cash
    string timestamp;
};

// --- AUTOMATED PARKING SYSTEM ---
class ParkingSystem {
private:
    vector<ParkingSlot> slots;
    unordered_map<string, int> lookupTable; // Plate -> Slot ID
    vector<AuditRecord> auditTrail;
    RateManager rateManager;

public:
    ParkingSystem(int totalSlots) {
        for (int i = 1; i <= totalSlots; ++i) {
            slots.push_back(ParkingSlot(i));
        }
    }

    // 1. Live Slot Availability & Web Exporter
    void displayAvailableSlots() {
        int availableCount = 0;
        cout << "\n============================================\n";
        cout << "    LIVE DRIVER VISUAL DISPLAY BOARD        \n";
        cout << "============================================\n";
        
        for (const auto& slot : slots) {
            if (!slot.isOccupied()) {
                cout << "[ Slot " << slot.getId() << ": FREE ] ";
                availableCount++;
            } else {
                cout << "[ Slot " << slot.getId() << ": OCCUPIED ] ";
            }
            if (slot.getId() % 5 == 0) cout << "\n";
        }
        cout << "\n--------------------------------------------\n";
        cout << "TOTAL AVAILABLE BAYS: " << availableCount << " / " << slots.size() << "\n";
        cout << "============================================\n";

        // Export Live View for Web/Mobile API
        exportWebJsonView();
    }

    void exportWebJsonView() {
        ofstream jsonFile("live_slots.json");
        jsonFile << "{\n  \"total_slots\": " << slots.size() << ",\n  \"slots\": [\n";
        for (size_t i = 0; i < slots.size(); ++i) {
            jsonFile << "    { \"id\": " << slots[i].getId() 
                     << ", \"status\": \"" << (slots[i].isOccupied() ? "OCCUPIED" : "FREE") << "\" }";
            if (i < slots.size() - 1) jsonFile << ",";
            jsonFile << "\n";
        }
        jsonFile << "  ]\n}\n";
        jsonFile.close();
    }

    // 2. Vehicle Check-In
    void checkIn(string plate, string owner, string phone, string model, string type) {
        if (lookupTable.find(plate) != lookupTable.end()) {
            cout << "\n[ERROR] Vehicle with plate " << plate << " is already inside!\n";
            return;
        }

        int targetSlot = -1;
        for (auto& slot : slots) {
            if (!slot.isOccupied()) {
                targetSlot = slot.getId();
                break;
            }
        }

        if (targetSlot == -1) {
            cout << "\n[ERROR] Parking Lot is FULL!\n";
            return;
        }

        auto vehicle = make_shared<Vehicle>(plate, owner, phone, model, type);
        slots[targetSlot - 1].park(vehicle);
        lookupTable[plate] = targetSlot;

        cout << "\n[CHECK-IN SUCCESSFUL]";
        cout << "\nPlate: " << plate << " | Allocated Bay: Slot " << targetSlot << "\n";
    }

    // 3. Vehicle Check-Out & Payment Verification
    void checkOut(string plate, double simulatedHours, int paymentChoice) {
        if (lookupTable.find(plate) == lookupTable.end()) {
            cout << "\n[ERROR] Plate number " << plate << " not found in system!\n";
            return;
        }

        int slotId = lookupTable[plate];
        auto vehicle = slots[slotId - 1].removeVehicle();
        lookupTable.erase(plate);

        double fee = rateManager.calculateFee(simulatedHours);
        double vatRate = rateManager.getVAT();
        double vatAmount = fee * (vatRate / (100.0 + vatRate));
        double netAmount = fee - vatAmount;

        string paymentMethod = (paymentChoice == 1) ? "M-Pesa" : (paymentChoice == 2) ? "Card" : "Cash";

        cout << "\n============================================\n";
        cout << "            EXIT BILLING RECEIPT            \n";
        cout << "============================================\n";
        cout << "Vehicle Plate : " << vehicle->plateNumber << "\n";
        cout << "Duration      : " << simulatedHours << " Hours\n";
        cout << "Total Fee     : Kshs " << fixed << setprecision(2) << fee << "\n";
        cout << "   - Net Amount: Kshs " << netAmount << "\n";
        cout << "   - VAT (" << vatRate << "%): Kshs " << vatAmount << "\n";
        cout << "Payment Method: " << paymentMethod << "\n";
        cout << "--------------------------------------------\n";
        
        // M-Pesa / Card Processing Simulation
        if (paymentChoice == 1) {
            cout << "[M-PESA] STK Push sent to " << vehicle->phoneNumber << "...\n";
            cout << "[M-PESA] Payment Confirmed! Transaction ID: MP" << rand() % 899999 + 100000 << "\n";
        } else if (paymentChoice == 2) {
            cout << "[CARD] Processing POS transaction...\n";
            cout << "[CARD] Approved by Bank!\n";
        } else {
            cout << "[CASH] Cash received at register.\n";
        }

        // Barrier Control Trigger
        cout << "--------------------------------------------\n";
        cout << "[BARRIER] Payment Confirmed -> Opening Exit Barrier Relay.\n";
        cout << "============================================\n";

        // Record Audit
        time_t now = time(0);
        string timeStr = ctime(&now);
        timeStr.pop_back(); // Remove newline
        auditTrail.push_back({"TICK-" + to_string(rand() % 9000 + 1000), plate, simulatedHours, fee, vatAmount, netAmount, paymentMethod, timeStr});
    }

    // 4. Admin Rate Adjustment
    void updateTariff() {
        double vat;
        cout << "\nEnter New VAT Percentage (e.g., 16): ";
        cin >> vat;
        rateManager.setVAT(vat);
    }

    // 5. Auditable Financial & VAT Reconciliation Report
    void generateAuditReport() {
        double totalRev = 0, totalVat = 0, totalNet = 0;
        cout << "\n=========================================================================\n";
        cout << "              AUDITABLE FINANCIAL & VAT RECONCILIATION REPORT           \n";
        cout << "=========================================================================\n";
        cout << left << setw(12) << "Ticket" << setw(12) << "Plate" << setw(10) << "Method" 
             << setw(12) << "Total (Ksh)" << setw(12) << "VAT (Ksh)" << setw(12) << "Net (Ksh)" << "\n";
        cout << "-------------------------------------------------------------------------\n";

        for (const auto& rec : auditTrail) {
            cout << left << setw(12) << rec.ticketCode << setw(12) << rec.plateNumber 
                 << setw(10) << rec.paymentMethod << setw(12) << fixed << setprecision(2) << rec.totalAmount 
                 << setw(12) << rec.vatAmount << setw(12) << rec.netAmount << "\n";
            totalRev += rec.totalAmount;
            totalVat += rec.vatAmount;
            totalNet += rec.netAmount;
        }

        cout << "-------------------------------------------------------------------------\n";
        cout << "SUMMARY TOTALS:\n";
        cout << "Gross Revenue Collected: Kshs " << totalRev << "\n";
        cout << "Total VAT (Payable)    : Kshs " << totalVat << "\n";
        cout << "Net Revenue            : Kshs " << totalNet << "\n";
        cout << "=========================================================================\n";
    }
};

int main() {
    ParkingSystem system(10); // Initialized with 10 slots
    int choice;

    while (true) {
        cout << "\n=== AUTOMATED PARKING MANAGEMENT SYSTEM ===\n";
        cout << "1. Display Live Slot Board & Export Web View\n";
        cout << "2. Register Vehicle Check-In\n";
        cout << "3. Register Vehicle Check-Out & Process Payment\n";
        cout << "4. Admin: Dynamic Rate & VAT Adjustment\n";
        cout << "5. Admin: Generate Auditable Financial & VAT Report\n";
        cout << "6. Exit\n";
        cout << "Select Option: ";
        cin >> choice;

        if (choice == 1) {
            system.displayAvailableSlots();
        } else if (choice == 2) {
            string plate, owner, phone, model, type;
            cout << "Enter License Plate: "; cin >> plate;
            cout << "Enter Owner Name: "; cin >> owner;
            cout << "Enter Phone Number: "; cin >> phone;
            cout << "Enter Vehicle Model: "; cin >> model;
            cout << "Enter Vehicle Type (Car/Bike/Truck): "; cin >> type;
            system.checkIn(plate, owner, phone, model, type);
        } else if (choice == 3) {
            string plate;
            double hours;
            int payChoice;
            cout << "Enter License Plate: "; cin >> plate;
            cout << "Enter Hours Spent (Simulated): "; cin >> hours;
            cout << "Select Payment Method (1: M-Pesa, 2: Card, 3: Cash): "; cin >> payChoice;
            system.checkOut(plate, hours, payChoice);
        } else if (choice == 4) {
            system.updateTariff();
        } else if (choice == 5) {
            system.generateAuditReport();
        } else if (choice == 6) {
            cout << "\nExiting System. Goodbye!\n";
            break;
        } else {
            cout << "\nInvalid Selection. Retry.\n";
        }
    }

    return 0;
}