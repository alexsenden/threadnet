export interface GraphNode {
  name: string;
  id: string;
  rssi: number;
  state: "leader" | "router" | "device";
  parent: string | undefined;
}

export interface GraphLink {
  source: string;
  target: string;
}

export type GraphData = {
  nodes: GraphNode[];
};

export const data: GraphData = {
  nodes: [
    {
      name: "7334",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
      rssi: -50,
      state: "leader",
      parent: undefined,
    },
    {
      name: "1f0c",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:1f0c",
      rssi: -50,
      state: "router",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
    },
    {
      name: "4b7e",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:4b7e",
      rssi: -50,
      state: "device",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:1f0c",
    },
    {
      name: "9d0d",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:9d0d",
      rssi: -50,
      state: "device",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:1f0c",
    },
    {
      name: "6f23",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:6f23",
      rssi: -50,
      state: "device",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:1f0c",
    },
    {
      name: "2e81",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:2e81",
      rssi: -50,
      state: "router",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
    },
    {
      name: "ae55",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:ae55",
      rssi: -50,
      state: "device",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:2e81",
    },
    {
      name: "cc91",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:cc91",
      rssi: -50,
      state: "router",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:7334",
    },
    {
      name: "5bfc",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:5bfc",
      rssi: -50,
      state: "device",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:cc91",
    },
    {
      name: "38d9",
      id: "2001:0db8:85a3:0000:0000:8a2e:0370:38d9",
      rssi: -50,
      state: "device",
      parent: "2001:0db8:85a3:0000:0000:8a2e:0370:cc91",
    },
  ],
};
