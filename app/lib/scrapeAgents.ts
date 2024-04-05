const AGENT_HOSTNAME = "http://192.168.4.1";

// 0,4,fd00:db8:a0:0:8e7e:ff6e:f9af:601c,fe80:0:0:0:e805:d71e:274:7031,23,64,1833009079,23552,127,0,1,0,0,0,0,240,0

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
  const result = await fetch(AGENT_HOSTNAME, { cache: "no-cache" });

  if (result.status != 200) {
    return {
      status: "error",
      error: result.statusText,
    };
  }

  const body = await result.text();
  const lines = body.trim().split("\n");
  const nodes: NodeEntry[] = [];

  if (lines.length < 2) {
    return {
      status: "success",
      result: [],
    };
  }

  const headerCols = (lines.shift() as string).split(",");

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

export { getAgentData };
