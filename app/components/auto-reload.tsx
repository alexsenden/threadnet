"use client";

import { useEffect } from "react";
import { useRouter } from "next/navigation";

function AutoReload() {
  const router = useRouter();

  // When the component is loaded on the client side
  useEffect(() => {
    // If the router is available
    if (router) {
      // Every 10 seconds
      const intervalId = setInterval(() => {
        // Refresh the page (soft reload, only fetches new data)
        router.refresh();
      }, 10000);

      // Clear the interval when the component is unmounted
      return () => clearInterval(intervalId);
    }
  }, [router]);

  // Return nothing
  return null;
}

export default AutoReload;
