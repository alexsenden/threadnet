const AGENT_HOSTNAME = "http://192.168.4.1";

export enum NodeRole {
  disabled = 0,
  detached = 1,
  child = 2,
  router = 3,
  leader = 4,
}

export interface NodeEntry {
  name: string;
  id: number;
  parent: number | undefined;
  index: number;
  role: NodeRole;
  meshLocalEid: string;
  linkLocalIPv6: string;
  leaderId: number;
  leaderWeight: number;
  partitionId: number;
  rloc16: number;
  parentAvgRssi: number;
  parentLastRssi: number;
  lastUpdated: number;
  parentNextHop: number;
  parentLinkQualityIn: number;
  parentLinkQualityOut: number;
  parentRloc16: number;
  parentAge: number;
  parentRouterId: number;
  avgRttMillis: number;
  packetSuccessRate: number;
}

type AgentDataResponse =
  | {
      status: "success";
      result: NodeEntry[];
    }
  | {
      status: "error";
      error: string;
    };

async function getAgentData(): Promise<AgentDataResponse> {
  // Fetch the data from the border router without caching
  const result = await fetch(AGENT_HOSTNAME, { cache: "no-cache" });

  // If the status is not 200, return an error
  if (result.status != 200) {
    return {
      status: "error",
      error: result.statusText,
    };
  }

  // Parse the response from the border router
  const body = await result.text();
  const lines = body.trim().split("\n");
  const nodes: NodeEntry[] = [];

  // If there are less than 2 lines, return an empty result
  if (lines.length < 2) {
    return {
      status: "success",
      result: [],
    };
  }

  // Parse the header columns
  const headerCols = (lines.shift() as string).split(",");

  // DIY CSV parsing
  const idx_index = headerCols.indexOf("index");
  const idx_role = headerCols.indexOf("role");
  const idx_ot_mesh_local_eid = headerCols.indexOf("ot_mesh_local_eid");
  const idx_ot_link_local_ipv6 = headerCols.indexOf("ot_link_local_ipv6");
  const idx_ot_leader_id = headerCols.indexOf("ot_leader_id");
  const idx_ot_leader_weight = headerCols.indexOf("ot_leader_weight");
  const idx_ot_partition_id = headerCols.indexOf("ot_partition_id");
  const idx_ot_rloc_16 = headerCols.indexOf("ot_rloc_16");
  const idx_parent_avg_rssi = headerCols.indexOf("parent_avg_rssi");
  const idx_parent_last_rssi = headerCols.indexOf("parent_last_rssi");
  const idx_last_updated = headerCols.indexOf("last_updated");
  const idx_parent_next_hop = headerCols.indexOf("parent_next_hop");
  const idx_parent_link_quality_in = headerCols.indexOf(
    "parent_link_quality_in"
  );
  const idx_parent_link_quality_out = headerCols.indexOf(
    "parent_link_quality_out"
  );
  const idx_parent_rloc16 = headerCols.indexOf("parent_rloc16");
  const idx_parent_age = headerCols.indexOf("parent_age");
  const idx_parent_router_id = headerCols.indexOf("parent_router_id");
  const idx_avg_rtt_millis = headerCols.indexOf("avg_rtt_millis");
  const idx_packet_success_rate = headerCols.indexOf("packet_success_rate");

  lines.forEach((line) => {
    const parts = line.split(",");

    nodes.push({
      id: Number(parts[idx_ot_rloc_16]),
      name: parts[idx_ot_mesh_local_eid],
      parent: Number(parts[idx_parent_rloc16]) || undefined,
      index: Number(parts[idx_index]),
      role: Number(parts[idx_role]),
      meshLocalEid: parts[idx_ot_mesh_local_eid],
      linkLocalIPv6: parts[idx_ot_link_local_ipv6],
      leaderId: Number(parts[idx_ot_leader_id]),
      leaderWeight: Number(parts[idx_ot_leader_weight]),
      partitionId: Number(parts[idx_ot_partition_id]),
      rloc16: Number(parts[idx_ot_rloc_16]),
      parentAvgRssi: Number(parts[idx_parent_avg_rssi]),
      parentLastRssi: Number(parts[idx_parent_last_rssi]),
      lastUpdated: Number(parts[idx_last_updated]),
      parentNextHop: Number(parts[idx_parent_next_hop]),
      parentLinkQualityIn: Number(parts[idx_parent_link_quality_in]),
      parentLinkQualityOut: Number(parts[idx_parent_link_quality_out]),
      parentRloc16: Number(parts[idx_parent_rloc16]),
      parentAge: Number(parts[idx_parent_age]),
      parentRouterId: Number(parts[idx_parent_router_id]),
      avgRttMillis: Number(parts[idx_avg_rtt_millis]),
      packetSuccessRate: Number(parts[idx_packet_success_rate]),
    });
  });

  // Connect implicit relationships between routers and leaders
  nodes.forEach(({ role, partitionId, parent }, index, arr) => {
    if (role == NodeRole.router && !parent) {
      const leader = nodes.find(
        (node) =>
          node.role == NodeRole.leader && node.partitionId == partitionId
      );
      arr[index].parent = leader?.rloc16;
    }
  });

  return {
    status: "success",
    result: nodes,
  };
}

// This function is used to generate sample data for the network graph
async function getSampleAgentData(): Promise<AgentDataResponse> {
  return {
    status: "success",
    result: [
      {
        id: 53248,
        name: "fd00:db8:a0:0:c153:c515:2c94:f913",
        parent: 36864,
        index: 0,
        role: 3,
        meshLocalEid: "fd00:db8:a0:0:c153:c515:2c94:f913",
        linkLocalIPv6: "fe80:0:0:0:748f:9a42:c345:7273",
        leaderId: 36,
        leaderWeight: 64,
        partitionId: 821843429,
        rloc16: 53248,
        parentAvgRssi: -30,
        parentLastRssi: -31,
        lastUpdated: new Date().getUTCSeconds(),
        parentNextHop: 0,
        parentLinkQualityIn: 3,
        parentLinkQualityOut: 3,
        parentRloc16: 36864,
        parentAge: 254,
        parentRouterId: 36,
        avgRttMillis: 98,
        packetSuccessRate: 0.25,
      },
      {
        id: 36864,
        name: "fd00:db8:a0:0:b079:5a2b:506d:df8e",
        parent: undefined,
        index: 1,
        role: 4,
        meshLocalEid: "fd00:db8:a0:0:b079:5a2b:506d:df8e",
        linkLocalIPv6: "fe80:0:0:0:686e:af66:ba07:f47f",
        leaderId: 36,
        leaderWeight: 64,
        partitionId: 821843429,
        rloc16: 36864,
        parentAvgRssi: 127,
        parentLastRssi: 0,
        lastUpdated: new Date().getUTCSeconds(),
        parentNextHop: 0,
        parentLinkQualityIn: 0,
        parentLinkQualityOut: 0,
        parentRloc16: 0,
        parentAge: 140,
        parentRouterId: 0,
        avgRttMillis: 0,
        packetSuccessRate: 0,
      },
    ],
  };
}

type AgentTransportResponse =
  | {
      status: "success";
      result: "UDP" | "TCP" | "MULTI";
    }
  | {
      status: "error";
    };

// Get transport mode from the border router
async function getTransportMode(): Promise<AgentTransportResponse> {
  // Fetch the transport mode from the border router without caching
  const result = await fetch(`${AGENT_HOSTNAME}/transport`, {
    cache: "no-cache",
  });

  // If the status is not 200, return an error
  if (result.status != 200) {
    return {
      status: "error",
    };
  }

  // Parse the response from the border router
  switch (await result.text()) {
    case "udp":
      return {
        status: "success",
        result: "UDP",
      };
    case "tcp":
      return {
        status: "success",
        result: "TCP",
      };
    case "multi":
      return {
        status: "success",
        result: "MULTI",
      };
    default:
      return {
        status: "error",
      };
  }
}

// Set transport mode on the border router
async function setTransportMode(mode: "UDP" | "TCP" | "MULTI") {
  // Send a POST request to the border router to set the transport mode
  const result = await fetch(`${AGENT_HOSTNAME}/transport`, {
    method: "POST",
    body: mode,
  });

  console.log(result);
  const response = await result.text();

  // Make sure the response is "OK"
  return response == "OK";
}

export { getAgentData, getSampleAgentData, getTransportMode, setTransportMode };
