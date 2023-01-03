# Prodigy

This project includes a Windows kernel driver and a client application that allows users to dynamically inject DLLs into running processes. The driver uses advanced techniques to bypass security measures and safely inject the DLL into the target process without causing instability or crashes.

## Features

- Injects DLLs into running processes without requiring access to the process's memory
- Supports both 32-bit and 64-bit processes
- Undetected by **Apex EAC**, able to be used in public matches.
- Can inject DLLs into processes that are running with high privileges, such as system processes

## Usage

Download the latest release from the **Releases** tab. Make sure you properly install the program and execute it
with administrative privileges in order for it to properly work.

To use the DLL injector, simply start the client application and select the target process and DLL to inject. 

## Requirements

This project requires a Windows operating system with the latest updates installed. The driver has been tested on Windows 7, 8, and 10, and should work on other versions of Windows as well.
