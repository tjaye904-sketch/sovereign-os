"use strict";
var __create = Object.create;
var __defProp = Object.defineProperty;
var __getOwnPropDesc = Object.getOwnPropertyDescriptor;
var __getOwnPropNames = Object.getOwnPropertyNames;
var __getProtoOf = Object.getPrototypeOf;
var __hasOwnProp = Object.prototype.hasOwnProperty;
var __copyProps = (to, from, except, desc) => {
  if (from && typeof from === "object" || typeof from === "function") {
    for (let key of __getOwnPropNames(from))
      if (!__hasOwnProp.call(to, key) && key !== except)
        __defProp(to, key, { get: () => from[key], enumerable: !(desc = __getOwnPropDesc(from, key)) || desc.enumerable });
  }
  return to;
};
var __toESM = (mod, isNodeMode, target) => (target = mod != null ? __create(__getProtoOf(mod)) : {}, __copyProps(
  // If the importer is in node compatibility mode or this is not an ESM
  // file that has been converted to a CommonJS file using a Babel-
  // compatible transform (i.e. "__esModule" has not been set), then set
  // "default" to the CommonJS "module.exports" for node compatibility.
  isNodeMode || !mod || !mod.__esModule ? __defProp(target, "default", { value: mod, enumerable: true }) : target,
  mod
));

// src/services/shellEnv.ts
var ORIGINAL_NODE_PATH_VAR = "AGENT_TERMINAL_ORIGINAL_NODE_PATH";
var TERMINAL_NAME = "xterm-256color";
function buildShellEnv(env) {
  const result = { ...env };
  delete result.ELECTRON_RUN_AS_NODE;
  delete result.NO_COLOR;
  result.TERM = TERMINAL_NAME;
  result.COLORTERM = "truecolor";
  const originalNodePath = result[ORIGINAL_NODE_PATH_VAR];
  delete result[ORIGINAL_NODE_PATH_VAR];
  if (originalNodePath) {
    result.NODE_PATH = originalNodePath;
  } else {
    delete result.NODE_PATH;
  }
  return result;
}

// src/services/bridgeTransport.ts
function receivePort(parentPort2) {
  return new Promise((resolve) => {
    parentPort2.once("message", (event) => resolve(event.ports[0]));
  });
}
function runAll(callbacks) {
  for (const callback of callbacks) {
    try {
      callback();
    } catch {
    }
  }
}
function createPortTransport(port, exitProcess = (code) => process.exit(code)) {
  const inputCallbacks = [];
  const controlCallbacks = [];
  const closeCallbacks = [];
  port.on("message", ({ data }) => {
    const msg = data;
    if (msg?.type === "input") {
      inputCallbacks.forEach((callback) => callback(msg.data));
    } else {
      controlCallbacks.forEach((callback) => callback(msg));
    }
  });
  port.on("close", () => {
    runAll(closeCallbacks);
    exitProcess(0);
  });
  port.start();
  const send = (msg) => port.postMessage(msg);
  return {
    onInput: (callback) => {
      inputCallbacks.push(callback);
    },
    onControl: (callback) => {
      controlCallbacks.push(callback);
    },
    onClose: (callback) => {
      closeCallbacks.push(callback);
    },
    sendData: (data) => send({ type: "data", data }),
    reply: send,
    fail: (message) => {
      send({ type: "error", message });
      send({ type: "exit", code: 1 });
    },
    // 立刻結束程序可能丟掉還在傳送中的訊息：只回報結束碼，外掛收到後會關閉 port，程序隨之結束
    exit: (code) => send({ type: "exit", code })
  };
}
function createStdioTransport() {
  const closeCallbacks = [];
  const close = () => {
    runAll(closeCallbacks);
    process.exit(0);
  };
  process.on("SIGTERM", close);
  process.on("SIGHUP", close);
  process.stdin.on("end", close);
  process.stdin.setEncoding("utf8");
  return {
    onInput: (callback) => {
      process.stdin.on("data", (data) => {
        if (data) callback(data);
      });
    },
    onControl: (callback) => {
      process.on("message", callback);
    },
    onClose: (callback) => {
      closeCallbacks.push(callback);
    },
    sendData: (data) => {
      process.stdout.write(data);
    },
    reply: (msg) => {
      process.send?.(msg);
    },
    // 錯誤一律以 "ERROR: " 開頭寫到 stderr，PtyService 會把這類訊息顯示在終端機畫面上
    // 訊息壓成單行，因為 PtyService 是逐行比對 "ERROR:" 前綴
    fail: (message) => {
      process.stderr.write(`ERROR: ${message.replace(/\s*\r?\n\s*/g, " ")}
`);
      process.exit(1);
    },
    // stdout 寫入 pipe 在 macOS 上是非同步的，直接 process.exit 會截斷最後的輸出：
    // 送出一個空寫入，它的 callback 會在先前所有資料寫出後才呼叫，再結束
    exit: (code) => {
      process.stdout.write("", () => process.exit(code));
    }
  };
}

// src/services/bridge.ts
function errorMessage(e) {
  return e instanceof Error ? e.message : String(e);
}
function isResizeMessage(msg) {
  return typeof msg === "object" && msg !== null && msg.type === "resize";
}
function isProcessQuery(msg) {
  return typeof msg === "object" && msg !== null && msg.type === "query-process";
}
async function run(transport) {
  const shell = process.argv[2];
  const cwd = process.argv[3];
  const cols = parseInt(process.argv[4] || "80");
  const rows = parseInt(process.argv[5] || "24");
  let pty;
  try {
    pty = await import("node-pty");
  } catch (e) {
    transport.fail(`Failed to load node-pty. Make sure it is installed in the plugin folder and built for your Node.js version. (${errorMessage(e)})`);
    return;
  }
  let ptyProcess;
  try {
    ptyProcess = pty.spawn(shell, [], {
      name: TERMINAL_NAME,
      cols,
      rows,
      cwd,
      // 移除 bridge 專用的環境變數並宣告色彩能力 (見 shellEnv.ts)
      env: buildShellEnv(process.env)
    });
  } catch (e) {
    transport.fail(`Failed to start shell "${shell}": ${errorMessage(e)}`);
    return;
  }
  transport.onInput((data) => ptyProcess.write(data));
  ptyProcess.onData((data) => transport.sendData(data));
  transport.onControl((msg) => {
    if (isResizeMessage(msg)) {
      ptyProcess.resize(msg.cols, msg.rows);
    } else if (isProcessQuery(msg)) {
      const name = process.platform === "win32" ? null : ptyProcess.process;
      transport.reply({ type: "process", id: msg.id, name });
    }
  });
  ptyProcess.onExit((event) => transport.exit(event.exitCode));
  transport.onClose(() => ptyProcess.kill());
}
var parentPort = process.parentPort;
if (parentPort) {
  void receivePort(parentPort).then((port) => run(createPortTransport(port)));
} else {
  void run(createStdioTransport());
}
