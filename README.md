# PatternScanner

C tool to scan for a pattern in a file. Uses the formatting ``AA BB ?? DD`` so you don't need to create a separate mask. Shows all results, and optionally shows an extra amount of bytes per result.

Usage:
```
./psc file "AA BB ?? DD"
```
or if you want to quickly peek x amount of bytes:
```
./psc file "AA BB ?? DD" 16
```
which shows 16 bytes + pattern length at each result.
