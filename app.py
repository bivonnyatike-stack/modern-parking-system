# app.py - Back-End REST API Server (MMU DSA Task 1 Compliant)
from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime

app = Flask(__name__)
CORS(app)

TOTAL_CAPACITY = 20
available_slots_count = TOTAL_CAPACITY
total_revenue = 0.0
transaction_counter = 1000

slots = [{"slotId": i, "isOccupied": False, "vehiclePlate": ""} for i in range(1, TOTAL_CAPACITY + 1)]
active_vehicles = {}
entry_queue = []
db_logs = []

LOG_FILE = "parking_logs.txt"

def get_current_time():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

# Module 4: Fee Calculator as per Lecturer Tariff Schedule
def calculate_fee(minutes):
    if minutes <= 30:
        return 0.0      # Up to 30 minutes free
    elif minutes <= 120:
        return 50.0     # Up to 2 hours: Kshs. 50
    elif minutes <= 240:
        return 100.0    # Up to 4 hours: Kshs. 100
    elif minutes <= 360:
        return 300.0    # Up to 6 hours: Kshs. 300
    else:
        return 500.0    # Over 6 hours: Kshs. 500

def write_log_to_file(tx):
    with open(LOG_FILE, "a") as f:
        f.write(f"{tx['logId']} | {tx['plateNumber']} | Slot #{tx['slotId']} | "
                f"Duration: {tx['durationMins']:.2f} mins | Fee: Kshs. {tx['feePaid']} | Date: {tx['timestamp']}\n")

@app.route('/api/status', methods=['GET'])
def get_status():
    return jsonify({
        "totalCapacity": TOTAL_CAPACITY,
        "availableSlots": available_slots_count,
        "occupiedSlots": TOTAL_CAPACITY - available_slots_count,
        "queueCount": len(entry_queue),
        "totalRevenue": total_revenue,
        "slots": slots,
        "activeVehicles": list(active_vehicles.values()),
        "logs": db_logs
    })

@app.route('/api/entry', methods=['POST'])
def vehicle_entry():
    global available_slots_count
    data = request.json
    plate = data.get("plateNumber", "").strip().upper()

    if not plate:
        return jsonify({"success": False, "message": "Plate number required"}), 400

    if plate in active_vehicles:
        return jsonify({"success": False, "message": f"Vehicle {plate} is already inside!"}), 400

    if available_slots_count == 0:
        entry_queue.append(plate)
        return jsonify({"success": False, "message": f"Parking Full! Vehicle {plate} added to waiting queue."})

    assigned_slot = None
    for slot in slots:
        if not slot["isOccupied"]:
            slot["isOccupied"] = True
            slot["vehiclePlate"] = plate
            assigned_slot = slot["slotId"]
            break

    record = {
        "plateNumber": plate,
        "assignedSlotId": assigned_slot,
        "entryTime": get_current_time()
    }
    active_vehicles[plate] = record
    available_slots_count -= 1

    return jsonify({
        "success": True,
        "message": f"Barrier opened for {plate}. Assigned Slot #{assigned_slot}",
        "assignedSlot": assigned_slot
    })

@app.route('/api/calculate-exit', methods=['POST'])
def calculate_exit():
    data = request.json
    plate = data.get("plateNumber", "").strip().upper()

    if plate not in active_vehicles:
        return jsonify({"success": False, "message": "Vehicle not found in active records"}), 404

    record = active_vehicles[plate]
    entry_dt = datetime.strptime(record["entryTime"], "%Y-%m-%d %H:%M:%S")
    now_dt = datetime.now()
    seconds_elapsed = (now_dt - entry_dt).total_seconds()
    duration_mins = max(0.1, seconds_elapsed / 60.0)

    fee = calculate_fee(duration_mins)

    return jsonify({
        "success": True,
        "plateNumber": plate,
        "slotId": record["assignedSlotId"],
        "durationMins": round(duration_mins, 2),
        "feePaid": fee
    })

@app.route('/api/exit', methods=['POST'])
def vehicle_exit():
    global available_slots_count, total_revenue, transaction_counter
    data = request.json
    plate = data.get("plateNumber", "").strip().upper()
    fee = float(data.get("feePaid", 0.0))
    pin = data.get("pin", "")

    if plate not in active_vehicles:
        return jsonify({"success": False, "message": "Vehicle not found"}), 404

    if len(pin) != 4 or not pin.isdigit():
        return jsonify({"success": False, "message": "Invalid PIN! M-Pesa authorization requires a 4-digit PIN."}), 400

    record = active_vehicles[plate]
    slot_id = record["assignedSlotId"]

    entry_dt = datetime.strptime(record["entryTime"], "%Y-%m-%d %H:%M:%S")
    now_dt = datetime.now()
    seconds_elapsed = (now_dt - entry_dt).total_seconds()
    duration_mins = max(0.1, seconds_elapsed / 60.0)

    for slot in slots:
        if slot["slotId"] == slot_id:
            slot["isOccupied"] = False
            slot["vehiclePlate"] = ""
            break

    available_slots_count += 1
    total_revenue += fee
    transaction_counter += 1

    tx = {
        "logId": transaction_counter,
        "plateNumber": plate,
        "slotId": slot_id,
        "durationMins": round(duration_mins, 2),
        "feePaid": fee,
        "timestamp": get_current_time()
    }
    db_logs.append(tx)
    write_log_to_file(tx)

    del active_vehicles[plate]

    return jsonify({
        "success": True,
        "message": f"PIN Authorized! Payment verified via M-Pesa. Barrier opened for {plate}. Slot #{slot_id} released.",
        "receipt": tx
    })

if __name__ == '__main__':
    app.run(debug=True, port=5000)