"use client";

import React from "react";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { BsFillLightningChargeFill } from "react-icons/bs";

function SettingsMenubar() {
  // <BsFillLightningChargeFill size="1.5rem" />
  return (
    <Select defaultValue="udp">
      <SelectTrigger className="w-[180px]">
        <SelectValue />
      </SelectTrigger>
      <SelectContent>
        <SelectItem value="udp">UDP</SelectItem>
        <SelectItem value="multicast">Multicast</SelectItem>
        <SelectItem value="tcp" disabled>
          TCP
        </SelectItem>
      </SelectContent>
    </Select>
  );
}

export default SettingsMenubar;
