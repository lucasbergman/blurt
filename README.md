# Blurt - Open Source Mumble Client for Xbox One

N.B. This is a work in progress. Almost nothing works yet.

This project is an attempt to make a [Mumble](https://www.mumble.info/) voice
chat program for Xbox One. It uses the [Universal Windows Platform
(UWP)](https://docs.microsoft.com/en-us/windows/uwp/get-started/universal-application-platform-guide),
so it can run on lots of Windows 10+ devices, but we're primarily targeting
Xbox One.

Here's a checklist—give or take a bit—I'd like to have complete before opening
up to testing and bashing by users:

- ❌ `network` - Connect to a server
- ❌ `network` - Ping support to keep connections alive
- ❌ `network` - Gracefully handle server rejections
- ❌ `network` - Disconnect cleanly from a server
- ❌ `network` - Join a particular channel
- ❌ `network` - Receive encoded audio over the control socket
- ❌ `network` - Send encoded audio over the control socket
- ❌ `UI` - Basic page to connect to a server
- ❌ `UI` - Page displaying current server channels
- ❌ `UI` - UI affordance for joining a channel
- ❌ `UI` - Text box for a rolling log of connection data
- ❌ `audio` - Decode Opus audio
- ❌ `audio` - Encode Opus audio

Note that this project was previously known as Maunder. It's changed to Blurt,
primarily because that word is funnier.
