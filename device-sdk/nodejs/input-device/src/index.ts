/**
 * @file index.ts
 * @brief atCloud365 Input Device - Node.js SDK Example
 *
 * This example demonstrates how to:
 * 1. Authenticate with atCloud365 platform via HTTPS POST
 * 2. Connect to Socket.IO for real-time communication
 * 3. Read sensor data (simulated with random values)
 * 4. Send data to platform via "dev-data" event
 * 5. Send status updates via "dev-status" event
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
const SERVER_URL = process.env.SERVER_URL || "http://localhost";
const API_PATH = process.env.API_PATH || "/api/dev/io/";
const DEVICE_AUTH_URI = process.env.DEVICE_AUTH_URI;
const SENSOR_COUNT = parseInt(process.env.SENSOR_COUNT || "3");
const DATA_UPLOAD_INTERVAL =
  parseInt(process.env.DATA_UPLOAD_INTERVAL || "10") * 1000;
const STATUS_REPORT_INTERVAL =
  parseInt(process.env.STATUS_REPORT_INTERVAL || "60") * 1000;

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
// Global Variables
// ==================================================
let sensorIds: number[] = [];
let sensorValues: number[] = [];
let isStatusSent = true;
let uploadIntervalId: NodeJS.Timeout | null = null;

// Initialize sensor arrays
for (let i = 0; i < SENSOR_COUNT; i++) {
  sensorIds.push(0x0f1234 + i);
  sensorValues.push(getRandom(0, 100));
}

console.log("\n" + "=".repeat(50));
console.log("atCloud365 Input Device Example");
console.log("Node.js SDK Version");
console.log("=".repeat(50));
console.log(`Device SN: ${DEVICE_SN}`);
console.log(
  `Sensor IDs: ${sensorIds.map((id) => `0x${id.toString(16)}`).join(", ")}`,
);
console.log(`Sensor Count: ${SENSOR_COUNT}`);
console.log("=".repeat(50) + "\n");

// ==================================================
// Utility Functions
// ==================================================
function getRandom(min: number, max: number): number {
  return Math.floor(Math.random() * (max - min + 1)) + min;
}

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
// Data Upload Function
// ==================================================
function uploadData(socket: Socket) {
  if (!socket.connected) {
    console.log("⚠️ Socket not connected, skipping data upload");
    return;
  }

  isStatusSent = !isStatusSent;

  if (isStatusSent) {
    // Time for data upload
    for (let i = 0; i < sensorValues.length; i++) {
      sensorValues[i] = getRandom(0, 100);
    }

    const data = { content: sensorValues };
    socket.emit("dev-data", data);
    console.log("📤 tx data>", data);
  } else {
    socket.emit("dev-status", "status-ok");
    console.log("📤 tx status>", "status-ok");
  }
}

// ==================================================
// Command Handler
// ==================================================
function handleAppCmd(socket: Socket, data: any) {
  console.log("📥 (rx) app-cmd:", JSON.stringify(data, null, 2));

  if (data.operation?.customCmd) {
    const { customCmd, fieldIndex, fieldValue } = data.operation;

    if (customCmd === "sync") {
      console.log("🔄 Sync command received");
      uploadData(socket);
    } else if (customCmd === "reboot") {
      console.log("🔄 Reboot command received - exiting...");
      socket.emit("dev-status", "Rebooting");
      setTimeout(() => process.exit(0), 1000);
    } else {
      console.log("⚠️ Unknown custom command:", customCmd);
    }
  } else {
    if (
      data.operation?.fieldIndex !== undefined &&
      data.operation?.fieldValue !== undefined
    ) {
      console.log(`⚠️ Field update command (not applicable for input device)`);
    } else {
      console.log("⚠️ Unknown operation command:", data);
    }
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
  //const socketUrl = `${SERVER_URL}:${SERVER_PORT}`;
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

    // Send initial data
    uploadData(socket);

    // Start periodic upload
    if (uploadIntervalId) clearInterval(uploadIntervalId);
    uploadIntervalId = setInterval(
      () => uploadData(socket),
      DATA_UPLOAD_INTERVAL,
    );
  });

  // Event: connected (server confirmation)
  socket.on("connected", (data: any) => {
    console.log("✅ Server confirmed connection:", data);
  });

  // Event: disconnect
  socket.on("disconnect", (reason: string) => {
    console.log(`❌ Disconnected: ${reason}`);
    if (uploadIntervalId) {
      clearInterval(uploadIntervalId);
      uploadIntervalId = null;
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

  // Graceful shutdown
  process.on("SIGINT", () => {
    console.log("\n\n[SIGNAL] Received SIGINT, shutting down...");
    if (uploadIntervalId) clearInterval(uploadIntervalId);
    socket.emit("dev-status", "Shutting down");
    setTimeout(() => {
      socket.disconnect();
      process.exit(0);
    }, 500);
  });

  process.on("SIGTERM", () => {
    console.log("\n\n[SIGNAL] Received SIGTERM, shutting down...");
    if (uploadIntervalId) clearInterval(uploadIntervalId);
    socket.emit("dev-status", "Shutting down");
    setTimeout(() => {
      socket.disconnect();
      process.exit(0);
    }, 500);
  });
})();
