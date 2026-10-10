Assignment 7: UDP Sockets

Configured for roll number 24527. This gives port 30527 and Service Type C
(digit sum 5+2+7=14; 14 modulo 3 is 2). The server accepts text and replies
with LEN, VOWELS, CONSONANTS, and DIGITS counts.

Compile on Linux/macOS (or a POSIX environment such as WSL):
  gcc -Wall -Wextra -std=c11 udp_server.c -o udp_server
  gcc -Wall -Wextra -std=c11 udp_client.c -o udp_client

If you set ROLL_NUMBER in the source, omit -DROLL_NUMBER=YOUR_ROLL above.
Run the server in one terminal:
  ./udp_server

Run the client in another terminal, using the server's IPv4 address and roll 24527:
  ./udp_client 127.0.0.1 24527

Enter a text string at the client prompt, for example:
  Hello123
The response will be LEN=8;VOWELS=2;CONSONANTS=3;DIGITS=3.

Enter EXIT at the client prompt to send the roll-prefixed server shutdown request.
The server replies once and then exits. The server accepts only requests whose
roll prefix exactly matches its configured roll.
