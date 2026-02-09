# ft_traceroute

Recode of the **traceroute** command (IPv4) in C.  
This project is part of **Project UNIX** (42).

`traceroute` is a network diagnostic tool that shows the route (hops) an IP packet takes to reach a destination by sending probes with increasing **TTL** and listening for **ICMP** replies.

---

## Features (Mandatory)

- **Executable:** `ft_traceroute`
- **Language:** C
- **Build:** Makefile
- **Only option supported:** `--help`
- **Target:** one parameter only — IPv4 **address or hostname**
- **IPv4 only**
- **FQDN handled for header** (prints `traceroute to <input> (<ip>) ...`)
- **No DNS resolution during hop display** (hops are printed as IP only)
- Output indentation close to the real `traceroute`
- Reasonable error handling (no crashes)

---

## How it works

For each TTL from `1` to `30`:

1. Send **UDP probes** to the destination with the current TTL.
2. Routers on the path decrement TTL; when it reaches `0`, they respond with **ICMP Time Exceeded**.
3. When the destination is reached, it responds with **ICMP Destination Unreachable (Port Unreachable)** (because we probe closed UDP ports).
4. Measure RTT per probe and print results like traceroute.

---

## Project Structure
```
.
├── includes/
│ └── ft_traceroute.h
├── src/
│ ├── main.c
│ ├── args.c
│ ├── resolve.c
│ ├── trace.c
│ ├── print.c
│ └── util.c
└── Makefile
```

---

## Build

```bash
make
```

## Usage

```bash
sudo ./ft_traceroute <destination>
sudo ./ft_traceroute --help
```
