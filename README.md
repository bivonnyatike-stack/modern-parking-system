# Modern Automated Parking System (DSA Task 1)
**Institution:** Multimedia University of Kenya  
**Course:** Data Structures and Algorithms  
**Lecturer:** Mr. P. Okodir  

---

## 1. System Architecture & Modules

The system is structured into five core modules:
1. **Module 1: Visual Display & Slot Monitoring** - Real-time monitoring of available, occupied, and total parking slots via a dynamic web interface.
2. **Module 2: Vehicle Arrival & Barrier Control** - Records vehicle plate numbers on arrival and assigns available slots.
3. **Module 3: Time Computation & Fee Engine** - Auto-calculates duration spent and applies the designated tariff schedule.
4. **Module 4: Payment Verification & Barrier Release** - Simulates M-Pesa 4-digit PIN verification to open the exit barrier.
5. **Module 5: Persistence & Audit Logging** - Records transactions dynamically and logs all financial records to `parking_logs.txt`.

---

## 2. Algorithms (Section A)

### Algorithm 1: Vehicle Entry & Bay Allocation
1. **INPUT:** `plateNumber`
2. **IF** `availableSlots == 0` THEN
      Add `plateNumber` to `entryQueue` (FIFO Queue)
      RETURN "Parking Full. Added to Queue."
   **ENDIF**
3. **FOR EACH** `slot` IN `slotsArray` DO
      **IF** `slot.isOccupied == FALSE` THEN
         `slot.isOccupied = TRUE`
         `slot.vehiclePlate = plateNumber`
         `assignedSlot = slot.slotId`
         BREAK
      **ENDIF**
   **ENDFOR**
4. Store `record` {`plateNumber`, `assignedSlot`, `entryTime = current_timestamp()`} in `activeVehiclesMap`
5. `availableSlots = availableSlots - 1`
6. Trigger Entry Barrier Open.

### Algorithm 2: Duration & Parking Fee Calculation
1. **INPUT:** `plateNumber`
2. Retrieve `entryTime` from `activeVehiclesMap[plateNumber]`
3. `durationMinutes = (currentTime() - entryTime) / 60`
4. **EVALUATE TARIFF:**
      - **IF** `durationMinutes <= 30` THEN `fee = 0` (Free)
      - **ELSE IF** `durationMinutes <= 120` THEN `fee = 50`
      - **ELSE IF** `durationMinutes <= 240` THEN `fee = 100`
      - **ELSE IF** `durationMinutes <= 360` THEN `fee = 300`
      - **ELSE** `fee = 500`
   **ENDIF**
5. **RETURN** `durationMinutes` AND `fee`

---

## 3. Data Structures Used & Justifications (Section B)

1. **Array / Fixed List (`slots`):**
   * *Justification:* Represents the fixed capacity of physical parking bays ($O(1)$ access efficiency).
2. **Hash Map / Dictionary (`activeVehicles`):**
   * *Justification:* Stores currently parked vehicle records using `plateNumber` as key. Provides $O(1)$ lookup time for instant fee calculation upon exit.
3. **Queue / Linear List (`entryQueue`):**
   * *Justification:* Enforces First-In, First-Out (FIFO) ordering for incoming vehicles when capacity is reached.
4. **Dynamic Vector / Array List (`db_logs`):**
   * *Justification:* Stores transaction logs sequentially for real-time dynamic reporting on the front-end dashboard.

---

## 4. Dynamic Database Schema Design (Section C)

### Table 1: `ParkingSlots`
| Column | Data Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `slot_id` | INT | PRIMARY KEY | Unique identifier for parking bay |
| `is_occupied` | BOOLEAN | DEFAULT FALSE | Current status of the bay |
| `current_plate` | VARCHAR(15) | NULLABLE | Assigned vehicle plate number |

### Table 2: `ActiveParkings`
| Column | Data Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `plate_number` | VARCHAR(15) | PRIMARY KEY | Unique vehicle license plate |
| `slot_id` | INT | FOREIGN KEY | References `ParkingSlots(slot_id)` |
| `entry_timestamp` | DATETIME | NOT NULL | Exact wall-clock arrival time |

### Table 3: `PaymentLogs`
| Column | Data Type | Constraints | Description |
| :--- | :--- | :--- | :--- |
| `log_id` | INT | PRIMARY KEY, AUTO_INCREMENT | Unique transaction receipt ID |
| `plate_number` | VARCHAR(15) | NOT NULL | License plate of exiting vehicle |
| `slot_id` | INT | NOT NULL | Released parking bay ID |
| `duration_mins` | FLOAT | NOT NULL | Total elapsed parking duration |
| `fee_paid` | DECIMAL(10,2)| NOT NULL | Amount charged and verified |
| `exit_timestamp` | DATETIME | NOT NULL | Settlement timestamp |