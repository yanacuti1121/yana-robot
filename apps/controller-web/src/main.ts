import { McpClient } from "./mcp-client";
import { Tools } from "./tools";

const $ = <T extends HTMLElement>(id: string) => document.getElementById(id) as T;

const statusEl = $<HTMLSpanElement>("status");
const client = new McpClient((connected) => {
  statusEl.textContent = connected ? "Connected" : "Disconnected";
  statusEl.className = `status ${connected ? "connected" : "disconnected"}`;
});

client.onBroadcast((payload) => {
  // Voice-triggered changes (e.g. someone says "turn on the lights") get
  // mirrored here too; for this simple controller we just log them --
  // extend this to refresh specific panel state if desired.
  console.log("[broadcast]", payload);
});

$<HTMLButtonElement>("connect-btn").addEventListener("click", () => {
  if (client.isConnected) {
    client.disconnect();
    return;
  }
  const url = $<HTMLInputElement>("device-url").value.trim();
  if (!url) return;
  client.connect(url);
});

function moveArgs() {
  return {
    duration_ms: Number($<HTMLInputElement>("duration-ms").value),
    speed: Number($<HTMLInputElement>("speed").value),
  };
}

$<HTMLButtonElement>("btn-forward").addEventListener("click", () =>
  client.callTool(Tools.moveForward, moveArgs()),
);
$<HTMLButtonElement>("btn-backward").addEventListener("click", () =>
  client.callTool(Tools.moveBackward, moveArgs()),
);
$<HTMLButtonElement>("btn-left").addEventListener("click", () =>
  client.callTool(Tools.turnLeft, moveArgs()),
);
$<HTMLButtonElement>("btn-right").addEventListener("click", () =>
  client.callTool(Tools.turnRight, moveArgs()),
);
$<HTMLButtonElement>("btn-stop").addEventListener("click", () => client.callTool(Tools.stop));

$<HTMLButtonElement>("motor-type-apply").addEventListener("click", () => {
  const type = $<HTMLSelectElement>("motor-type").value;
  client.callTool(Tools.setMotorType, { type });
});

$<HTMLButtonElement>("pins-apply").addEventListener("click", () => {
  client.callTool(Tools.setMotorPins, {
    in1: Number($<HTMLInputElement>("pin-in1").value),
    in2: Number($<HTMLInputElement>("pin-in2").value),
    in3: Number($<HTMLInputElement>("pin-in3").value),
    in4: Number($<HTMLInputElement>("pin-in4").value),
  });
});

$<HTMLButtonElement>("stop-pulse-apply").addEventListener("click", () => {
  client.callTool(Tools.setServoStopPulse, {
    microseconds: Number($<HTMLInputElement>("stop-pulse").value),
  });
});

$<HTMLInputElement>("rev-left").addEventListener("change", (e) => {
  client.callTool(Tools.setServoReverse, {
    side: "left",
    reversed: (e.target as HTMLInputElement).checked,
  });
});
$<HTMLInputElement>("rev-right").addEventListener("change", (e) => {
  client.callTool(Tools.setServoReverse, {
    side: "right",
    reversed: (e.target as HTMLInputElement).checked,
  });
});

$<HTMLButtonElement>("refresh-motor-config").addEventListener("click", async () => {
  const config = await client.callTool(Tools.getMotorConfig);
  $<HTMLPreElement>("motor-config-out").textContent = JSON.stringify(config, null, 2);
});

$<HTMLButtonElement>("cliff-apply").addEventListener("click", () => {
  client.callTool(Tools.cliffSetEnabled, {
    enabled: $<HTMLInputElement>("cliff-enabled").checked,
  });
  client.callTool(Tools.cliffSetThreshold, {
    threshold_mm: Number($<HTMLInputElement>("cliff-threshold").value),
  });
});

$<HTMLButtonElement>("cliff-test").addEventListener("click", async () => {
  const result = await client.callTool(Tools.cliffTestNow);
  $<HTMLSpanElement>("cliff-result").textContent = `${result} mm`;
});

document.querySelectorAll<HTMLButtonElement>("[data-mode]").forEach((btn) => {
  btn.addEventListener("click", () => {
    client.callTool(Tools.ledSetMode, { mode: btn.dataset.mode! });
  });
});

$<HTMLInputElement>("arm-angle").addEventListener("input", (e) => {
  client.callTool(Tools.armSetAngle, { angle: Number((e.target as HTMLInputElement).value) });
});
$<HTMLButtonElement>("arm-wave").addEventListener("click", () => client.callTool(Tools.armWave));
$<HTMLButtonElement>("arm-release").addEventListener("click", () =>
  client.callTool(Tools.armRelease),
);

$<HTMLInputElement>("neck-angle").addEventListener("input", (e) => {
  client.callTool(Tools.neckSetAngle, { angle: Number((e.target as HTMLInputElement).value) });
});
$<HTMLButtonElement>("neck-left").addEventListener("click", () =>
  client.callTool(Tools.neckTurn, { direction: "left" }),
);
$<HTMLButtonElement>("neck-center").addEventListener("click", () =>
  client.callTool(Tools.neckTurn, { direction: "center" }),
);
$<HTMLButtonElement>("neck-right").addEventListener("click", () =>
  client.callTool(Tools.neckTurn, { direction: "right" }),
);
$<HTMLButtonElement>("neck-release").addEventListener("click", () =>
  client.callTool(Tools.neckRelease),
);

$<HTMLButtonElement>("theme-apply").addEventListener("click", () => {
  client.callTool(Tools.screenSetTheme, { theme: $<HTMLSelectElement>("theme").value });
});

$<HTMLButtonElement>("orientation-apply").addEventListener("click", () => {
  client.callTool(Tools.screenSetOrientation, {
    orientation: $<HTMLSelectElement>("orientation").value,
  });
});
