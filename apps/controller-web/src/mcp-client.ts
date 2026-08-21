// Thin wrapper around the WebSocket control server implemented in
// main/boards/yana-wheelbot/websocket_control_server.cc (copied from
// main/boards/otto-robot, board-agnostic). Speaks the JSON-RPC-2.0-style
// envelope documented in main/boards/otto-robot/README.md's "WebSocket
// 直连调试接口" section: {"jsonrpc":"2.0","method":"tools/call","params":
// {"name":...,"arguments":{...}},"id":N}. The server forwards this straight
// into McpServer::ParseMessage() -- see websocket_control_server.cc's
// HandleMessage() -- so this is the same protocol the cloud channel speaks,
// just over a local LAN WebSocket instead of MQTT/WebSocket-to-cloud.

export type ToolArguments = Record<string, string | number | boolean>;

type PendingCall = {
  resolve: (value: unknown) => void;
  reject: (reason: unknown) => void;
};

export type BroadcastListener = (payload: unknown) => void;

export class McpClient {
  private ws: WebSocket | null = null;
  private nextId = 1;
  private pending = new Map<number, PendingCall>();
  private broadcastListeners: BroadcastListener[] = [];
  private onStatusChange: (connected: boolean) => void;

  constructor(onStatusChange: (connected: boolean) => void) {
    this.onStatusChange = onStatusChange;
  }

  connect(deviceUrl: string) {
    this.ws = new WebSocket(deviceUrl);

    this.ws.onopen = () => {
      this.onStatusChange(true);
      this.send("initialize", {
        protocolVersion: "2024-11-05",
        capabilities: {},
      });
      this.send("tools/list", {});
    };

    this.ws.onclose = () => {
      this.onStatusChange(false);
    };

    this.ws.onerror = () => {
      this.onStatusChange(false);
    };

    this.ws.onmessage = (event) => {
      this.handleMessage(event.data);
    };
  }

  disconnect() {
    this.ws?.close();
    this.ws = null;
  }

  get isConnected(): boolean {
    return this.ws !== null && this.ws.readyState === WebSocket.OPEN;
  }

  onBroadcast(listener: BroadcastListener) {
    this.broadcastListeners.push(listener);
  }

  callTool(name: string, args: ToolArguments = {}): Promise<unknown> {
    return this.send("tools/call", { name, arguments: args });
  }

  private send(method: string, params: unknown): Promise<unknown> {
    if (!this.isConnected) {
      return Promise.reject(new Error("Not connected"));
    }
    const id = this.nextId++;
    const envelope = { jsonrpc: "2.0", method, params, id };
    return new Promise((resolve, reject) => {
      this.pending.set(id, { resolve, reject });
      this.ws!.send(JSON.stringify(envelope));
    });
  }

  private handleMessage(raw: string) {
    let msg: any;
    try {
      msg = JSON.parse(raw);
    } catch {
      return;
    }

    if (typeof msg.id === "number" && this.pending.has(msg.id)) {
      const pending = this.pending.get(msg.id)!;
      this.pending.delete(msg.id);
      if (msg.error) {
        pending.reject(msg.error);
      } else {
        pending.resolve(msg.result);
      }
      return;
    }

    // Unsolicited frame (e.g. a voice-triggered MCP response mirrored via
    // Application::RegisterMcpBroadcastCallback in yana_wheelbot_board.cc) --
    // hand it to anyone listening so the UI can reflect state it didn't
    // itself request.
    for (const listener of this.broadcastListeners) {
      listener(msg);
    }
  }
}
