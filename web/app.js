const logView = document.getElementById("logView");
const statusView = document.getElementById("statusView");
const frameView = document.getElementById("frameView");
document.getElementById("origin").textContent = window.location.origin;

function log(message) {
  const line = `[${new Date().toLocaleTimeString()}] ${message}`;
  logView.textContent = `${line}\n${logView.textContent}`.slice(0, 5000);
}

async function api(path, method = "GET", body = undefined) {
  const response = await fetch(path, {
    method,
    headers: { "Content-Type": "application/json" },
    body: body ? JSON.stringify(body) : undefined,
  });
  const payload = await response.json().catch(() => ({}));
  if (!response.ok || payload.ok === false) {
    throw new Error(payload.error || `HTTP ${response.status}`);
  }
  return payload.data;
}

document.getElementById("connectBtn").onclick = async () => {
  const host = document.getElementById("host").value.trim();
  const port = Number(document.getElementById("port").value || "9000");
  await api("/api/connection/connect", "POST", { host, port });
  log(`已连接设备 ${host}:${port}`);
};

document.getElementById("disconnectBtn").onclick = async () => {
  await api("/api/connection/disconnect", "POST", {});
  log("已断开设备");
};

document.getElementById("startScanBtn").onclick = async () => {
  const massStart = Number(document.getElementById("massStart").value || "1");
  const massEnd = Number(document.getElementById("massEnd").value || "200");
  await api("/api/scan/start", "POST", {
    mode: "full",
    massStart,
    massEnd,
    scanSpeed: 1000,
    scanTimeMs: 500,
    flybackTimeMs: 100,
    sampleRateHz: 1000,
    samplingPoints: 1024,
  });
  log("扫描已启动");
};

document.getElementById("stopScanBtn").onclick = async () => {
  await api("/api/scan/stop", "POST", {});
  log("扫描已停止");
};

document.getElementById("saveFrameBtn").onclick = async () => {
  const result = await api("/api/frame/save", "POST", {});
  log(`帧已保存: ${result.path}`);
};

document.getElementById("applyTuneBtn").onclick = async () => {
  await api("/api/tune/apply", "POST", {
    detector: "em",
    repellerVoltage: Number(document.getElementById("repeller").value || "12"),
    lens1Voltage: Number(document.getElementById("lens1").value || "6"),
    lens2Voltage: Number(document.getElementById("lens2").value || "10"),
  });
  log("调谐参数已应用");
};

function bindStream() {
  const stream = new EventSource("/api/stream");
  stream.addEventListener("status", (event) => {
    const data = JSON.parse(event.data);
    statusView.textContent = JSON.stringify(data, null, 2);
  });
  stream.addEventListener("frame", (event) => {
    const data = JSON.parse(event.data);
    frameView.textContent = JSON.stringify({
      timestamp: data.timestamp,
      points: data.masses?.length || 0,
      peaks: data.peaks?.length || 0,
      topPeak: data.peaks?.[0] || null,
    }, null, 2);
  });
  stream.addEventListener("error", (event) => {
    try {
      const data = JSON.parse(event.data);
      log(`设备错误: ${data.message}`);
    } catch (_) {
      log("实时通道错误");
    }
  });
  stream.onerror = () => {
    log("实时连接断开，3秒后重连");
    stream.close();
    setTimeout(bindStream, 3000);
  };
}

bindStream();
api("/api/status")
  .then((data) => {
    statusView.textContent = JSON.stringify(data.status || {}, null, 2);
    if (data.frame) {
      frameView.textContent = JSON.stringify(data.frame, null, 2);
    }
  })
  .catch((error) => log(`初始化失败: ${error.message}`));
