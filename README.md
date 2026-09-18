# Automated Parking Management System (Task One)

## Overview
This repository contains the architecture, algorithms, dynamic database design, and C++ implementation for an automated parking system designed for Kenyan parking facilities. The system automatically displays available slots, registers vehicle check-ins, tracks elapsed time, computes variable parking fees, and automates exit barrier releases.

---

## Part A: Algorithms for System Modules

### Module 1: Visual Display Module

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

### Module 2: Vehicle Check-In & Ticket Generation Module
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

### Module 3: Vehicle Check-Out & Barrier Automation Module
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

---

## Part B: Data Structures Analysis & Justification

| Data Structure | Primary Role in System | Selection Rationale & Complexity |
| :--- | :--- | :--- |
| **`std::vector` (Dynamic Array)** | Sequential storage of `ParkingSlot` objects. | Guarantees contiguous memory layout matching physical, numbered parking bays. Provides $O(1)$ direct index access via `slotId - 1`. |
| **`std::unordered_map` (Hash Table)** | Direct mapping of `License Plate` $\rightarrow$ `Slot ID`. | Provides average $O(1)$ time complexity for searching vehicles during exit, preventing costly $O(N)$ linear scans across physical slots. |
| **`std::unique_ptr` (Smart Pointers)** | Explicit memory management for polymorphism. | Ensures strict ownership semantics (one vehicle per physical slot) and automatic memory cleanup without manual `delete` calls, preventing memory leaks. |

---

## Part C: Dynamic Database Design

To transition from transient memory storage to a production system, the relational database structure below handles persistence, transactional integrity, and reporting.

```sql
-- 1. Parking Bay Configurations
CREATE TABLE ParkingSlots (
    slot_id INT PRIMARY KEY AUTO_INCREMENT,
    slot_number VARCHAR(10) NOT NULL UNIQUE,
    status ENUM('FREE', 'OCCUPIED', 'MAINTENANCE') DEFAULT 'FREE'
);

-- 2. Registered Vehicles Catalog
CREATE TABLE Vehicles (
    plate_number VARCHAR(15) PRIMARY KEY,
    owner_name VARCHAR(100) NOT NULL,
    phone_number VARCHAR(15) NOT NULL,
    vehicle_type ENUM('Bike', 'Car', 'Truck') NOT NULL,
    model VARCHAR(50)
);

-- 3. Dynamic Parking Transactions & Billing Records
CREATE TABLE ParkingTransactions (
    transaction_id INT PRIMARY KEY AUTO_INCREMENT,
    ticket_code VARCHAR(20) NOT NULL UNIQUE,
    plate_number VARCHAR(15) NOT NULL,
    slot_id INT NOT NULL,
    entry_time DATETIME NOT NULL,
    exit_time DATETIME NULL,
    duration_hours DECIMAL(5,2) NULL,
    amount_paid DECIMAL(10,2) DEFAULT 0.00,
    payment_status ENUM('PENDING', 'PAID') DEFAULT 'PENDING',
    FOREIGN KEY (plate_number) REFERENCES Vehicles(plate_number),
    FOREIGN KEY (slot_id) REFERENCES ParkingSlots(slot_id)
);