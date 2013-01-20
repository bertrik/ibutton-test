ibutton-test
============

Testcode to read/write data on a DS1961 SHA-1 iButton



Commands to query the ibutton over the serial connection:

* P

  Poll for found iButton id numbers

* S XXXXXXXXXXXXXXXX

  Write the 16 hex-digit secret to the iButton

* C XXXXXX

  Do an authenticated read with the 6 hex-digits as 3-byte challenge
Prints the data (all zero currently) and the 160-bit MAC, which is
computed from the data, ibutton id, ibutton secret and challenge code.
