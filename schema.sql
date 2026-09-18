-- Dynamic Database Schema for Automated Parking System

CREATE DATABASE IF NOT EXISTS parking_system;
USE parking_system;

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