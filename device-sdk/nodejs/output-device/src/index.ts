/**
 * @file index.ts
 * @brief atCloud365 Output Device - Node.js SDK Example
 *
 * This example demonstrates how to:
 * 1. Authenticate with atCloud365 platform via HTTPS POST
 * 2. Connect to Socket.IO for real-time communication
 * 3. Receive control commands from platform
 * 4. Control outputs (simulated)
 * 5. Send status feedback to platform
 *
 * @author atCloud365
 * @date 2026-02-12
 */

import "dotenv/config";
import { io, Socket } from "socket.io-client";
import axios from "axios";

// ==================================================
// Environment Variables Validation
// ==================================================
const DEVICE_SN = process.env.DEVICE_SN;
const CLIENT_SECRET_KEY = process.env.CLIENT_SECRET_KEY;
const SERVER_URL = process.env.SERVER_URL || "https://atcloud365.com";
const API_PATH = process.env.API_PATH || "/api/dev/io/";
const DEVICE_AUTH_URI = process.env.DEVICE_AUTH_URI;
const SENSOR_COUNT = parseInt(process.env.SENSOR_COUNT || "3");
const STATUS_REPORT_INTERVAL =
  parseInt(process.env.STATUS_REPORT_INTERVAL || "60") * 1000;
const BLINK_INTERVAL = parseInt(process.env.BLINK_INTERVAL || "500");

// ==================================================
// Validate Required Configuration
// ==================================================
if (!DEVICE_SN) {
  console.error("❌ check .env : DEVICE_SN is missing && exiting...");
  process.exit(1);
}
if (!CLIENT_SECRET_KEY) {
  console.error("❌ check .env : CLIENT_SECRET_KEY is missing && exiting...");
  process.exit(1);
}
if (!DEVICE_AUTH_URI) {
  console.error("❌ check .env : DEVICE_AUTH_URI is missing && exiting...");
  process.exit(1);
}

// ==================================================
// Output Pin Class
// ==================================================
class OutputPin {
  index: number;
  name: string;
  state: boolean = false;
  blinkCount: number = 0;

  constructor(index: number, sensorId: number) {
    this.index = index;
    this.name = `Output-0x${sensorId.toString(16)}`;
  }
}

// ==================================================
// Global Variables
// ==================================================
let sensorIds: number[] = [];
let outputPins: OutputPin[] = [];
let statusReportIntervalId: NodeJS.Timeout | null = null;
let blinkIntervalId: NodeJS.Timeout | null = null;
let stateChanged: boolean = false;

// Initialize sensor and output arrays
for (let i = 0; i < SENSOR_COUNT; i++) {
  const sensorId = 0x0f1234 + i;
  sensorIds.push(sensorId);
  outputPins.push(new OutputPin(i, sensorId));
}

console.log("\n" + "=".repeat(50));
console.log("atCloud365 Output Device Example");
console.log("Node.js SDK Version");
console.log("=".repeat(50));
console.log(`Device SN: ${DEVICE_SN}`);
console.log(
  `Sensor IDs: ${sensorIds.map((id) => `0x${id.toString(16)}`).join(", ")}`,
);
console.log(`Output Count: ${SENSOR_COUNT}`);
console.log("=".repeat(50) + "\n");

// ==================================================
// HTTP Authentication
// ==================================================
async function fetchDeviceAuthToken(
  sn: string,
  uri: string,
  key: string,
): Promise<string | null> {
  console.log("[AUTH] Authenticating with atCloud365...");
  console.log(`[AUTH] Device SN: ${sn}`);
  console.log(`[AUTH] Sensor IDs: ${JSON.stringify(sensorIds)}`);

  try {
    const payload = {
      sn: sn,
      client_secret_key: key,
      sensorIds: sensorIds,
    };

    console.log(`[AUTH] URL: ${uri}`);
    console.log(`[AUTH] Payload: ${JSON.stringify(payload, null, 2)}`);

    const response = await axios.post(uri, payload, {
      headers: { "Content-Type": "application/json" },
      timeout: 30000,
    });

    if (response.status === 200 && response.data.token) {
      console.log("[AUTH] ✅ Authentication successful!");
      return response.data.token;
    } else {
      console.log("[AUTH] ❌ Invalid response:", response.data);
      return null;
    }
  } catch (err: any) {
    if (err.response) {
      console.error(
        `[AUTH] ❌ HTTP Error ${err.response.status}:`,
        err.response.data,
      );
    } else {
      console.error("[AUTH] ❌ Fetch Error:", err.message);
    }
    return null;
  }
}

// ==================================================
// Output Control Functions
// ==================================================
function setOutputState(index: number, state: boolean) {
  if (index >= 0 && index < outputPins.length) {
    outputPins[index].state = state;
    outputPins[index].blinkCount = 0; // Stop blinking
    stateChanged = true;

    const status = state ? "ON" : "OFF";
    console.log(`[OUTPUT] Pin ${index} (${outputPins[index].name}): ${status}`);
  }
}

function setAllOutputs(state: boolean) {
  for (let i = 0; i < outputPins.length; i++) {
    setOutputState(i, state);
  }
  const status = state ? "ON" : "OFF";
  console.log(`[OUTPUT] All pins set to: ${status}`);
}

function setOutputBlink(index: number, count: number) {
  if (index >= 0 && index < outputPins.length) {
    outputPins[index].blinkCount = count * 2; // *2 for on+off cycles
    console.log(
      `[OUTPUT] Pin ${index} (${outputPins[index].name}) blink started (count: ${count})`,
    );
  }
}

function handleBlinkLogic() {
  for (const pin of outputPins) {
    if (pin.blinkCount > 0) {
      pin.state = !pin.state;
      pin.blinkCount--;
      stateChanged = true;

      if (pin.blinkCount === 0) {
        pin.state = false; // Ensure OFF when done
        console.log(`[OUTPUT] Pin ${pin.index} blink completed`);
      }
    }
  }
}

function displayStatus() {
  console.log("\n" + "=".repeat(50));
  console.log("CURRENT OUTPUT STATUS");
  console.log("=".repeat(50));
  for (const pin of outputPins) {
    const status = pin.state ? "🟢 ON" : "⚫ OFF";
    const blink =
      pin.blinkCount > 0 ? ` (Blinking: ${Math.ceil(pin.blinkCount / 2)})` : "";
    console.log(`  Pin ${pin.index} (${pin.name}): ${status}${blink}`);
  }
  console.log("=".repeat(50) + "\n");
}

// ==================================================
// Data Emission Functions
// ==================================================
function emitDevData(socket: Socket) {
  if (!socket.connected) {
    console.log("⚠️ Socket not connected, skipping data upload");
    return;
  }

  try {
    const content = outputPins.map((pin) => (pin.state ? 1 : 0));
    const data = { content };

    socket.emit("dev-data", data);
    console.log("📤 tx data>", data);

    stateChanged = false;
  } catch (err) {
    console.error("[DATA] ❌ Error emitting data:", err);
  }
}

function emitDevStatus(socket: Socket, status: string) {
  if (!socket.connected) {
    console.log("⚠️ Socket not connected, skipping status upload");
    return;
  }

  try {
    socket.emit("dev-status", status);
    console.log("📤 tx status>", status);
  } catch (err) {
    console.error("[STATUS] ❌ Error emitting status:", err);
  }
}

// ==================================================
// Command Handler
// ==================================================
function handleAppCmd(socket: Socket, data: any) {
  console.log("📥 (rx) app-cmd:", JSON.stringify(data, null, 2));

  try {
    if (!data.operation) {
      console.log("⚠️ No operation field in command");
      return;
    }

    const { customCmd, fieldIndex, fieldValue } = data.operation;

    console.log(
      `[CMD] Parsed - cmd: ${customCmd || "none"}, index: ${fieldIndex}, value: ${fieldValue}`,
    );

    // Process command
    if (customCmd === "output") {
      if (
        fieldIndex >= 0 &&
        fieldIndex < outputPins.length &&
        fieldValue >= 0
      ) {
        setOutputState(fieldIndex, fieldValue > 0);
        displayStatus();
      }
    } else if (customCmd === "output-all") {
      if (fieldValue >= 0) {
        setAllOutputs(fieldValue > 0);
        displayStatus();
      }
    } else if (customCmd === "blinkLed") {
      if (fieldIndex >= 0 && fieldIndex < outputPins.length) {
        const count = fieldValue > 0 ? fieldValue : 5;
        setOutputBlink(fieldIndex, count);
      }
    } else if (customCmd === "sync") {
      console.log("🔄 Sync command received");
      emitDevData(socket);
    } else if (customCmd === "reboot") {
      console.log("🔄 Reboot command received - exiting...");
      emitDevStatus(socket, "Rebooting");
      setTimeout(() => process.exit(0), 1000);
    } else if (!customCmd) {
      // Simple index/value control
      if (
        fieldIndex >= 0 &&
        fieldIndex < outputPins.length &&
        fieldValue >= 0
      ) {
        setOutputState(fieldIndex, fieldValue > 0);
      }
    } else {
      console.log("⚠️ Unknown command:", customCmd);
    }
  } catch (err) {
    console.error("[CMD] ❌ Error processing command:", err);
  }
}

// ==================================================
// Main Logic
// ==================================================
(async () => {
  const sn = DEVICE_SN!;
  const token = await fetchDeviceAuthToken(
    sn,
    DEVICE_AUTH_URI!,
    CLIENT_SECRET_KEY!,
  );

  if (!token) {
    console.error("❌ Authentication failed && exiting...");
    process.exit(1);
  }

  // Socket.IO Connection
  const socketUrl = `${SERVER_URL}`;
  console.log("\n[SOCKET] Connecting to Socket.IO...");
  console.log(`[SOCKET] URL: ${socketUrl}`);
  console.log(`[SOCKET] Path: ${API_PATH}`);

  const socket: Socket = io(socketUrl, {
    path: API_PATH,
    transports: ["websocket", "polling"],
    auth: { token },
    query: {
      sn,
      clientType: "device",
      sensorIds: JSON.stringify(sensorIds),
      clientVersion: "V4",
    },
    reconnection: true,
    reconnectionAttempts: 50,
    reconnectionDelay: 5000,
  });

  // Event: connect
  socket.on("connect", () => {
    console.log(`\n✅ Connected to server. Socket ID: ${socket.id}`);

    // Send bootup status
    emitDevStatus(socket, "Bootup & Ready");

    // Send initial data
    emitDevData(socket);

    // Start periodic status report
    if (statusReportIntervalId) clearInterval(statusReportIntervalId);
    statusReportIntervalId = setInterval(() => {
      emitDevStatus(socket, "Status OK");
    }, STATUS_REPORT_INTERVAL);

    // Display initial status
    displayStatus();
  });

  // Event: connected (server confirmation)
  socket.on("connected", (data: any) => {
    console.log("✅ Server confirmed connection:", data);
  });

  // Event: disconnect
  socket.on("disconnect", (reason: string) => {
    console.log(`❌ Disconnected: ${reason}`);
    if (statusReportIntervalId) {
      clearInterval(statusReportIntervalId);
      statusReportIntervalId = null;
    }
  });

  // Event: app-cmd
  socket.on("app-cmd", (data: any) => {
    handleAppCmd(socket, data);
  });

  // Event: alarm-linkage
  socket.on("alarm-linkage", (data: any) => {
    console.log("📥 (rx) alarm-linkage:", data);
    handleAppCmd(socket, data);
  });

  // Event: connect_error
  socket.on("connect_error", (error: Error) => {
    console.error("🔴 Connection error:", error.message);
  });

  // Event: message-received
  socket.on("message-received", (data: any) => {
    console.log("✅ Message received acknowledgment:", data);
  });

  // Start blink logic timer
  blinkIntervalId = setInterval(() => {
    handleBlinkLogic();

    // Emit data if state changed
    if (stateChanged && socket.connected) {
      emitDevData(socket);
    }
  }, BLINK_INTERVAL);

  // Display status every 10 seconds
  setInterval(() => {
    displayStatus();
  }, 10000);

  // Graceful shutdown
  process.on("SIGINT", () => {
    console.log("\n\n[SIGNAL] Received SIGINT, shutting down...");
    if (statusReportIntervalId) clearInterval(statusReportIntervalId);
    if (blinkIntervalId) clearInterval(blinkIntervalId);
    emitDevStatus(socket, "Shutting down");
    setTimeout(() => {
      socket.disconnect();
      process.exit(0);
    }, 500);
  });

  process.on("SIGTERM", () => {
    console.log("\n\n[SIGNAL] Received SIGTERM, shutting down...");
    if (statusReportIntervalId) clearInterval(statusReportIntervalId);
    if (blinkIntervalId) clearInterval(blinkIntervalId);
    emitDevStatus(socket, "Shutting down");
    setTimeout(() => {
      socket.disconnect();
      process.exit(0);
    }, 500);
  });

  console.log("\n💡 Waiting for commands from platform...");
  console.log("   Press Ctrl+C to exit\n");
})();
