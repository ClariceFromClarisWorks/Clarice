# Next milestone: Clarice Remote Sessions

Once the build/deployment consistency task is complete, I want our next major feature to be independent remote shell access over TCP Telnet and serial.

This is a deliberately bounded milestone. Our recent development has involved too much iterative repair and scope expansion. I want a useful, finished feature with objective acceptance tests.

## Goal

Allow a remote operator to connect to Clarice, authenticate if required, obtain a fully functional independent Clarice prompt, execute existing commands and transfer files.

This must work without hijacking, echoing into, or otherwise interfering with the local graphical terminal.

The primary motivation is easier automated testing and remote management of the physical Macintosh SE, but this should be a permanent user-facing capability.

## 1. Investigate and reuse existing infrastructure

Before designing anything, inspect:

- The existing shell command dispatcher and executable loader.
- Input/output handling and any assumptions about a single active terminal.
- Existing serial and TCP networking implementations.
- The existing Telnet client and SEControl facilities.
- Existing session state, memory allocation and preferences handling.
- Available established, lightweight Telnet and file-transfer implementations that could realistically be adapted for System 6 and the 68000.

Avoid reinventing protocols or duplicating the shell.

Prefer established standards and reusable components, but assess their actual memory footprint, compatibility and licensing before adopting code.

## 2. Independent remote shell sessions

A remote connection must provide its own functioning Clarice prompt.

Session-specific state must include:

- Input and output streams.
- Current working directory.
- Command editing and history where supported.
- Active command and interruption handling.
- Relevant shell/session state.

The remote session must run existing Clarice commands, including appropriate `/bin` executables.

Output must be directed to the originating session.

Running a command remotely must not change the local terminal's current directory, commandeer its prompt or interfere with its input.

Filesystem changes are naturally shared, but session interaction must remain independent.

Initially support one authenticated remote session at a time. Additional simultaneous sessions are explicitly out of scope.

## 3. TCP and serial access

Support two independently configurable transports:

**TCP:** Implement a standards-compatible Telnet server that works with ordinary Telnet clients, including the essential protocol negotiation and terminal handling needed for reliable interaction.

**Serial:** Expose the same remote shell over an existing serial connection, using standard terminal behaviour without unnecessary Telnet protocol negotiation.

Both transports should use the same shell/session infrastructure, not separately implemented command interpreters.

Investigate and preserve existing serial control functionality. Avoid contention with SEControl or other serial-port users.

## 4. Preferences and lifecycle

Add Preferences controls for:

- Enable/disable TCP remote access.
- Enable/disable serial remote access.
- TCP listening port.
- Remote-access password, including the option to disable password authentication.
- A clear warning about unencrypted access.

Remote access should be disabled by default.

Enabling and disabling services must work without restarting Clarice where reasonably possible.

Disabling a transport must close its listener and relevant active connections, release resources and preserve unrelated functionality.

Do not retain unnecessary remote-session buffers or network allocations while the service is disabled.

## 5. Authentication and security

This is deliberately a lightweight, trusted-network feature, not an enterprise-grade secure remote-access implementation.

Use a simple, established password-verification mechanism appropriate to the hardware. Do not store a recoverable plaintext password.

Avoid unnecessary cryptographic infrastructure.

Provide reasonable protection against repeated password guesses and permit cancelling unauthenticated connections.

Default to disabled and avoid unintended exposure beyond the intended trusted network. Explain any limitations of the available networking stack.

Document explicitly that Telnet transmits credentials and session contents without encryption and must never be exposed to untrusted networks or the public internet.

Do not represent the password mechanism as providing transport security.

Future SSH support is separate and out of scope.

## 6. Connection indication

When a remote session is connected, Clarice must visibly indicate this somewhere unobtrusive in its existing window or interface.

The indicator should identify whether the connection is TCP or serial and update correctly on connection and disconnection.

Do not introduce frequent full-window redraws or interfere with the existing renderer optimisations.

## 7. Bidirectional file transfers

Investigate established terminal file-transfer protocols, prioritising Kermit given our previous serial-transfer experience.

The goal is reliable sending and receiving of files using common, compatible terminal software over supported transports.

Do not invent a Clarice-specific file-transfer protocol.

Macintosh files require special consideration: resource forks, Finder type/creator information and other necessary metadata must not be silently lost.

Use a standard preservation/encapsulation format if the selected transfer protocol cannot represent Macintosh file metadata directly.

Determine the simplest practical implementation that supports both transfer directions and offers interoperability with standard client software.

If file transfer materially increases the scope, isolate it as the final implementation phase rather than compromising the core remote shell.

## 8. Performance and stability

The baseline remains an 8 MHz Motorola 68000 Macintosh SE with 4 MB RAM, running System 6.0.8.

Avoid permanent memory overhead where possible.

The local graphical terminal must remain responsive while a remote connection is idle or executing commands.

Remote connections must not introduce excessive polling or busy-waiting.

Test disconnection during commands, authentication, file transfers and application shutdown.

A remote client disconnect must not crash Clarice or leave the shell unusable.

## 9. Strict acceptance criteria

The milestone is complete when:

1. TCP and serial access can be enabled or disabled independently through Preferences.
2. A standard Telnet client can connect and reach a functioning Clarice prompt.
3. A serial terminal client can reach the same shell functionality.
4. Remote commands execute using the existing Clarice command infrastructure.
5. Local and remote terminals maintain independent prompt, input, output and working-directory state.
6. Authentication accepts the correct password and rejects incorrect passwords.
7. The graphical interface accurately indicates remote connection state.
8. Both sending and receiving files work, with Macintosh metadata preserved through a documented method.
9. Disconnects and interrupted transfers recover cleanly.
10. Disabling remote access releases its listeners and session resources.
11. Existing local command execution, serial control, recovery and rendering behaviour do not regress.
12. Memory use and performance are measured on the physical SE, not inferred solely from Snow.

## 10. Scope control

Do not implement SSH, SCP, SFTP, multiple simultaneous remote clients, user accounts, permissions, remote desktop, elaborate terminal emulation or a new shell architecture unless an absolutely necessary prerequisite is demonstrated.

Do not refactor unrelated subsystems.

Break implementation into independently verifiable phases, with clear completion criteria. Do not automatically proceed into unrelated work discovered during implementation.

Investigate first and present a concise technical proposal identifying the reusable components, architectural risks, expected memory footprint and planned implementation phases before modifying production code.

Do not push, publish, tag or deploy to physical hardware without explicit authorisation.

Update CURRENT-STATE and relevant documentation as phases are completed.

The priority is a reliable, useful feature—not a theoretically perfect general-purpose remote access framework.