# Rover Reveal Script — Embedded Subteam

*Non-technical audience. PEEL format. ~6 minutes.*

---

## INTRODUCTION

Hi everyone, I'm [NAME], and I'm here to talk about what's happening *inside* the rover — the software running on the computers that make it all work. You've heard about the mechanical design and the electronics — our job on the embedded subteam was to write the code that ties it all together. I'll walk you through the five main systems we built this year, and I promise to keep the jargon to a minimum.

---

## 1. THE ARCHITECTURE — Six Computers, One Rover

**Point:** The rover runs on six independent mini-computers. None of them make decisions — they receive commands from the base station and execute them reliably, in real time.

**Example:** Think of a postal system. The base station writes the letters — "drive forward," "move the arm," "read the pH." Our boards are the postal workers: they deliver those commands to the right hardware, collect the responses, and send them back. No board decides *what* to do — they ensure it *gets done*, reliably, every millisecond.

**Explanation:** Each board runs a real-time operating system called FreeRTOS that guarantees critical tasks are always processed first. If one board gets overloaded, the others keep running independently. Multiple tasks run in parallel on each board — receiving commands, forwarding them to hardware, sending data back — all simultaneously, all prioritised.

**Link:** But six separate computers are useless if they can't talk to each other.

---

## 2. THE COMMUNICATION SYSTEM — A Shared Language

**Point:** We built a custom communication system where every board exchanges structured messages over Ethernet using a format called Protocol Buffers — guaranteeing every board understands every other board perfectly.

**Example:** When the operator pushes the joystick, that command gets packed into a digital "envelope," travels over a cable, and gets decoded on the other end — under a millisecond. We defined over 25 message types: speed commands, arm positions, sensor readings, brake commands, diagnostics.

**Explanation:** Every message gets wrapped in a single master envelope. We strip trailing zeros before sending to reduce traffic, pad them back on receipt. The Ethernet driver was written from scratch: it manages low-level memory transfers, maintains address tables mapping each board to its hardware address, and filters incoming packets so boards only process messages meant for them.

**Link:** Getting data from A to B is half the problem. Routing it to the right place is the other half.

---

## 3. THE PACKET DISPATCHER — Automatic Message Routing

**Point:** The Packet Dispatcher sorts incoming messages by type and delivers each one to the right handler — like a post office dropping every letter in the correct mailbox.

**Example:** The driving board might receive a speed command, a brake command, and a diagnostic request simultaneously. The Dispatcher decodes each envelope, identifies its type, and places it in a dedicated queue. A worker already waiting at that queue picks it up and acts. Each type gets its own queue — so a flood of sensor data never blocks a brake command.

**Explanation:** Adding support for a new message type requires one line of code — a macro that says "when you see this type, call this function." It automatically allocates memory, creates the queue, and spawns the worker. This meant different subteams could work completely independently without touching each other's code.

**Link:** For that independence to work, every board needs to handle errors and debugging the same way.

---

## 4. SHARED LIBRARIES — Common Building Blocks

**Point:** We built shared libraries every board uses, so six sub-projects maintained by different people all handle errors, logging, and data identically.

**Example:** Instead of each developer inventing their own error reporting, we defined 22 standardised error codes — "OK," "timeout," "buffer too small" — with human-readable descriptions. Macros chain operations together: if any step fails, execution stops and the error gets logged automatically.

**Explanation:** The logging system colour-codes messages by severity and streams them over USB. Plugging a cable into any board gave us a live feed of everything happening on it — every packet received, every error encountered. We also built a priority queue ensuring urgent data always gets served first, and a memory-efficient key-value store that avoids unpredictable dynamic allocation.

**Link:** These shared tools are what let the whole team contribute without breaking each other's work.

---

## 5. THE BUILD SYSTEM — One Codebase, Six Firmwares

**Point:** A custom build system compiles one codebase into six different firmware images — one per board — each pulling in only what it needs.

**Example:** Everything lives in one repository. A Python script reads our configuration and determines which files belong to each board. Message definitions are auto-generated from schema files, so changing a format is one edit that takes effect everywhere instantly.

**Explanation:** When we update a shared library, every board picks up the change immediately — no risk of one board running old code while another runs new. This is what allowed a team of students working on different subsystems to contribute to the same project without conflicts.

**Link:** And that's what makes this rover a cohesive system, not six disconnected pieces.

---

## CLOSING

Our boards don't make decisions — the operators do. What we built is the infrastructure that makes those decisions *happen*: six coordinated computers, a custom networking stack, automatic message routing, shared error handling, and a build system that kept the whole team in sync. Every layer was designed, implemented, and tested by our embedded subteam this year. And importantly, we didn't just build something that works — we built something that's *maintainable and extendable*. Next year's team can add a new sensor, a new board, or a new message type without rewriting what's already there. That was a conscious choice: build the infrastructure right, so the rover can grow.

Now, we'd like to show you what it can do.

*[Live demonstration]*

---

**Notes:** ~5 min at measured pace. Sections self-contained — can cut 4 if short on time.
