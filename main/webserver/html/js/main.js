document.getElementById("current-year").textContent = new Date().getFullYear();

const loader = document.getElementById("loader");
const output = document.getElementById("output");

const showLoader = () => (loader.style.display = "block");
const hideLoader = () => (loader.style.display = "none");
const showOutput = (html) => (output.innerHTML = html);

const formatStatus = (data) => `
  <h3>Device Status</h3>
  <ul>
    <li><strong>Status :</strong> ${data.status}</li>
    <li><strong>IP :</strong> ${data.ip}</li>
    <li><strong>Uptime :</strong> ${data.uptime}</li>
    <li><strong>Free Heap :</strong> ${data.free_heap}</li>
  </ul>
`;

const formatWifiScan = (data) => {
  if (!Array.isArray(data.networks) || data.networks.length === 0) {
    return `<p>No networks found.</p>`;
  }

  return `
    <h3>Available Wi-Fi Networks</h3>
    <div class="wifi-list">
      ${data.networks
        .map(
          (ap, i) => `
        <div class="wifi-card">
          <h4>📶 ${ap.ssid || "(hidden SSID)"}</h4>
          <ul>
            <li><strong>RSSI :</strong> ${ap.rssi} dBm</li>
            <li><strong>Channel :</strong> ${ap.channel}</li>
            <li><strong>Auth :</strong> ${ap.authmode}</li>
            <li><strong>BSSID :</strong> ${ap.bssid}</li>
          </ul>
          <button class="connect-btn" data-ssid="${encodeURIComponent(
            ap.ssid
          )}">Connect</button>
        </div>
      `
        )
        .join("")}
    </div>
  `;
};

document.addEventListener("click", (e) => {
  if (e.target.classList.contains("connect-btn")) {
    const ssid = decodeURIComponent(e.target.dataset.ssid);

    const password = prompt(`Enter password for "${ssid}"`);
    if (password === null) return;

    fetch("/api/v1/wifi/connect", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ ssid, password }),
    })
      .then((res) => res.json())
      .then((data) => {
        if (data.success) {
          alert(`✅ Connected to ${ssid}`);
        } else {
          alert(`❌ Failed: ${data.message || "Unknown error"}`);
        }
      })
      .catch((err) => {
        alert("❌ Connection error: " + err.message);
      });
  }
});

const handleApi = (url, formatter) => {
  showLoader();
  showOutput("");
  fetch(url)
    .then((res) => res.json())
    .then((data) => {
      if (data.success === false) {
        throw new Error(data.message || "Unknown error");
      }
      showOutput(formatter(data));
    })
    .catch((err) => {
      showOutput(`<p style="color: var(--error-color);">⚠️ ${err.message}</p>`);
    })
    .finally(hideLoader);
};

document.getElementById("api-status-btn").onclick = () =>
  handleApi("/api/v1/status", formatStatus);

document.getElementById("wifi-scan-btn").onclick = () =>
  handleApi("/api/v1/wifi/scan", formatWifiScan);
