#!/usr/bin/env bash
# =============================================================================
# netem: make the network BAD on purpose, then watch what your system does.
#
# `tc` (traffic control) + the `netem` queueing discipline are built into
# Linux. They add delay, loss, duplication, and reordering to every packet
# LEAVING an interface. Containers need the NET_ADMIN capability for this
# (docker-compose.yml already grants it).
#
# Run INSIDE a container, e.g. to make engine_xau's market data lossy:
#   docker compose exec engine_xau bash /app/tools/netem.sh loss 5%
#   docker compose exec engine_xau bash /app/tools/netem.sh show
#   docker compose exec engine_xau bash /app/tools/netem.sh clear
#
# EXPERIMENTS TO TRY (and write down what you see):
#   1. loss 5% on an engine       -> client gap counters climb (UDP has no retransmit)
#                                    ...but orders over TCP still arrive. Why? (TCP retransmits!)
#                                    Watch TCP retransmits in Wireshark or with `ss -ti`.
#   2. delay 50ms on gateway1     -> clients on gateway1 get fills later. Do they trade worse?
#   3. delay 20ms 10ms (jitter)   -> combined with `reorder`, UDP packets arrive out of order
#   4. duplicate 2%               -> does your gap detector drop the duplicates?
#   5. loss 30% on a gateway      -> do the LB health checks flap? (Hysteresis helps!)
#
# NOTE: netem affects EGRESS (outgoing) traffic on the interface. A container
# on two networks has two interfaces (eth0, eth1). Check with `ip addr` and
# pass the interface name as the last argument if needed.
# =============================================================================
set -euo pipefail

ACTION="${1:-show}"
VALUE="${2:-}"
IFACE="${3:-eth0}"

case "$ACTION" in
  delay)     tc qdisc replace dev "$IFACE" root netem delay "$VALUE" ;;          # e.g. 50ms
  jitter)    tc qdisc replace dev "$IFACE" root netem delay "$VALUE" 10ms ;;     # e.g. 20ms (+/- 10ms)
  loss)      tc qdisc replace dev "$IFACE" root netem loss "$VALUE" ;;           # e.g. 5%
  duplicate) tc qdisc replace dev "$IFACE" root netem duplicate "$VALUE" ;;      # e.g. 2%
  reorder)   tc qdisc replace dev "$IFACE" root netem delay 10ms reorder "$VALUE" ;;  # e.g. 25%
  clear)     tc qdisc del dev "$IFACE" root 2>/dev/null || true ;;
  show)      tc qdisc show dev "$IFACE" ;;
  *) echo "usage: $0 {delay|jitter|loss|duplicate|reorder|clear|show} [value] [iface]"; exit 1 ;;
esac

tc qdisc show dev "$IFACE"

# TODO(P4): add a `capture` action:
#   tcpdump -i "$IFACE" -n -w /tmp/capture.pcap port 9300
#   then copy it out: docker compose cp engine_xau:/tmp/capture.pcap .
#   and open it in Wireshark on Windows. Find your seq numbers in the raw bytes!
