```csv
index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate
0,3,fd00:db8:a0:0:fd:b357:fc4f:6894,fe80:0:0:0:c019:127b:fc11:bc2d,21,64,546387869,23552,-26,-26,4,0,3,3,21504,221,21,68,1.00
1,3,fd00:db8:a0:0:ca0a:f9f1:42ee:5105,fe80:0:0:0:3c81:ef94:577a:d171,21,64,546387869,12288,-20,-20,1,0,3,3,21504,184,21,69,1.00
2,3,fd00:db8:a0:0:5b2c:bf75:1690:37d0,fe80:0:0:0:fcbd:698e:e5e9:2af,21,64,546387869,14336,127,0,10,0,0,0,0,18,0,70,1.00
3,3,fd00:db8:a0:0:c153:c515:2c94:f913,fe80:0:0:0:748f:9a42:c345:7273,21,64,546387869,53248,127,0,7,0,0,0,0,174,0,74,1.00
4,3,fd00:db8:a0:0:8e7e:ff6e:f9af:601c,fe80:0:0:0:e805:d71e:274:7031,21,64,546387869,62464,-41,-39,6,0,3,3,14336,85,14,74,1.00
5,4,fd00:db8:a0:0:78de:9cd6:3deb:d27c,fe80:0:0:0:a8bb:b466:bc73:e02f,21,64,546387869,21504,127,0,5,0,0,0,0,64,0
```

5:34 UDP on the same bench, wheel and spoke toplogy with one more

```csv
index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate
0,3,fd00:db8:a0:0:fd:b357:fc4f:6894,fe80:0:0:0:c019:127b:fc11:bc2d,21,64,546387869,23552,-26,-26,4,0,3,3,21504,149,21,105,0.92
1,3,fd00:db8:a0:0:ca0a:f9f1:42ee:5105,fe80:0:0:0:3c81:ef94:577a:d171,21,64,546387869,12288,-20,-20,1,0,3,3,21504,112,21,108,1.00
2,3,fd00:db8:a0:0:5b2c:bf75:1690:37d0,fe80:0:0:0:fcbd:698e:e5e9:2af,21,64,546387869,14336,127,0,8,0,0,0,0,144,0,77,1.00
3,3,fd00:db8:a0:0:c153:c515:2c94:f913,fe80:0:0:0:748f:9a42:c345:7273,21,64,546387869,53248,127,0,7,0,0,0,0,102,0,82,1.00
4,3,fd00:db8:a0:0:8e7e:ff6e:f9af:601c,fe80:0:0:0:e805:d71e:274:7031,21,64,546387869,62464,127,0,3,0,0,0,0,100,0,87,1.00
5,4,fd00:db8:a0:0:78de:9cd6:3deb:d27c,fe80:0:0:0:a8bb:b466:bc73:e02f,21,64,546387869,21504,127,0,5,0,0,0,0,248,0,0,
```

5:40 MULTI on same bench, wheel and spoke toplogy

```csv
index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate
0,3,fd00:db8:a0:0:fd:b357:fc4f:6894,fe80:0:0:0:c019:127b:fc11:bc2d,21,64,546387869,23552,-26,-26,5,0,3,3,21504,123,21,69,1.00
1,3,fd00:db8:a0:0:ca0a:f9f1:42ee:5105,fe80:0:0:0:3c81:ef94:577a:d171,21,64,546387869,12288,-20,-20,2,0,3,3,21504,86,21,69,1.00
2,3,fd00:db8:a0:0:5b2c:bf75:1690:37d0,fe80:0:0:0:fcbd:698e:e5e9:2af,21,64,546387869,14336,127,0,9,0,0,0,0,118,0,70,1.00
3,3,fd00:db8:a0:0:c153:c515:2c94:f913,fe80:0:0:0:748f:9a42:c345:7273,21,64,546387869,53248,127,0,8,0,0,0,0,76,0,125,1.00
4,3,fd00:db8:a0:0:8e7e:ff6e:f9af:601c,fe80:0:0:0:e805:d71e:274:7031,21,64,546387869,62464,127,0,4,0,0,0,0,74,0,73,1.00
5,4,fd00:db8:a0:0:78de:9cd6:3deb:d27c,fe80:0:0:0:a8bb:b466:bc73:e02f,21,64,546387869,21504,127,0,6,0,0,0,0,222,0,0,-1.0
```

5:45 UDP on same bench, wheel and spoke toplogy

```csv
index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate
0,3,fd00:db8:a0:0:c153:c515:2c94:f913,fe80:0:0:0:748f:9a42:c345:7273,21,64,1329068250,53248,-84,-76,1,0,3,3,21504,223,21,435,0.33
1,3,fd00:db8:a0:0:fd:b357:fc4f:6894,fe80:0:0:0:c019:127b:fc11:bc2d,21,64,1329068250,23552,127,0,18,0,0,0,0,84,0,257,0.08
2,3,fd00:db8:a0:0:8e7e:ff6e:f9af:601c,fe80:0:0:0:e805:d71e:274:7031,21,64,1329068250,50176,-95,-96,6,0,3,0,21504,154,21,69,0.58
3,4,fd00:db8:a0:0:78de:9cd6:3deb:d27c,fe80:0:0:0:a8bb:b466:bc73:e02f,21,64,1329068250,21504,127,0,3,0,0,0,0,80,0,0,-1.00
4,3,fd00:db8:a0:0:ca0a:f9f1:42ee:5105,fe80:0:0:0:3c81:ef94:577a:d171,21,64,1329068250,12288,-91,-91,13,0,3,3,21504,178,21,331,0.33
5,3,fd00:db8:a0:0:5b2c:bf75:1690:37d0,fe80:0:0:0:fcbd:698e:e5e9:2af,21,64,1329068250,14336,127,0,8,0,
```

6:29 MULTI in massive line, 300 meters

```csv
index,role,ot_mesh_local_eid,ot_link_local_ipv6,ot_leader_id,ot_leader_weight,ot_partition_id,ot_rloc_16,parent_avg_rssi,parent_last_rssi,last_updated,parent_next_hop,parent_link_quality_in,parent_link_quality_out,parent_rloc16,parent_age,parent_router_id,avg_rtt_millis,packet_success_rate
0,3,fd00:db8:a0:0:fd:b357:fc4f:6894,fe80:0:0:0:c019:127b:fc11:bc2d,21,64,1329068250,23552,127,0,14,0,0,0,0,130,0,162,0.67
1,3,fd00:db8:a0:0:8e7e:ff6e:f9af:601c,fe80:0:0:0:e805:d71e:274:7031,21,64,1329068250,50176,127,0,3,0,0,0,0,194,0,98,1.00
2,4,fd00:db8:a0:0:78de:9cd6:3deb:d27c,fe80:0:0:0:a8bb:b466:bc73:e02f,21,64,1329068250,21504,127,0,9,0,0,0,0,68,0,0,-1.00
3,3,fd00:db8:a0:0:5b2c:bf75:1690:37d0,fe80:0:0:0:fcbd:698e:e5e9:2af,21,64,1329068250,14336,127,0,3,0,0,0,0,20,0,0,0.00
4,3,fd00:db8:a0:0:ca0a:f9f1:42ee:5105,fe80:0:0:0:3c81:ef94:577a:d171,21,64,1329068250,12288,-94,-95,21,0,3,3,14336,175,14,563,0.42
6,3,fd00:db8:a0:0:c153:c515:2c94:f913,fe80:0:0:0:748f:9a42:c345:7273,21,64,1329068250,5120,-101,-98,59,0,2,3,12288,48,
```

6:40 UDP in massive line, 300 meters
